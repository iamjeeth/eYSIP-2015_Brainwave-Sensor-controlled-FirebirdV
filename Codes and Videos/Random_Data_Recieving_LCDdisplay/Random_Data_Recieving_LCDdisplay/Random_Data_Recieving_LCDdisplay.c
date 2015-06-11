/*PROGRAM TO RECIEVE ANY DATA
FROM THE BRAINWAVE SENSOR
*/

/*IF THE BOT IS RECIEVING ANY DATA FROM THE SENSOR
THEN THE BAR GRAPH LED WILL GLOW
*/

#define F_CPU 14745600
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#include"lcd.h"
int i;
volatile unsigned char data=0;             //to take data
 //Led configuration
void LED_bargraph_config (void)              
{
	DDRJ = 0xFF;  //PORT J is configured as output
	PORTJ = 0x00; //Output is set to 0
	
	//LCD
	DDRC = DDRC | 0xF7;		
	PORTC = PORTC & 0x80;		
}
//Pin configuration
void buzzer_pin_config (void)                  
{
	DDRC = DDRC | 0x08;		//Setting PORTC 3 as outpt
	PORTC = PORTC & 0xF7;		//Setting PORTC 3 logic low to turnoff buzzer
}
//Motion configuration
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
//Buzzer On
void buzzer_on (void)
{
	unsigned char port_restore = 0;
	port_restore = PINC;
	port_restore = port_restore | 0x08;
	PORTC = port_restore;
}
//Buzzer Off
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
//Function to take data from ISR byte by byte
char USART1_RX_vect()
{
	while(!(UCSR1A & (1<<RXC1)));
	return UDR1;
}
void detect()
{
	data=USART1_RX_vect();
	switch(data)
	{
		case 0xAA: PORTJ=0xFF;
		_delay_ms(100);
		PORTJ=0x00;
		break;
		case 0x02: PORTJ=0x01;
		_delay_ms(100);
		PORTJ=0x00;
		case 0x04: PORTJ=0x03;
		_delay_ms(100);
		PORTJ=0x00;
		break;
		case  0x20: PORTJ=0x07;
		_delay_ms(100);
		PORTJ=0x00;
		break;
		case 0x80: PORTJ=0x0F;
		_delay_ms(100);
		PORTJ=0x00;
		break;
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
	lcd_init();             //LCD initialization
	lcd_set_4bit();         //Set LCD bit
	
	while(1){
		detect();
	}
}

