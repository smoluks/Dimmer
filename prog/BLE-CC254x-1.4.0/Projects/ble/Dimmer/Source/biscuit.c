//-----inc--------
#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "OnBoard.h"
#include "hal_adc.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_lcd.h"
#include "gatt.h"
#include "hci.h"
#include "gapgattserver.h"
#include "gattservapp.h"
#include "devinfoservice.h"
#include "peripheral.h"
#include "gapbondmgr.h"
#include "biscuit.h"
#include "txrxservice.h"
#include "npi.h"
#include "string.h"
//------const-------
// How often to perform periodic event
#define SBP_PERIODIC_EVT_PERIOD                   5000
// What is the advertising interval when device is discoverable (units of 625us, 160=100ms)
#define DEFAULT_ADVERTISING_INTERVAL          160
// Limited discoverable mode advertises for 30.72s, and then stops
// General discoverable mode advertises indefinitely
#define DEFAULT_DISCOVERABLE_MODE             GAP_ADTYPE_FLAGS_GENERAL
// Minimum connection interval (units of 1.25ms, 80=100ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL     16
// Maximum connection interval (units of 1.25ms, 800=1000ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL     16
// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY         0
// Supervision timeout value (units of 10ms, 1000=10s) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT          1000
// Whether to enable automatic parameter update request when a connection is formed
#define DEFAULT_ENABLE_UPDATE_REQUEST         TRUE
// Connection Pause Peripheral time value (in seconds)
#define DEFAULT_CONN_PAUSE_PERIPHERAL         6
#define INVALID_CONNHANDLE                    0xFFFF
// Length of bd addr as a string
#define B_ADDR_STR_LEN                        15        
#define MAX_RX_LEN                            128
#define SBP_RX_TIME_OUT                       5
//--------var-----------
static uint8 biscuit_TaskID;   // Task ID for internal task/event processing
static uint8 RXBuf[MAX_RX_LEN];
static uint8 rxLen = 0;
static uint8 rxHead = 0, rxTail = 0;
// GAP - SCAN RSP data (max size = 31 bytes)
static uint8 scanRspData[] =
{
  // Tx power level
  //0x02,   // length of this data
  //GAP_ADTYPE_POWER_LEVEL,
  //0,       // 0dBm  
  // service UUID, to notify central devices what services are included
  // in this peripheral
  17,   // length of this data
  GAP_ADTYPE_128BIT_COMPLETE,      // some of the UUID's, but not all
  TXRX_SERV_UUID,    
};
// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
static uint8 advertData[31] =
{
  // Flags; this sets the device to use limited discoverable
  // mode (advertises for 30 seconds at a time) instead of general
  // discoverable mode (advertises indefinitely)
  0x02,   // length of this data
  GAP_ADTYPE_FLAGS,
  DEFAULT_DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,  
  // complete name 
  9,   // length of this data
  GAP_ADTYPE_LOCAL_NAME_COMPLETE,
  'B','L','E',' ','M','i','n','i',  
};              

// GAP GATT Attributes
static uint8 attDeviceName[GAP_DEVICE_NAME_LEN] = "Dimmer01";
/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void biscuit_ProcessOSALMsg( osal_event_hdr_t *pMsg );
static void peripheralStateNotificationCB( gaprole_States_t newState );
static void performPeriodicTask( void );
static void txrxServiceChangeCB( uint8 paramID );
static void dataHandler( uint8 port, uint8 events );
static void addtorx(uint8 buf[], uint8 len);
/*********************************************************************
 * PROFILE CALLBACKS */
// GAP Role Callbacks
static gapRolesCBs_t biscuit_PeripheralCBs =
{
  peripheralStateNotificationCB,  // Profile State Change Callbacks
  NULL                            // When a valid RSSI is read from controller (not used by application)
};

// GAP Bond Manager Callbacks
static gapBondCBs_t biscuit_BondMgrCBs =
{
  NULL,                     // Passcode callback (not used by application)
  NULL                      // Pairing / Bonding state Callback (not used by application)
};
// Simple GATT Profile Callbacks
static txrxServiceCBs_t biscuit_TXRXServiceCBs =
{
  txrxServiceChangeCB    // Charactersitic value change callback
};


