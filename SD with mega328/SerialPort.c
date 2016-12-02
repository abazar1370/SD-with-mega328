/*
 * SerialPort.c
 * Only For Mega328
 * Created: 2016/4/30 10:26:51
 *  Author: Stone
 */ 


#define F_CPU 16000000UL
#include "avr/io.h"
#include "SerialPort.h"

/*
    ���ڳ�ʼ������    
*/
void Serial_Init(unsigned long BAUD)
{	
	/*    ���ò�����    */
	UBRR0L= (unsigned char)((F_CPU/BAUD/16)-1)%256;
	UBRR0H= (unsigned char)((F_CPU/BAUD/16)-1)/256;
	/*    ʹ�ܽ����жϣ�ʹ�ܽ������뷢����    */
	UCSR0B=(1<<RXEN0)|(1<<TXEN0);
	/*    ����֡��ʽ ��  8������λ��1��ֹͣλ    */
	UCSR0C=(1<<UCSZ01)|(1<<UCSZ00);	
}

void Serial_Send(unsigned char data)
{
	/* �ȴ����ͻ�����Ϊ�� */
	while ( !( UCSR0A & (1<<UDRE0)) )
	;
	/* �����ݷ��뻺�������������� */
	UDR0 = data;
}

void Serial_SendString(char *s)
{
	while(*s)
	{
		Serial_Send(*s);
		s++;
	}
}