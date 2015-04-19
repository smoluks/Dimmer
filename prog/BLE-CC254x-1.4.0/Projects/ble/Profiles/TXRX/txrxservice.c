#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"
#include "txrxservice.h"

//------------------GLOBAL VARIABLES-----------------

// TXRX Profile Service UUID
CONST uint8 txrxServUUID[ATT_UUID_SIZE] =
{ 
  TXRX_SERV_UUID
};

// TX Data Char UUID: 0x0002
CONST uint8 txDataCharUUID[ATT_UUID_SIZE] =
{ 
  TX_DATA_CHAR_UUID
};

//-------------------LOCAL VARIABLES-----------------

static txrxServiceCBs_t *txrxService_AppCBs = NULL;

//----------Profile Attributes - variables-----------

// Service attribute
static CONST gattAttrType_t txrxService = { ATT_UUID_SIZE, txrxServUUID };

// Characteristic 2 Properties
static uint8 txrxServiceChar2Props = GATT_PROP_READ | GATT_PROP_WRITE;

// Characteristic 2 Value
static uint8 txDataChar[20] = {0};

// Characteristic 2 Length
static uint8 txDataLen = 0;

// Characteristic 2 User Description
static uint8 txrxServiceChar2UserDesp[17] = "Characteristic 2\0";

// Characteristic 3 Value
static uint8 rxDataChar[20] = {0};

// Characteristic 3 Length
static uint8 rxDataLen = 0;

static gattCharCfg_t simpleProfileChar4Config[GATT_MAX_NUM_CONN];

//-------------Profile Attributes - Table-------------------

static gattAttribute_t txrxAttrTbl[] = 
{
  // Simple Profile Service
  { 
    { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
    GATT_PERMIT_READ,                         /* permissions */
    0,                                        /* handle */
    (uint8 *)&txrxService            /* pValue */
  },
    // Characteristic 2 Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &txrxServiceChar2Props 
    },
      // Characteristic Value 2
      { 
        { ATT_UUID_SIZE, txDataCharUUID },
        GATT_PERMIT_READ  | GATT_PERMIT_WRITE, 
        0, 
        0 
      },
      // Characteristic 2 User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        txrxServiceChar2UserDesp 
      },           
      
};

//-------------------------Prototypes-----------------
static uint8 txrx_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr, 
                            uint8 *pValue, uint8 *pLen, uint16 offset, uint8 maxLen );
static bStatus_t txrx_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                 uint8 *pValue, uint8 len, uint16 offset );


/*********************************************************************
 * PROFILE CALLBACKS
 */
// Simple Profile Service Callbacks
CONST gattServiceCBs_t txrxCBs =
{
  txrx_ReadAttrCB,  // Read callback function pointer
  txrx_WriteAttrCB, // Write callback function pointer
  NULL                       // Authorization callback function pointer
};

/*********************************************************************
 * @fn      TXRX_AddService
 *
 * @brief   Initializes the Simple Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 *
 * @return  Success or Failure
 */
static void simpleProfile_HandleConnStatusCB( uint16 connHandle, uint8 changeType )
{ 
  // Make sure this is not loopback connection
  if ( connHandle != LOOPBACK_CONNHANDLE )
  {
    // Reset Client Char Config if connection has dropped
    if ( ( changeType == LINKDB_STATUS_UPDATE_REMOVED )      ||
         ( ( changeType == LINKDB_STATUS_UPDATE_STATEFLAGS ) && 
           ( !linkDB_Up( connHandle ) ) ) )
    { 
      GATTServApp_InitCharCfg( connHandle, simpleProfileChar4Config );
    }
  }
}

bStatus_t TXRX_AddService( uint32 services )
{
  uint8 status = SUCCESS;
  // Register with Link DB to receive link status change callback  
  // Initialize Client Characteristic Configuration attributes
  GATTServApp_InitCharCfg( INVALID_CONNHANDLE, simpleProfileChar4Config );

  // Register with Link DB to receive link status change callback
  VOID linkDB_Register( simpleProfile_HandleConnStatusCB );
  if ( services & TXRX_SERVICE )
  {
    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService( txrxAttrTbl, GATT_NUM_ATTRS( txrxAttrTbl ),&txrxCBs);
  }
  return ( status );
}



/*********************************************************************
 * @fn      TXRX_RegisterAppCBs
 *
 * @brief   Registers the application callback function. Only call 
 *          this function once.
 *
 * @param   callbacks - pointer to application callbacks.
 *
 * @return  SUCCESS or bleAlreadyInRequestedMode
 */
bStatus_t TXRX_RegisterAppCBs( txrxServiceCBs_t *appCallbacks )
{
  if ( appCallbacks )
  {
    txrxService_AppCBs = appCallbacks;    
    return ( SUCCESS );
  }
  else
  {
    return ( bleAlreadyInRequestedMode );
  }
}
  