void Biscuit_Init( uint8 task_id )
{
  biscuit_TaskID = task_id;
  // Setup the GAP
  VOID GAP_SetParamValue( TGAP_CONN_PAUSE_PERIPHERAL, DEFAULT_CONN_PAUSE_PERIPHERAL );  
  // Setup the GAP Peripheral Role Profile
  {
    // Device starts advertising upon initialization
    uint8 initial_advertising_enable = TRUE;
    // By setting this to zero, the device will go into the waiting state after
    // being discoverable for 30.72 second, and will not being advertising again
    // until the enabler is set back to TRUE
    uint16 gapRole_AdvertOffTime = 0;
    uint8 enable_update_request = DEFAULT_ENABLE_UPDATE_REQUEST;
    uint16 desired_min_interval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
    uint16 desired_max_interval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
    uint16 desired_slave_latency = DEFAULT_DESIRED_SLAVE_LATENCY;
    uint16 desired_conn_timeout = DEFAULT_DESIRED_CONN_TIMEOUT;
    // Set the GAP Role Parametersuint8 initial_advertising_enable = TRUE;
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &initial_advertising_enable );
    GAPRole_SetParameter( GAPROLE_ADVERT_OFF_TIME, sizeof( uint16 ), &gapRole_AdvertOffTime );
    GAPRole_SetParameter( GAPROLE_SCAN_RSP_DATA, sizeof ( scanRspData ), scanRspData );
    GAPRole_SetParameter( GAPROLE_PARAM_UPDATE_ENABLE, sizeof( uint8 ), &enable_update_request );
    GAPRole_SetParameter( GAPROLE_MIN_CONN_INTERVAL, sizeof( uint16 ), &desired_min_interval );
    GAPRole_SetParameter( GAPROLE_MAX_CONN_INTERVAL, sizeof( uint16 ), &desired_max_interval );
    GAPRole_SetParameter( GAPROLE_SLAVE_LATENCY, sizeof( uint16 ), &desired_slave_latency );
    GAPRole_SetParameter( GAPROLE_TIMEOUT_MULTIPLIER, sizeof( uint16 ), &desired_conn_timeout );
  }  
  // Set the GAP Characteristics
  GGS_SetParameter( GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, attDeviceName );
  uint8 len = strlen( (char const *)attDeviceName );
  TXRX_SetParameter( DEV_NAME_CHAR, len, attDeviceName );
  advertData[3] = len + 1;  
  osal_memcpy(&advertData[5], attDeviceName, len);  
  osal_memset(&advertData[len+5], 0, 31-5-len);
  GAPRole_SetParameter( GAPROLE_ADVERT_DATA, sizeof( advertData ), advertData );
  // Set advertising interval
  {
    uint16 advInt = DEFAULT_ADVERTISING_INTERVAL;
    GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MIN, advInt );
    GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MAX, advInt );
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MIN, advInt );
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MAX, advInt );
  }
  // Setup the GAP Bond Manager
  {
    uint32 passkey = 0; // passkey "000000"
    uint8 pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
    uint8 mitm = TRUE;
    uint8 ioCap = GAPBOND_IO_CAP_DISPLAY_ONLY;
    uint8 bonding = TRUE;
    GAPBondMgr_SetParameter( GAPBOND_DEFAULT_PASSCODE, sizeof ( uint32 ), &passkey );
    GAPBondMgr_SetParameter( GAPBOND_PAIRING_MODE, sizeof ( uint8 ), &pairMode );
    GAPBondMgr_SetParameter( GAPBOND_MITM_PROTECTION, sizeof ( uint8 ), &mitm );
    GAPBondMgr_SetParameter( GAPBOND_IO_CAPABILITIES, sizeof ( uint8 ), &ioCap );
    GAPBondMgr_SetParameter( GAPBOND_BONDING_ENABLED, sizeof ( uint8 ), &bonding );
  }
  // Initialize GATT attributes
  GGS_AddService( GATT_ALL_SERVICES );            // GAP
  GATTServApp_AddService( GATT_ALL_SERVICES );    // GATT attributes
  //DevInfo_AddService();                           // Device Information Service
  TXRX_AddService( GATT_ALL_SERVICES );  // Simple GATT Profile
 //----io---
 P0SEL = 0x7C;
 P1SEL = 0xC0;
 P2SEL = 0;
 P0DIR = 0x7C;
 P1DIR = 0x40; 
 P2DIR = 0x00;
 P0 = 0x40;
 P1 = 0; 
 P2 = 0;
 //
 T1CTL = 0x01;
 T1CCTL0 = 0x24;
 T1CC0H = 0x00;
 T1CC0L = 0x01;
 T1CCTL1 = 0x24;
 T1CC1H = 0x80;
 T1CC1L = 0x00;
 T1CCTL2= 0x24;
 T1CC2H = 0x00;
 T1CC2L = 0x01;
 // Register callback with TXRXService
 VOID TXRX_RegisterAppCBs( &biscuit_TXRXServiceCBs );
 // Enable clock divide on halt
 // This reduces active current while radio is active and CC254x MCU
 // is halted
 //  HCI_EXT_ClkDivOnHaltCmd( HCI_EXT_ENABLE_CLK_DIVIDE_ON_HALT );
 // Initialize serial interface
 PERCFG |= 1;
 NPI_InitTransport(dataHandler);  
 U0GCR &= 0xE0;      // Default baudrate 57600
 U0GCR |= 0x0A;
 U0BAUD = 216;
 // Setup a delayed profile startup
 osal_set_event( biscuit_TaskID, SBP_START_DEVICE_EVT );
}

