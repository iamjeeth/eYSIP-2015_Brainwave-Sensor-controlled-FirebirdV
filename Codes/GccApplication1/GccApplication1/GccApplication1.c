#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#include"lcd.h"
#define F_CPU 14745600
long unsigned payloaddata[]={0};
long unsigned payloadchecksum=0;
long unsigned checksum=0;
int attention;
int meditation;
int PoorQuality;
int i;
unsigned char data;

void LED_bargraph_config (void)
{
	DDRJ = 0xFF;  //PORT J is configured as output
	PORTJ = 0x00; //Output is set to 0
}


//Function To Initialize UART2
// desired baud rate:9600
// actual baud rate:9600 (error 0.0%)
// char size: 8 bit
// parity: Disabled
void uart2_init(void)
{
 UCSR2B = 0x00; //disable while setting baud rate
 UCSR2A = 0x00;
 UCSR2C = 0x06;
 UBRR2L = 0x5F; //set baud rate lo
 UBRR2H = 0x00; //set baud rate hi
 UCSR2B = 0x98;
}


    SIGNAL(SIG_USART2_RECV) 		// ISR for receive complete interrupt
{
	
	data = UDR2; 				//making copy of data from UDR2 in 'data' variable

					//echo data back to PC

}


void AA_Detection()             //[SYNC] detection for 
{

  int q=0;
  while(q<=1)

{
 SIGNAL(SIG_USART2_RECV);
		if(data == 0xAA)
        {
            q++;
        }
            else
            {
                q=0;
            }

}
length();
}

void length()
{
 SIGNAL(SIG_USART2_RECV);
 if(data==170){
    length();
 }
 else if(data>170)
 {
     lcd_string("Something is wrong");
	 AA_Detection();
 }
 else
 {
     int payloadlength=data;
     for(i=0;i<payloadlength;i++)
        {
        SIGNAL(SIG_USART2_RECV);
        payloaddata[i]=data;
        payloadchecksum +=payloaddata[i];
        }
        SIGNAL(SIG_USART2_RECV);
        checksum=data;
        payloadchecksum=255-payloadchecksum;
        if(checksum==payloadchecksum)
        {
            attention=0;
            meditation=0;
            PoorQuality=200;
        }
        else
        {
            AA_Detection();
        }


        for(int i = 0; i < payloadlength; i++) { // Parse the payload
          switch (payloaddata[i]) {
          case 02:
            i++;
            PoorQuality = payloaddata[i];
            lcd_string("Poor Quality signal");
            break;
          case 03:
		    lcd_string("Heart Rate");
            break;
          case 04:
            i++;
            attention = payloaddata[i];
            lcd_string("Attention");
			lcd_print(2,1,payloaddata[i],3);
            checkData();
            break;
          case 05:
            i++;
            meditation = payloaddata[i];
            lcd_string("Meditation");
			lcd_print(2,1,payloaddata[i],3);
            checkData();
            break;
          case 06:
            lcd_string("Raw value");
            break;
          case 07:
            lcd_string("Raw value");
            break;
          case 0x80:
         
            break;
          case 0x83:                         // ASIC EEG POWER INT
            lcd_string("EEG power");
            break;
          case 0xAA:
            break;
          case 0x55:
            break;
          }                                 // switch
        }                                   // for loop
AA_Detection();
 }
}

void checkData(){
            if(payloaddata[i]<=20)
                PORTJ=0X01;
            if(payloaddata[i]>20 && payloaddata[i]<=30)
                PORTJ=0x03;
            if(payloaddata[i]>30 && payloaddata[i]<=40)
                PORTJ=0X07;
            if(payloaddata[i]>40 && payloaddata[i]<=50)
                PORTJ=0X0F;
            if(payloaddata[i]>50 && payloaddata[i]<=60)
                PORTJ=0X1F;
            if(payloaddata[i]>60 && payloaddata[i]<=70)
                PORTJ=0X3F;
            if(payloaddata[i]>70 && payloaddata[i]<=80)
                PORTJ=0X7F;
            if(payloaddata[i]>80 && payloaddata[i]<=100)
                PORTJ=0xFF;
        }



void init_devices()
{
 cli(); //Clears the global interrupts
 LED_bargraph_config();//Initialize ports
 uart2_init(); //Initialize UART2 for serial communication
 sei();   //Enables the global interrupts
}


int main(){
init_devices();
uart2_init();
AA_Detection();
while(1);
}

