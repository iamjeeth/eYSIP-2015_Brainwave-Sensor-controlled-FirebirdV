/*
Program to control Firebird V using attention level and eye-blink of a person

*/
#define F_CPU 14745600
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#define   Theshold_Eyeblink 110
#define   EEG_AVG           70
volatile unsigned char attention[3]={0};
volatile unsigned char payloadDataS[5] = {0};
volatile unsigned char payloadDataB[32] = {0};
volatile unsigned char checksum=0,generatedchecksum=0;
volatile unsigned char Att_Avg;
unsigned int Raw_data,Poorquality,Plength,Eye_Enable=0,On_Flag=0,Off_Flag=1;
unsigned int timer_flag=0,eye_count=0,j,l=0,f,n=0,k=0,z=0,p=0,i=0;
long Temp,Temp1,Avg_Raw,Temp_Avg;
volatile unsigned int ShaftCountRight,ShaftCountLeft,flag;
 void soft_left_2 (void) //Left wheel backward, right wheel stationary
 {
	 motion_set(0x01);
 }
 void LED_bargraph_config (void)
 {
	 DDRJ = 0xFF;  //PORT J is configured as output
	 PORTJ = 0x00; //Output is set to 0
	 
	 //LCD
// 	 DDRC = DDRC | 0xF7;
// 	 PORTC = PORTC & 0x80;
 }

void left_encoder_pin_config (void)
{
	DDRE  = DDRE & 0xEF;  //Set the direction of the PORTE 4 pin as input
	PORTE = PORTE | 0x10; //Enable internal pull-up for PORTE 4 pin
}

//Function to configure INT5 (PORTE 5) pin as input for the right position encoder
void right_encoder_pin_config (void)
{
	DDRE  = DDRE & 0xDF;  //Set the direction of the PORTE 4 pin as input
	PORTE = PORTE | 0x20; //Enable internal pull-up for PORTE 4 pin
}

 
 void left_position_encoder_interrupt_init (void) //Interrupt 4 enable
 {
	 cli(); //Clears the global interrupt
	 EICRB = EICRB | 0x02; // INT4 is set to trigger with falling edge
	 EIMSK = EIMSK | 0x10; // Enable Interrupt INT4 for left position encoder
	 sei();   // Enables the global interrupt
 }

 void right_position_encoder_interrupt_init (void) //Interrupt 5 enable
 {
	 cli(); //Clears the global interrupt
	 EICRB = EICRB | 0x08; // INT5 is set to trigger with falling edge
	 EIMSK = EIMSK | 0x20; // Enable Interrupt INT5 for right position encoder
	 sei();   // Enables the global interrupt
 }

 //ISR for right position encoder
 ISR(INT5_vect)
 {
	 ShaftCountRight++;  //increment right shaft position count
 }


 //ISR for left position encoder
 ISR(INT4_vect)
 {
	 ShaftCountLeft++;  //increment left shaft position count
 }


 
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
void motion_set (unsigned char Direction)
{
	unsigned char PortARestore = 0;

	Direction &= 0x0F; 			// removing upper nibbel as it is not needed
	PortARestore = PORTA; 			// reading the PORTA's original status
	PortARestore &= 0xF0; 			// setting lower direction nibbel to 0
	PortARestore |= Direction; 	// adding lower nibbel for direction command and restoring the PORTA status
	PORTA = PortARestore; 			// setting the command to the port
}

void angle_rotate(unsigned int Degrees)
{
	float ReqdShaftCount = 0;
	unsigned long int ReqdShaftCountInt = 0;

	ReqdShaftCount =  Degrees/ 4.090; // division by resolution to get shaft count
	ReqdShaftCountInt =  ReqdShaftCount;
	ShaftCountRight = 0;
	ShaftCountLeft = 0;

	while (1)
	{
		if((ShaftCountRight >= ReqdShaftCountInt) | (ShaftCountLeft >= ReqdShaftCountInt))
		break;
	}
	stop(); //Stop robot
}
void right_degrees(unsigned int Degrees)
{
	// 88 pulses for 360 degrees rotation 4.090 degrees per count
	right(); //Turn right
	angle_rotate(Degrees);
}


