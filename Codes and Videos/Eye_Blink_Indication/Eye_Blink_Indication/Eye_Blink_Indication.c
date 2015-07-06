/*
Members: Omkar Rajendra Mohite
         Ashish Kumar Jain
Mentors: Mehul Makwana
         Rutuja Ekatpure
Program for Eyeblink Detection using Mindwave mobile connected to Firebird V.
Eyeblink is detected by Buzzer beep.
Sensor paired is indicated by one LED blink
Sensor removed from head is indicated by all LEDs blink.
*/
#define F_CPU 14745600
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#define   Theshold_Eyeblink  110
#define   EEG_AVG            70
long payloadDataS[5] = {0};
long payloadDataB[32] = {0};
volatile unsigned char checksum=0,generatedchecksum=0;
unsigned int Raw_data,Poorquality,Plength,Eye_Enable=0,On_Flag=0,Off_Flag=1 ;
unsigned int timer_flag=0,eye_count=0,j,n=0,k=0,z=0,p=0,i=0;
long Temp,Avg_Raw,Temp_Avg;

 //LED bargraph configuration
 void LED_bargraph_config (void)
 {
	 DDRJ = 0xFF;  //PORT J is configured as output
	 PORTJ = 0x00; //Output is set to 0
 }
 //Buzzer configuration
void buzzer_pin_config (void)
{
	DDRC = DDRC | 0x08;		//Setting PORTC 3 as outpt
	PORTC = PORTC & 0xF7;		//Setting PORTC 3 logic low to turnoff buzzer
}

//Function to initialize ports
void port_init()
{
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

 void Small_Packet ()
 {
   generatedchecksum = 0;
   for(int i = 0; i < Plength; i++)
   { 
     payloadDataS[i] = USART1_RX_vect();      //Read payload into memory
     generatedchecksum  += payloadDataS[i] ;
   }
   generatedchecksum = 255 - generatedchecksum;
   checksum  = USART1_RX_vect();
   if(checksum == generatedchecksum)        // Verify Checksum
   { 
	//Detection of eye-blink value   
     if (j<64)                             //Taking average of 64 data packets for detection of eye-blink value
     {
       Raw_data  = ((payloadDataS[2] <<8)| payloadDataS[3]);
       if(Raw_data&0xF000)
       {
         Raw_data = (((~Raw_data)&0xFFF)+1);
       }
       else
       {
		   
         Raw_data = (Raw_data&0xFFF);
       }
       Temp += Raw_data;
       j++;
     }
     else
     {
       Onesec_Rawval_Fun ();
     }
   }
 }
 
 void Big_Packet()                            //Detection of zero poor quality signal for verification of eye-blink detection
 {
   for(int i = 0; i < Plength; i++)
   { 
     payloadDataB[i] = USART1_RX_vect();      //Read payload into memory
   }
     Poorquality = payloadDataB[1];
     if (Poorquality==0 )
     {
       Eye_Enable = 1;
     }
     else
     {
       Eye_Enable = 0;
     }
 }
 
 void Onesec_Rawval_Fun ()       //Taking average of 3 values obtained from 64 data packets for confirmation of eye-blink detection
 {
   Avg_Raw = Temp/64;
   if (On_Flag==0 && Off_Flag==1)
   {
     if (n<3)
     {
       Temp_Avg += Avg_Raw;
       n++;
     }
     else
     {
       Temp_Avg = Temp_Avg/3;
       if (Temp_Avg<EEG_AVG)
       {
         On_Flag=1;Off_Flag=0;
       }
       n=0;Temp_Avg=0;
     } 
   }             
   Eye_Blink ();
   j=0;
   Temp=0; 
   }
 
 void Eye_Blink ()
 {
   if (Eye_Enable)         
   {
     if (On_Flag==1 && Off_Flag==0)
     {
       if ((Avg_Raw>Theshold_Eyeblink) && (Avg_Raw<350))  //Eye-blink detection confirmed
       {
		   buzzer_on();_delay_ms(250);buzzer_off();	
	   }		   
       else
       {
         if (Avg_Raw>350)  //Not connected to head or Raw data values indication
         {
			PORTJ=0xFF;
           On_Flag==0;Off_Flag==1;
         }	 
       }
	   }	   
     
	 else
     {
       PORTJ=0x00;
     }  
	 }	      
   else    //Device is paired
   {
     PORTJ=0x01;
   }
	    
   i++;
   if(i<=10000)
   {
	   buzzer_on();
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
			 if(Plength == 4)   // Small Packet
			 {
				 
				 Small_Packet ();
			 }
			 else if(Plength == 32)   // Big Packet
			 {
				 Big_Packet ();
			 }
		 }			 
		 }
 } 
	 }