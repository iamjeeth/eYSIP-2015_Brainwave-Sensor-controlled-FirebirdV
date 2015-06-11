/*
 * Checksum_confirmation_buzzeron.c
 * This code turns on the buzzer as checksum byte received at the end of the packet
   is equal to calculated checksum. This shows that the packet received is correct and 
   can be used to further process the data. */ 

#define F_CPU 14745600
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#include"lcd.h"
int i;
volatile unsigned char data=0;
volatile unsigned char payloadlength;
volatile unsigned char poorQuality;
volatile unsigned char checksum;
volatile unsigned char attention;
volatile unsigned char generatedChecksum;
volatile unsigned char payloaddata[]={0};
volatile unsigned char data_array[]={0};
void LED_bargraph_config (void)
{
	DDRJ = 0xFF;  //PORT J is configured as output
	PORTJ = 0x00; //Output is set to 0
	
	//LCD port initialization
	DDRC = DDRC | 0xF7;		
	PORTC = PORTC & 0x80;		
}

void buzzer_pin_config (void)
{
	DDRC = DDRC | 0x08;		//Setting PORTC 3 as outpt
	PORTC = PORTC & 0xF7;		//Setting PORTC 3 logic low to turnoff buzzer
}

void motion_pin_config (void)
{
	DDRA = DDRA | 0x0F;
	PORTA = PORTA & 0xF0;
	DDRL = DDRL | 0x18;   //Setting PL3 and PL4 pins as output for PWM generation
	PORTL = PORTL | 0x18; //PL3 and PL4 pins are for velocity control using PWM.
}

//Function to initialize ports
void port_init()
{
	motion_pin_config();
	buzzer_pin_config();
	LED_bargraph_config();
}

void buzzer_on (void)
{
	unsigned char port_restore = 0;
	port_restore = PINC;
	port_restore = port_restore | 0x08;
	PORTC = port_restore;
}

void buzzer_off (void)
{
	unsigned char port_restore = 0;
	port_restore = PINC;
	port_restore = port_restore & 0xF7;
	PORTC = port_restore;
}

//Function To Initialize UART2
// desired baud rate:9600
// actual baud rate:9600 (error 0.0%)
// char size: 8 bit
// parity: Disabled
void uart1_init(void)
{
	UCSR1B = 0x00; //disable while setting baud rate
	UCSR1A = 0x00;
	UCSR1C = 0x06;
	UBRR1L = 0x5F; //set baud rate lo
	UBRR1H = 0x00; //set baud rate hi
	UCSR1B = 0xD8;
}

char USART1_RX_vect(){
	while(!(UCSR1A & (1<<RXC1)));
	return UDR1;
	
}
void detect(){	
if(USART1_RX_vect()==0xAA){ //check SYNC byte 1
if(USART1_RX_vect()==0xAA){ //check SYNC byte 2
	payloadlength=USART1_RX_vect();
	      if(payloadlength < 169) {                     //Payload length can not be greater than 169
      generatedChecksum = 0;
      for(int i = 0; i < payloadlength; i++) {
	      payloaddata[i] = USART1_RX_vect();            //Read payload into memory
	      generatedChecksum += payloaddata[i];
      }
	checksum = USART1_RX_vect();                      //Read checksum byte from stream
	generatedChecksum = 255 - generatedChecksum;
	
	        if(checksum == generatedChecksum) {       //Checking whether checksum received is correct. If buzzer turns on then packet is correct.
		buzzer_on();_delay_ms(100);buzzer_off();_delay_ms(100);
					}
					}
				}												        
				}	
}	 
//Function To Initialize all The Devices
void init_devices(void)
{
	cli(); //Clears the global interrupts
	port_init();  //Initializes all the ports
	uart1_init(); //Initailize UART1 for serial communiaction
	sei();   //Enables the global interrupts
}

//Main Function
int main(void)
{
	init_devices();
	lcd_init();
	lcd_set_4bit();
	while(1){
		detect();
	}
}

