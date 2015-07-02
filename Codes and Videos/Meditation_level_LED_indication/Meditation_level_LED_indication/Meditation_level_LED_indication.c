/*
 * This code glows LED bar graph depending upon the meditation level of the brain of user. 
 * As meditation is high number of LEDs glowing will be more.
 * 
 */ 
#define F_CPU 14745600
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
volatile unsigned char Meditation[3]={0};
volatile unsigned char payloadDataB[32] = {0};
volatile unsigned char checksum=0,generatedchecksum=0;
volatile unsigned char Med_Avg;
unsigned int Plength;
unsigned int f,k=0;
long Temp1;

//Configuration of LED Bar graph
void LED_bargraph_config (void)
{
	DDRJ = 0xFF;  //PORT J is configured as output
	PORTJ = 0x00; //Output is set to 0
}
//Configuration of Buzzer
void buzzer_pin_config (void)
{
	DDRC = DDRC | 0x08;		//Setting PORTC 3 as outpt
	PORTC = PORTC & 0xF7;   //Setting PORTC 3 logic low to turnoff buzzer
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
//Configuration of UART1 for receiving data packets via bluetooth from EEG sensor
void uart1_init(void)
{
	UCSR1B = 0x00; //disable while setting baud rate
	UCSR1A = 0x00;
	UCSR1C = 0x06;
	UBRR1L = 0x5F; //set baud rate lo
	UBRR1H = 0x00; //set baud rate hi
	UCSR1B = 0xD8;
}
// Receives data packets and store them in the buffer
char USART1_RX_vect()
{
	while(!(UCSR1A & (1<<RXC1)));
	return UDR1;
}
//Initialization of buzzer and LED bargraph 
void port_init()
{
	buzzer_pin_config();
	LED_bargraph_config();
}
//function for detecting various levels of attention 
void checkData(){
	if(Med_Avg>=1 && Med_Avg<=10){  //Poor meditation level
		PORTJ=0X01;
	}
	else if(Med_Avg>10 && Med_Avg<=30){  //Poor meditation level
		PORTJ=0x03;
	}
	else if(Med_Avg>30 && Med_Avg<=40){ //Meditation level building up
		PORTJ=0X07;
	}
	else if(Med_Avg>40 && Med_Avg<=50){  //Neutral
		PORTJ=0X0F;
	}
	else if(Med_Avg>50 && Med_Avg<=60){  //Neutral
		PORTJ=0X1F;
	}
	else if(Med_Avg>60 && Med_Avg<=70){  //Slightly elevated
		PORTJ=0X3F;
	}
	else if(Med_Avg>70 && Med_Avg<=80){  //Slightly elevated
		PORTJ=0X7F;
	}
	else if(Med_Avg>80 && Med_Avg<=100){ //Elevated
		PORTJ=0xFF;
	}
}


void Big_Packet()
{
	generatedchecksum = 0;
	for(int i = 0; i < Plength; i++)
	{
		payloadDataB[i]     = USART1_RX_vect();      //Read payload into memory
		generatedchecksum  += payloadDataB[i] ;
	}
	generatedchecksum = 255 - generatedchecksum;
	checksum  = USART1_RX_vect();
	
	if(checksum == generatedchecksum)        // Verify Checksum
	{
		if (payloadDataB[30]==5) //Checking for meditation
		{
			if (f<2)
			{
				Meditation [k] = payloadDataB[31]; // meditation level indication
				Temp1 += Meditation [k];
				f++;
			}
			else
			{
				Med_Avg = Temp1/2;
				checkData();
				f=0;
				Temp1=0;
			}
		}
	}
}	

void init_devices(void)
{
	cli(); //Clears the global interrupts
	port_init();  //Initializes all the ports
	uart1_init(); //Initialize UART1 for serial communication
	sei();   //Enables the global interrupts
}


void main(void)                     // Main Function
{
	init_devices();
	int j=0;
	while (1)
	{
		
		if(USART1_RX_vect() == 170)        // AA 1 st Sync data
		{
			if(USART1_RX_vect() == 170)      // AA 2 st Sync data
			{
				Plength = USART1_RX_vect();
				if(Plength == 32)   // Big Packet
				{
					Big_Packet ();
				}
			}
		}
	}
}