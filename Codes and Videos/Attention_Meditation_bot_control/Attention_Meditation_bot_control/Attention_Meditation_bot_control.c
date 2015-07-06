/*
Members: Omkar Rajendra Mohite
         Ashish Kumar Jain
Mentors: Mehul Makwana
         Rutuja Ekatpure
Program on controlling Firebird V using attention and meditation level.
Depending upon variation of attention level, velocity of the bot will also change.
If mediatation of a person reaches above 70% the bot will take left turn
Meditation of such high level can be achieved by closing eyes and taking long breathe and concentrating on it.
*/
#define F_CPU 14745600
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#define   EEG_AVG           60
 int x=0;
volatile unsigned char payloadDataS[5] = {0};
volatile unsigned char attention[3]={0}, meditation[3]={0};
volatile unsigned char payloadDataB[32] = {0};
volatile unsigned char checksum=0,generatedchecksum=0;
volatile unsigned char Att_Avg, Med_Avg;
unsigned int Raw_data,Poorquality,Plength,Eye_Enable=0,On_Flag=0,Off_Flag=1;
unsigned int timer_flag=0,eye_count=0,j,l=0,n=0,z=0,p=0,i=0;
unsigned int Plength;
unsigned int f,a,b=0,k=0;
long Temp,Temp1,Temp2,Avg_Raw,Temp_Avg;
volatile unsigned int ShaftCountRight,ShaftCountLeft;

//LED bargraph configurationn
void LED_bargraph_config (void)
{
	DDRJ = 0xFF;  //PORT J is configured as output
	PORTJ = 0x00; //Output is set to 0
}
//Buzzer cnfiguration
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


//Initialization of buzzer and LED bargraph 
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
//For taking left turn
void left (void) //Left wheel backward, Right wheel forward
{
	motion_set(0x05);
}
//Timer 5 initialized for velocity changes
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
//for right turn
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
//stop
void stop (void) //hard stop
{
	motion_set(0x00);
}
//Velocity
void velocity (unsigned char left_motor, unsigned char right_motor)
{
	OCR5AL = (unsigned char)left_motor;
	OCR5BL = (unsigned char)right_motor;
}
//Forward
void forward (void) //both wheels forward
{
	motion_set(0x06);
}

//function for detecting various levels of attention 
void checkData(){
	if(Att_Avg>=1 && Att_Avg<=10){  //wandering level attention
		PORTJ=0X01;
		stop();
	}
	else if(Att_Avg>10 && Att_Avg<=30){ //Poor level of Attention
		PORTJ=0x03;
		velocity(210,210);
		forward();
	}
	else if(Att_Avg>30 && Att_Avg<=40){ //Attention level building up
		PORTJ=0X07;
		velocity(210,210);
		forward();
	}
	else if(Att_Avg>40 && Att_Avg<=50){ //Neutral
		PORTJ=0X0F;
		velocity(220,220);
		forward();
	}
	else if(Att_Avg>50 && Att_Avg<=60){ //Neutral
		PORTJ=0X1F;
		velocity(220,220);
		forward();
	}
	else if(Att_Avg>60 && Att_Avg<=70){ //Slightly elevated
		PORTJ=0X3F;
		velocity(230,230);
		forward();
	}
	else if(Att_Avg>70 && Att_Avg<=80){ //Slightly elevated
		PORTJ=0X7F;
		velocity(240,240);
		forward();
	}
	else if(Att_Avg>80 && Att_Avg<=100){ //Elevated
		PORTJ=0xFF;
		velocity(240,240);
		forward();
	}
}
//Function after detection of 0x04 payloadlength
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
	if(checksum == generatedchecksum)           // Verify Checksum
	{
		if (j<100)								//Taking average of 100 data packets sample
		{
			Raw_data  = ((payloadDataS[2] <<8)| payloadDataS[3]); //Checking of raw data values
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
//Function to verify eye-blink & rawdata values
 void Onesec_Rawval_Fun ()
 {
	 Avg_Raw = Temp/100;
	 if (On_Flag==0 && Off_Flag==1)
	 {
		 if (n<3)    //Taking 3 samples of average values obtained from 100 data packets 
		 {
			 Temp_Avg += Avg_Raw;
			 n++;
		 }
		 else
		 {
			 Temp_Avg = Temp_Avg/3;  //Taking average
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
 //Function to detect Raw data values
 void Eye_Blink ()
 {
	 if (Eye_Enable)
	 {
		 if (On_Flag==1 && Off_Flag==0)
		 {
				 if (Avg_Raw>350)  //Raw data values indication
				 {
					 buzzer_on();_delay_ms(50);buzzer_off();
					 stop();
					 On_Flag==0;Off_Flag==1;
				 }
			 }
		 }
	 else    //Device is paired
	 {
		 x++;
		 if(x<5){
		 buzzer_on();_delay_ms(50);buzzer_off();
		 }		 
	 }
 }
 //After detecting 0x20 payloadlength 
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
		if(payloadDataB[28]==4){             //Detecting attention signal
		if(a<2){
			attention [b] = payloadDataB[29];  //Detecting attention level
			Temp1 += attention [b];
			a++;
		}
		else{
			Att_Avg = Temp/2;
			checkData();
			a=0;
			Temp1=0;
		}
		}
		
		if (payloadDataB[30]==5) //Checking for meditation signal 
		{
			if (f<2)
			{
				meditation [k] = payloadDataB[31];  //Detecting meditation level
				Temp2 += meditation [k];
				f++;
			}
			else
			{
				Med_Avg = Temp2/2;
				if(Med_Avg>70)  //if meditation level is more than 70% take left turn
				   {
					buzzer_on();					
					stop();
					velocity(240,240);
					left();
					_delay_ms(700);
					stop();
					velocity(190,190);
					forward();
					checkData();
					buzzer_off();
					
			       }
				f=0;
				Temp2=0;
			}
		}
		 Poorquality = payloadDataB[1]; //Check for poor quality signal to be zero
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
//initialization
void init_devices(void)
{
	cli(); //Clears the global interrupts
	port_init();  //Initializes all the ports
	uart1_init();
	timer5_init(); //Initialize UART1 for serial communication
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