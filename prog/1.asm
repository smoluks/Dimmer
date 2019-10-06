#define minbitsize 8    
#define maxbitsize 12 		
#define valuechangebitsize 5  
#define t_en 4
#define t_max 88
#define presc_change 3

;последовательность для on - 00011100 01011111 00001111 
#define on1 0b00000000
#define on2 0b00011100
#define on3 0b01011111
#define on4 0b00001111

#define off1 0b00000000
#define off2 0b00011100
#define off3 0b01011111
#define off4 0b11000011

#define sleep1 0b00000000
#define sleep2 0b00011100
#define sleep3 0b01011111
#define sleep4 0b00000011

#define bitcnt 24

.ORG 0x000 rjmp RESET ; Reset Handler
;.ORG 0x001 rjmp EXT_INT0 ; IRQ0 Handler
.ORG 0x002 rjmp PCINT0e ; PCINT0 Handler
;.ORG 0x003 rjmp TIM0_OVF ; Timer0 Overflow Handler
;.ORG 0x004 rjmp EE_RDY ; EEPROM Ready Handler
;.ORG 0x005 rjmp ANA_COMP ; Analog Comparator Handler
.ORG 0x006 rjmp TIM0_COMPA ; Timer0 CompareA Handler
;.ORG 0x007 rjmp TIM0_COMPB ; Timer0 CompareB Handler
;.ORG 0x008 rjmp WATCHDOG ; Watchdog Interrupt Handler
;.ORG 0x009 rjmp ADC ; ADC Conversion Handler
;

PCINT0e:
push r16
push r17
in r16, SREG
push r16
;свет включен?
sbrs r23, 3
rjmp p2
 ;100%
 cbi portb, 1 ;включение
 sbi ddrb, 1
 lds r16, 0x60 ;время выключения
 lds r17, 0x61
 add r16, r24
 adc r17, r25
 movw r8, r16 
 rjmp pexit
 ;промежуточно
 p2:
 lds r16, 0x62 ;время включения
 lds r17, 0x63
 ;100%?
 cp r16, r2
 cpc r17, r2
 brne p5
  sbr r23, 0b00001000
  cbi portb, 1 ;переключение порта
  sbi ddrb, 1
  lds r16, 0x60 ;время выключения
  lds r17, 0x61
  add r16, r24
  adc r17, r25
  movw r8, r16 
  rjmp p3
 p5:
 add r16, r24
 adc r17, r25
 movw r8, r16 
 ;изменение яркости света
 inc r28
 cpi r28, presc_change
 brlo pexit;
  clr r28
  lds r26, 0x62 
  lds r27, 0x63
  sbrs r23, 4
  rjmp p4
   ;+
   sbiw r26, 1  
   rjmp p3
   ;-	
   p4:
   adiw r26, 1
   ldi r18, low(t_max) 
   ldi r19, high(t_max)
   cp r26, r18
   cpc r27, r19
   brlo p3
    ;0%
	cbi ddrb, 1
    sbi portb, 1
    cbr r23, 0b00000100
    out gimsk, r2	
 p3:
 sts 0x62, r26
 sts 0x63, r27
pexit:
pop r16
out SREG, r16
pop r17
pop r16
reti

TIM0_COMPA:
push r16
push r17
push r26
push r27
in r16, SREG
push r16
;
adiw r24, 1 ;systick
;обработка света
sbrs r23, 2
rjmp t1;
 ;свет включен
 cp r24, r8
 brne t1;
 cp r25, r9
 brne t1
  sbic ddrb, 1
  rjmp t6
   ;вкл
   cbi portb, 1
   sbi ddrb, 1
   lds r16, 0x60 ;время выключения
   lds r17, 0x61
   add r16, r24
   adc r17, r25
   movw r8, r16 
   rjmp t1
  t6:
   ;откл
   cbi ddrb, 1
   sbi portb, 1
      
 t1:
;обработка пульта
 cp r24, r14
 brne t5;
 cp r25, r15
 brne t5
  cbr r23, 0b00000001
  sbic pinb, 4
  sbr r23, 0b00000001 
t5:  
;
pop r16
out SREG, r16
pop r27
pop r26 
pop r17
pop r16 
reti
 
