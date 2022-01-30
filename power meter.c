/*
 * power_meter.c
 *
 * Created: 4/25/2016 10:52:16 PM
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

void delay (char x);

void port_initial(void);

void usart_initial(void);
char usart_receive (void);
void usart_transmit (char t);
void usart_string (const char *s);

void lcd_initial(void);
void lcd_command (char c);
void lcd_data (char d);
void lcd_string (const char *p);

void hex_ascii (char u);

char con[5];
char count=0,unit=0;
char msg_loc,dummy_data,m;
char rec[15],number[12];
char msg_data[10];

int main(void)
{	
	port_initial();
	usart_initial();
	lcd_initial();
	
	GICR|=(1<<INT0);
	MCUCR=0X02;
	cli();
	
	lcd_string("  POWER METER   ");
	lcd_command(0xc0);
	lcd_string("DoneBy: IIET-CSE");
	delay(150);
	
	usart_string("AT+CMGD=1");                     
	usart_transmit(0x0D);
	usart_transmit(0x0A);                       
	delay(100);
	
	lcd_command(0x01);	
	lcd_string("U: 000    C: 000");
	
	sei();
			
    while(1)
    {
		while(usart_receive()!='+');
		
		cli();
		
		for(m=0;m<12;m++)
	    {
	   		rec[m]=usart_receive();                        
	   	}
	    msg_loc=rec[11];                 
	    lcd_command(0x01);
	    lcd_string("msg received");  
		delay(5);           
	    usart_string("AT+CMGR=");                 
	    usart_transmit(msg_loc);
		delay(10);                        
	   	usart_transmit(0x0D);
	    usart_transmit(0x0A); 
		                  
		while(usart_receive()!='+');
		while(usart_receive()!='+');
		for(m=0;m<2;m++)                              
	    {
			dummy_data=usart_receive(); 
	    }
		for(m=0;m<10;m++)
	    {
	   		number[m]=usart_receive();                               
	   	}
		number[m]='\0';
		while(usart_receive()!=0x0A);
		m=0;
		while((msg_data[m++]=usart_receive())!=0X0D);             
		msg_data[m-1]='\0';
		delay(5);
	    lcd_string("  ok");
		delay(100);
		
		lcd_command(0xc0);
		lcd_string("Processing......");
		delay(200);

		if(msg_data[0]=='R')
		{
			lcd_command(0x01);
			lcd_string("msg sending.....");  
			usart_string("AT+CMGS=");
			usart_transmit('"'); 
			delay(30);              
			usart_string(number);
			usart_transmit('"'); 
			delay(30);                          
			usart_transmit(0x0D);
			usart_transmit(0x0A);                      
			delay(30);
			usart_string(" Unit: ");
			delay(30);
			hex_ascii(unit);
			usart_string(con);
			usart_transmit(0x1A);
			delay(10); 
			usart_transmit(0x1A); 
			usart_transmit(0x1A);
			delay(10);         
			while(usart_receive()!='+'); 
			lcd_command(0xc0);
			lcd_string(" msg  delivered "); 
			delay(200);
		}
		
		usart_string("AT+CMGD=");                     
		delay(10);
		usart_transmit(msg_loc);                    
		usart_transmit(0x0D);
		usart_transmit(0x0A);                       
		delay(250);
		
		lcd_command(0x01);	
		lcd_string("U: 000    C: 000");
		
		sei();
	}
}

ISR(INT0_vect)
{
	cli();
	
	count++;        
	if(count>=5)       
	{	
     	unit++;            
    	count=0;
	}
	lcd_command(0x83);                    
	hex_ascii(unit);                           
	lcd_string(con);
	
	hex_ascii(count);
	lcd_command(0x8d);                             
	lcd_string(con);

	sei();
}

void delay (char x)
{
	int i,j;
	for(j=0;j<x;j++)
	for(i=0;i<1000;i++);
}

void port_initial(void)
{
	DDRA=0XFF;
	DDRB=0xFF;
	DDRC=0xFF;
	DDRD=0xF0;
	
	PORTD|=(1<<2);
}

void usart_initial(void)
{	
	UCSRA=0X00;
	UCSRB=0X18;
	UCSRC=0X06;
	UBRRL=0X19;
	UBRRH=0X00;
}

char usart_receive (void)
{
	while ((UCSRA & (1<<RXC))==0);
	UCSRA&=~(1<<RXC);
	return(UDR);
}

void usart_transmit (char t)
{
	UDR=t;
	while((UCSRA & (1<<TXC))==0);
	UCSRA|=(1<<TXC);
}

void usart_string(const char *s)
{
	while(*s)
	{
		usart_transmit(*s++);
		delay(30);
	}
}

void lcd_initial(void)
{
	lcd_command(0X38);
	lcd_command(0X80);
	lcd_command(0X0C);
	lcd_command(0X06);
	lcd_command(0X01); 
	delay(5);
}

void lcd_command(char c)
{
	PORTC=c;
    PORTD&=~(1<<6);
	PORTD|=(1<<7);
	delay(2);
    PORTD&=~(1<<7);
}

void lcd_data(char d)
{
	PORTC=d;
    PORTD|=(1<<6);
	PORTD|=(1<<7);
	delay(2);
    PORTD&=~(1<<7);
}

void lcd_string(const char *p)
{
	while(*p)
	{
		lcd_data(*p++);
	}
}

void hex_ascii (char u)
{ 
    con[3]='\0';
	con[2]=(u%10)+0x30;
	u=u/10;
    con[1]=(u%10)+0x30;
    con[0]=(u/10)+0x30;
}
