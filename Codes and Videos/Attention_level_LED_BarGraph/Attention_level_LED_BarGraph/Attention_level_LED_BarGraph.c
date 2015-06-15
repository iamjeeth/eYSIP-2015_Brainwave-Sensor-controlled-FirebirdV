/*
 * Attention_level_LED_BarGraph.c
 * This code glows LED bar graph depending upon the attention level of the brain of user. As attention is high number of LEDs glowing will be more.
 * Created: 11-06-2015 10:37:08
 * 
 */ 

#define F_CPU 14745600
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
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
void checkData(){
	if(payloaddata[i]>=1 && payloaddata[i]<=10){
	PORTJ=0X01;}
	else if(payloaddata[i]>10 && payloaddata[i]<=30){
	PORTJ=0x03;}
	else if(payloaddata[i]>30 && payloaddata[i]<=40){
	PORTJ=0X07;}
	else if(payloaddata[i]>40 && payloaddata[i]<=50){
	PORTJ=0X0F;}
	else if(payloaddata[i]>50 && payloaddata[i]<=60){
	PORTJ=0X1F;}
	else if(payloaddata[i]>60 && payloaddata[i]<=70){
	PORTJ=0X3F;}
	else if(payloaddata[i]>70 && payloaddata[i]<=80){
	PORTJ=0X7F;}
	else if(payloaddata[i]>80 && payloaddata[i]<=100){
	PORTJ=0xFF;}
	
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
	
	        if(checksum == generatedChecksum) {
		        for(int i = 0; i < payloadlength; i++) {    // Parse the payload
			        if (payloaddata[i]==0x04){ //check attention level and display
			        i++;
			        checkData();
					}	
					}
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
	while(1){
		detect();
	}
}