check:
;обработчик пакета данных
 ;---on---
 cpi r18, on1
 brne c2
 cpi r19, on2
 brne c2
 cpi r20, on3
 brne c2
 cpi r21, on4
 brne c2
  sbrc r23, 3
  ret
  sbrc r23, 2
  rjmp c6;
   ldi r16, low(t_max) ;стартовый ноль
   ldi r17, high(t_max);
   sts 0x62, r16
   sts 0x63, r17
  c6:
  sbr r23, 0b00010100 ; тенденция +
  ldi r16, 0b00100000
  out gimsk, r16 ;int
  ret  
 ;---off---
 c2:
 cpi r18, off1
 brne c3
 cpi r19, off2
 brne c3
 cpi r20, off3
 brne c3
 cpi r21, off4
 brne c3
 sbrs r23, 3
 rjmp c5;
  ldi r16, 0x01
  sts 0x62, r16
  sts 0x63, r2
 c5: 
 cbr r23, 0b00011000 ; тенденция -
 ret
 c3:
 ;---sleep---
 cpi r18, sleep1
 brne c4
 cpi r19, sleep2
 brne c4
 cpi r20, sleep3
 brne c4
 cpi r21, sleep4
 brne c4
  cbi portb, 0
 ret
 c4:
 ;другие кнопки 
ret  
 
 
 
RESET:
;стек
ldi r16, low(RAMEND)
out  SPL,r16
;константы
clr r16
ser r17
movw r2, r16
;порты
ldi r16, 0b00011000
out portb, r16
ldi r16, 0b00000000
out ddrb, r16
rcall delay
;pcint
out gimsk, r2
ldi r16, 0b00000001
out pcmsk, r16
;t0
ldi r16, 0b00000010
out tccr0a, r16
ldi r16, 120
out ocr0a, r16
ldi r16, 0b00000010
out tccr0b, r16
ldi r16, 0b00000100
out timsk0, r16
;data
ldi r16, low(t_en);время пуска симистора
ldi r17, high(t_en)
sts 0x60, r16
sts 0x61, r17
ldi r16, low(t_max);стартовый ноль
ldi r17, high(t_max);
sts 0x62, r16
sts 0x63, r17
mov r23, r2
clr r28
;init
ldi r22, bitcnt
sei
l1:
 wdr
 ;обработчик RF
 sbic pinb, 4
 rjmp l5
   ;спад
   cbr r23, 0b00100000
   rjmp l6
   ;фронт
   l5:
   sbrc r23, 5
   rjmp l6
    sbr r23, 0b00100000
	;проверка на размер бита
	movw r16, r24
	sub r16, r12
	sbc r17, r13
	ldi r26, low(maxbitsize);max
	ldi r27, high(maxbitsize)
	cp r16, r26
	cpc r17, r27
	brsh reinit
	ldi r26, low(minbitsize);min
	ldi r27, high(minbitsize)
	cp r16, r26
	cpc r17, r27
	brlo reinit
    ;запись в r18-r21
    clc
    rol r21
    rol r20 
    rol r19
    rol r18
	cbr r21, 0b00000001
    sbrc r23, 0;
     sbr r21, 0b00000001
    dec r22
    brne l8;
	 ;собран полный пакет
     rcall check
     reinit:
     clr r21
     ldi r22, bitcnt
	l8: 
	movw r12, r24 ;время начала пакета
	ldi r16, low(valuechangebitsize)
	ldi r17, high(valuechangebitsize)
	add r16, r12
	adc r17, r13
	movw r14, r16 ; время чека состояния
 l6:  
 ;обработчик кнопки
 sbic pinb, 3
 rjmp l1
  sbrs r23, 2
  rjmp l9
  ;свет включен
  sbrs r23, 3
  rjmp l10
   ;включен полностью
   ldi r16, 0x01
   sts 0x62, r16
   sts 0x63, r2
   cbr r23, 0b00011000 ; тенденция -
   rjmp lbutton
   ;меняет состояние - меняем тенденцию на противоположную
   l10:
    sbrs r23, 4
	rjmp l11
     cbr r23, 0b00010000	
     rjmp lbutton
	l11:
     sbr r23, 0b00010000
	 rjmp lbutton 
  ;свет выключен
  l9: 
   ldi r16, low(t_max) ;стартовый ноль
   ldi r17, high(t_max);
   sts 0x62, r16
   sts 0x63, r17
   sbr r23, 0b00010100 ; тенденция +
   ldi r16, 0b00100000
   out gimsk, r16 ;int
   rjmp lbutton
lbutton:
wdr
sbis pinb, 3
 rjmp lbutton;   
rjmp l1   

delay:
ldi r17, 0xA0
d3:
 clr r23
  d2:
   clr r16
   d1:
   wdr
   dec r16
   brne d1
  dec r23
  brne d2
 dec r17
 brne d3
ret