uint16 biscuit_ProcessEvent( uint8 task_id, uint16 events )
{

  VOID task_id; // OSAL required parameter that isn't used in this function
  if ( events & SYS_EVENT_MSG )
  {
    uint8 *pMsg;
    if ( (pMsg = osal_msg_receive( biscuit_TaskID )) != NULL )
    {
      biscuit_ProcessOSALMsg( (osal_event_hdr_t *)pMsg );
      // Release the OSAL message
      VOID osal_msg_deallocate( pMsg );
    }
    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }
  if ( events & SBP_START_DEVICE_EVT )
  {
    // Start the Device
    VOID GAPRole_StartDevice( &biscuit_PeripheralCBs );
    // Start Bond Manager
    VOID GAPBondMgr_Register( &biscuit_BondMgrCBs );
    // Set timer for first periodic event
    osal_start_timerEx( biscuit_TaskID, SBP_PERIODIC_EVT, SBP_PERIODIC_EVT_PERIOD );
    return ( events ^ SBP_START_DEVICE_EVT );
  }  
  if ( events & SBP_RX_TIME_OUT_EVT )
  {
    uint8 data[20];
    uint8 send;
    while(rxLen != 0)
    {
      if(rxLen <= 20)
      {
        send = rxLen;
        rxLen = 0;
      }
      else
      { 
        send = 20;      
        rxLen -= 20;
      }
      for(uint8 i=0; i<send; i++)
      {
        data[i] = RXBuf[rxTail];
        rxTail++;
        if(rxTail == MAX_RX_LEN)
        {
          rxTail = 0;
        }
      }
      TXRX_SetParameter(TX_DATA_CHAR, send, data);
    }
    return (events ^ SBP_RX_TIME_OUT_EVT);
  }
  if ( events & SBP_PERIODIC_EVT )
  {
    // Restart timer
    if ( SBP_PERIODIC_EVT_PERIOD )
    {
      osal_start_timerEx( biscuit_TaskID, SBP_PERIODIC_EVT, SBP_PERIODIC_EVT_PERIOD );
    }
    // Perform periodic application task
    performPeriodicTask();
    return (events ^ SBP_PERIODIC_EVT);
  }
  // Discard unknown events
  return 0;
}