/*********************************************************************
 * @fn      TXRX_SetParameter
 *
 * @brief   Set a Simple Profile parameter.
 *
 * @param   param - Profile parameter ID
 * @param   len - length of data to right
 * @param   value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t TXRX_SetParameter( uint8 param, uint8 len, void *value )
{
  bStatus_t ret = SUCCESS;
  
  switch ( param )
  {
    case TX_DATA_CHAR:
      if ( len <= 20 ) 
      {
        VOID osal_memcpy( txDataChar, value, len );
        txDataLen = len;        
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;
    default:
      ret = INVALIDPARAMETER;
      break;
  }
  
  return ( ret );
}

/*********************************************************************
 * @fn      Biscuit_GetParameter
 *
 * @brief   Get a Simple Profile parameter.
 *
 * @param   param - Profile parameter ID
 * @param   value - pointer to data to put.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t TXRX_GetParameter( uint8 param, uint8 *len, void *value )
{
  bStatus_t ret = SUCCESS;  
  switch ( param )
  {   
    case RX_DATA_CHAR:
      len[0] = rxDataLen;
      VOID osal_memcpy(value, rxDataChar, rxDataLen);
      break;       
    default:
      ret = INVALIDPARAMETER;
      break;
  }  
  return ( ret );
}

/*********************************************************************
 * @fn          txrx_ReadAttrCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 *
 * @return      Success or Failure
 */
static uint8 txrx_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr, 
                            uint8 *pValue, uint8 *pLen, uint16 offset, uint8 maxLen )
{
  bStatus_t status = SUCCESS;

  // If attribute permissions require authorization to read, return error
  if ( gattPermitAuthorRead( pAttr->permissions ) )
  {
    // Insufficient authorization
    return ( ATT_ERR_INSUFFICIENT_AUTHOR );
  }
  
  // Make sure it's not a blob operation (no attributes in the profile are long)
  if ( offset > 0 )
  {
    return ( ATT_ERR_ATTR_NOT_LONG );
  }
 
  if ( pAttr->type.len == ATT_UUID_SIZE )
  {
    if ( osal_memcmp(pAttr->type.uuid, txDataCharUUID, ATT_UUID_SIZE) )
    {
      *pLen = txDataLen;
      VOID osal_memcpy( pValue, txDataChar, txDataLen );     
    }
    else
    {
      // Should never get here!
      *pLen = 0;
      status = ATT_ERR_ATTR_NOT_FOUND;
    }
  }

  return ( status );
}

/*********************************************************************
 * @fn      txrx_WriteAttrCB
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 * @param   offset - offset of the first octet to be written
 * @param   complete - whether this is the last packet
 * @param   oper - whether to validate and/or write attribute value  
 *
 * @return  Success or Failure
 */
static bStatus_t txrx_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                 uint8 *pValue, uint8 len, uint16 offset )
{
  bStatus_t status = SUCCESS;
  uint8 notifyApp = 0xFF;
  
  // If attribute permissions require authorization to write, return error
  if ( gattPermitAuthorWrite( pAttr->permissions ) )
  {
    // Insufficient authorization
    return ( ATT_ERR_INSUFFICIENT_AUTHOR );
  }

  if ( pAttr->type.len == ATT_BT_UUID_SIZE )
  {
    // 16-bit UUID
    uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
    switch ( uuid )
    {
      case GATT_CLIENT_CHAR_CFG_UUID:
        status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                                 offset, GATT_CLIENT_CFG_NOTIFY );
        if (status == SUCCESS)
        {
          uint16 charCfg = BUILD_UINT16( pValue[0], pValue[1] );
          txrxService_AppCBs->pfnTXRXServiceChange( (charCfg == GATT_CFG_NO_OPERATION) ?
                                                      TXRX_RX_NOTI_DISABLED :
                                                      TXRX_RX_NOTI_ENABLED );
        }
        break;
        
      default:
        // Should never get here! (characteristics 2 and 4 do not have write permissions)
        status = ATT_ERR_ATTR_NOT_FOUND;
        break;
    }
  }
  else // 128-bit  
  {
    if ( osal_memcmp(pAttr->type.uuid, txDataCharUUID, ATT_UUID_SIZE) )
    {
      //Validate the value
      // Make sure it's not a blob oper
      if ( offset == 0 )
      {
        if ( len > 20 )
        {
          status = ATT_ERR_INVALID_VALUE_SIZE;
        }
      }
      else
      {
        status = ATT_ERR_ATTR_NOT_LONG;
      }
        
      //Write the value
      if ( status == SUCCESS )
      {                
        osal_memcpy(rxDataChar, pValue, len);
        rxDataLen = len;
        notifyApp = TXRX_RX_DATA_READY;           
      }
    }
    else
    {      
      // Should never get here! (characteristics 2 and 4 do not have write permissions)
      status = ATT_ERR_ATTR_NOT_FOUND;
    }
  }

  // If a charactersitic value changed then callback function to notify application of change
  if ( (notifyApp != 0xFF ) && txrxService_AppCBs && txrxService_AppCBs->pfnTXRXServiceChange )
  {
    txrxService_AppCBs->pfnTXRXServiceChange( notifyApp );  
  }
  
  return ( status );
}



/*********************************************************************
*********************************************************************/