//Function to initialize ports
void port_init()
{
	motion_pin_config();
	left_encoder_pin_config(); //left encoder pin config
	right_encoder_pin_config(); //right encoder pin config
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
void left (void) //Left wheel backward, Right wheel forward
{
	motion_set(0x05);
}
void timer5_init()
{
	TCCR5B = 0x00;	//Stop
	TCNT5H = 0xFF;	//Counter higher 8-bit value to which OCR5xH value is compared with
	TCNT5L = 0x01;	//Counter lower 8-bit value to which OCR5xH value is compared with
	OCR5AH = 0x00;	//Output compare register high value for Left Motor
	OCR5AL = 0xFF;	//Output compare register low value for Left Motor
	OCR5BH = 0x00;	//Output compare register high value for Right Motor
	OCR5BL = 0xFF;	//Output compare register low value for Right Motor
	OCR5CH = 0x00;	//Output compare register high value for Motor C1
	OCR5CL = 0xFF;	//Output compare register low value for Motor C1
	TCCR5A = 0xA9;	/*{COM5A1=1, COM5A0=0; COM5B1=1, COM5B0=0; COM5C1=1 COM5C0=0}
 					  For Overriding normal port functionality to OCRnA outputs.
				  	  {WGM51=0, WGM50=1} Along With WGM52 in TCCR5B for Selecting FAST PWM 8-bit Mode*/
	
	TCCR5B = 0x0B;	//WGM12=1; CS12=0, CS11=1, CS10=1 (Prescaler=64)
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
void right (void) //Left wheel forward, Right wheel backward
{
	motion_set(0x0A);
}
//Function used for turning robot by specified degrees

//Function to take data from ISR byte by byte
char USART1_RX_vect()
{
	while(!(UCSR1A & (1<<RXC1)));
	return UDR1;
}
void stop (void) //hard stop
{
	motion_set(0x00);
}

void velocity (unsigned char left_motor, unsigned char right_motor)
{
	OCR5AL = (unsigned char)left_motor;
	OCR5BL = (unsigned char)right_motor;
}
void forward (void) //both wheels forward
{
	motion_set(0x06);
}
void run(){
	 if(Att_Avg>=1 && Att_Avg<=10){
		 PORTJ=0X01;
		 stop();
		 }
	 else if(Att_Avg>10 && Att_Avg<=30){
		 PORTJ=0x03;
		 velocity(170,170);
		 forward();
	 }
	 else if(Att_Avg>30 && Att_Avg<=40){
		 PORTJ=0X07;
		 velocity(200,200);
		 forward();
	 }
	 else if(Att_Avg>40 && Att_Avg<=50){
		 PORTJ=0X0F;
		 velocity(220,220);
		 forward();
	 }
	 else if(Att_Avg>50 && Att_Avg<=60){
		 PORTJ=0X1F;
		 velocity(220,220);
		 forward();
	 }
	 else if(Att_Avg>60 && Att_Avg<=70){
		 PORTJ=0X3F;
		 velocity(230,230);
		 forward();
	 }
	 else if(Att_Avg>70 && Att_Avg<=80){
		 PORTJ=0X7F;
		 velocity(240,240);
		 forward();
	 }
	 else if(Att_Avg>80 && Att_Avg<=100){
		 PORTJ=0xFF;
		 velocity(240,240);
		 forward();
	 }
} 
 
 
 void checkData(){
	 if(p<2){
	 run();	
	 }	 	  
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
   if(checksum == generatedchecksum)        // Varify Checksum
   { 	   
     if (j<100)
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
	  
	  if(checksum == generatedchecksum)        // Varify Checksum
	  {
		   if (payloadDataB[28]==4)
		   {
			   if (f<2)
			   {
				   attention [k] = payloadDataB[29];
				   Temp1 += attention [k];
				   f++;
			   }
			   else
			   {
				   Att_Avg = Temp1/2;
				   checkData();
				   f=0;
				   Temp1=0;
			   }
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
  }

 void Onesec_Rawval_Fun ()
 {
   Avg_Raw = Temp/100;
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
       if ((Avg_Raw>Theshold_Eyeblink) && (Avg_Raw<350))
       {
		   p++;
		   buzzer_on();
		   _delay_ms(500);
		   buzzer_off();
		   switch (p){
		   case 2:
			   stop();
			   velocity(0,170);
		       forward();
			   break;
		   case 3:
		       stop();
		       run();
			   p=0;
			   break;
		   }		   	
		   	  
	   }
       else
       {
         if (Avg_Raw>350)  //Raw data values indication
         {
			buzzer_on();_delay_ms(50);buzzer_off();
			stop();		
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
	 stop();
   }
	    
 }
void init_devices(void)
{
	cli(); //Clears the global interrupts
	port_init();  //Initializes all the ports
	uart1_init(); //Initialize UART1 for serial communication
	timer5_init();
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