#include <avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
long unsigned payloaddata[]={0};
long unsigned payloadchecksum=0;
long unsigned checksum=0;
bool brainwave;
int attention;
int meditation;
int PoorQuality;
bool bigpacket;
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
void datarecieve()
{
    SIGNAL(SIG_USART2_RECV) 		// ISR for receive complete interrupt
{
	data = UDR2; 				//making copy of data from UDR2 in 'data' variable

	UDR2 = data; 				//echo data back to PC

}

}
void AA_Detection(){
int i=0;
while(i<=1)
{
 datarecieve();
		if(data == 0xAA)
        {
            i++;
        }
            else
            {
                i=0;
            }

}


void length()
{
 datarecieve();
 if(data==170){
    length();
 }
 else if(data>170)
 {
     cout<<"Something's wrong..."<<endl;
     AA_Detection();
 }
 else
 {
     int payloadlength=data;
     int i=0;
     for(i=0;i<payloadlength;i++)
        {
        datarecieve();
        payloaddata[i]=data;
        payloadchecksum +=payloaddata[i];
        }
        datarecieve();
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
            brainwave=false;
        }


        for(int i = 0; i < payloadLength; i++) { // Parse the payload
          switch (payloadData[i]) {
          case 02:
            i++;
            poorQuality = payloadData[i];
            cout<<"Poor Quality Signal detected"<<endl;
            break;
          case 03:
            cout<<"showing Heart-Rate"<<endl;
          case 04:
            i++;
            attention = payloadData[i];
            cout<<"Attention Level:"<<attention<<endl;
            checkData();
            break;
          case 05:
            i++;
            meditation = payloadData[i];
            cout<<"Meditation Level:"<<meditation<<endl;
            checkData();
            break;
          case 06:
            cout<<"Raw_wave detected"<<endl;
            break;
          case 07:
            cout<<"Raw_Marker"<<endl;
            break;
          case 0x80:
            cout<<"Raw_wave detected"<<endl;
            break;
          case 0x83:                         // ASIC EEG POWER INT
            cout<<"EEG Power detected"<<endl;
            break;
          case 0xAA:
            break;
          case 0x55:
            break;
          }                                 // switch
        }                                   // for loop
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
 port_init();  //Initializes all the ports
 uart2_init(); //Initialize UART1 for serial communication
 sei();   //Enables the global interrupts
}


int main(){
init_devices();
uart2_init(void);
AA_Detection();
length();
return 0;
}