/*********************************************************************
 * @fn      biscuit_ProcessOSALMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void biscuit_ProcessOSALMsg( osal_event_hdr_t *pMsg )
{
  switch ( pMsg->event )
  {
  #if defined( CC2540_MINIDK )
    case KEY_CHANGE:
      biscuit_HandleKeys( ((keyChange_t *)pMsg)->state, ((keyChange_t *)pMsg)->keys );
      break;
  #endif // #if defined( CC2540_MINIDK )

  default:
    // do nothing
    break;
  }
}

#if defined( CC2540_MINIDK )
/*********************************************************************
 * @fn      biscuit_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
static void biscuit_HandleKeys( uint8 shift, uint8 keys )
{
  // do nothing
}
#endif // #if defined( CC2540_MINIDK )

/*********************************************************************
 * @fn      peripheralStateNotificationCB
 *
 * @brief   Notification from the profile of a state change.
 *
 * @param   newState - new state
 *
 * @return  none
 */
static void peripheralStateNotificationCB( gaprole_States_t newState )
{  
  switch ( newState )
  {
    case GAPROLE_STARTED:
      {
        uint8 ownAddress[B_ADDR_LEN];
        uint8 systemId[DEVINFO_SYSTEM_ID_LEN];

        GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddress);

        // use 6 bytes of device address for 8 bytes of system ID value
        systemId[0] = ownAddress[0];
        systemId[1] = ownAddress[1];
        systemId[2] = ownAddress[2];

        // set middle bytes to zero
        systemId[4] = 0x00;
        systemId[3] = 0x00;

        // shift three bytes up
        systemId[7] = ownAddress[5];
        systemId[6] = ownAddress[4];
        systemId[5] = ownAddress[3];

        DevInfo_SetParameter(DEVINFO_SYSTEM_ID, DEVINFO_SYSTEM_ID_LEN, systemId);
      }
      break;

    case GAPROLE_ADVERTISING:
      {
      }
      break;

    case GAPROLE_CONNECTED:
      {     
        uint8 advertising_enable = FALSE;
        GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &advertising_enable );
      }
      break;

    case GAPROLE_CONNECTED_ADV:
      {
      }
      break;      
    case GAPROLE_WAITING:
      {
      }
      break;

    case GAPROLE_WAITING_AFTER_TIMEOUT:
      {
      }
      break;

    case GAPROLE_ERROR:
      {
      }
      break;

    default:
      {
      }
      break;

  }
}

/*********************************************************************
 * @fn      performPeriodicTask
 *
 * @brief   Perform a periodic application task. This function gets
 *          called every five seconds as a result of the SBP_PERIODIC_EVT
 *          OSAL event. In this example, the value of the third
 *          characteristic in the SimpleGATTProfile service is retrieved
 *          from the profile, and then copied into the value of the
 *          the fourth characteristic.
 *
 * @param   none
 *
 * @return  none
 */
static void performPeriodicTask( void )
{
  // do nothing
}

/*********************************************************************
 * @fn      txrxServiceChangeCB
 *
 * @brief   Callback from SimpleBLEProfile indicating a value change
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  none
 */
static void txrxServiceChangeCB(uint8 paramID)
{
  uint8 data[20];
  uint8 len;
  uint8 buf[7];  
  uint8 len2;
  if (paramID == TXRX_RX_DATA_READY)
  {
    TXRX_GetParameter(RX_DATA_CHAR, &len, data);   
    if(len>0)
    {
      if((data[0] == 'B') || (data[0] == 'b'))
      {
        switch(data[1])
        {
          //ping
          case 0x01:
            {
            buf[0] = 'A';
            len2 = 1;
             addtorx(buf, len2);
            }
           break;
          //setled
          case 0x02:
            {
            if(data[2] != 0 && data[3] != 0)
            {
             T1CC0H = data[2];
             T1CC0L = data[3];
             T1CCTL0 = 0x24;
            }
            else
            {
              T1CCTL0 = 0x00;
            }
            if(data[4] != 0 && data[5] != 0)
            {
             T1CC1H = data[4];
             T1CC1L = data[5];
             T1CCTL1 = 0x24;
            }
            else
            {
              T1CCTL1 = 0x00;
            }
            if(data[6] != 0 && data[7] != 0)
            {
             T1CC2H = data[6];
             T1CC2L = data[7];
             T1CCTL2 = 0x24;
            }
            else
            {
              T1CCTL2 = 0x00;
            }
            buf[0]='A';
            TXRX_SetParameter(TX_DATA_CHAR, 1, buf);
        }
        break;
        //read led
        case 0x03:
            {
              buf[0] = T1CC0H;
              buf[1] = T1CC0L;
              buf[2] = T1CC1H;
              buf[3] = T1CC1L;
              buf[4] = T1CC2H;
              buf[5] = T1CC2L;
              buf[6] = buf[0] + buf[1] + buf[2] + buf[3] + buf[4] + buf[5];
              len2 = 7;
              addtorx(buf, len2);
            }
            break;
        //set boot
        case 0x04:
            {
              if(buf[2])
                P0 |= 0x20;
              else
                P0 &= ~0x20;
              buf[0] = 'A';
              len2 = 1;
              addtorx(buf, len2);
            }
            break;
        //read boot
        case 0x05:
            {
              if(P0 & 0x20)                
                buf[0] = 1;
              else
                buf[0] = 0;
              buf[1]=buf[0];
              len2 = 2;
              addtorx(buf, len2);  
            }
            break;
        //set reset
        case 0x06:
            {
              if(buf[2])
                P0 |= 0x40;
              else
                P0 &= ~0x40;
              buf[0] = 'A';
              len2 = 1;
              addtorx(buf, len2);
            }
            break;
        //read reset
        case 0x07:
            {
              if(P0 & 0x40)                
                buf[0] = 1;
              else
                buf[0] = 0;
              buf[1]=buf[0];
              len2 = 2;
              addtorx(buf, len2);  
            }
            break;
        }
      }
      else if((data[0] == 'M') || (data[0] == 'm')) 
      {
        HalUARTWrite(NPI_UART_PORT, (uint8*)data, len);
      }
    }
  }
  else if (paramID == TXRX_RX_NOTI_ENABLED)
  {
    GAPRole_SendUpdateParam( DEFAULT_DESIRED_MAX_CONN_INTERVAL, DEFAULT_DESIRED_MIN_CONN_INTERVAL,
                            DEFAULT_DESIRED_SLAVE_LATENCY, DEFAULT_DESIRED_CONN_TIMEOUT, GAPROLE_RESEND_PARAM_UPDATE );
  }
}

/*********************************************************************
 * @fn      dataHandler
 *
 * @brief   Callback from UART indicating a data coming
 *
 * @param   port - data port.
 *
 * @param   events - type of data.
 *
 * @return  none
 */
static void dataHandler( uint8 port, uint8 events )
{  
  if((events & HAL_UART_RX_TIMEOUT) == HAL_UART_RX_TIMEOUT)
  {
    osal_stop_timerEx( biscuit_TaskID, SBP_RX_TIME_OUT_EVT);    
    uint8 len = NPI_RxBufLen();
    uint8 buf[128];
    NPI_ReadTransport( buf, len );
    addtorx(buf, len);        
    osal_start_timerEx( biscuit_TaskID, SBP_RX_TIME_OUT_EVT, SBP_RX_TIME_OUT);
  }
  return;
}

static void addtorx(uint8 buf[], uint8 len)
{
  uint8 copy;   
  if(len > (MAX_RX_LEN-rxLen))
  {    
    copy = MAX_RX_LEN - rxLen;
    rxLen = MAX_RX_LEN;
  }
  else
  {
    rxLen += len;
    copy = len;
  }
  for(uint8 i=0; i<copy; i++)
  {
    RXBuf[rxHead] = buf[i];
    rxHead++;
    if(rxHead == MAX_RX_LEN)
    {
      rxHead = 0;
    }
  }
}

/*********************************************************************
*********************************************************************/
