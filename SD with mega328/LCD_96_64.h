/*
 * LCD_96_64.h
 *
 * Created: 2015/10/25 12:36:51
 *  Author: stone
 */ 


#ifndef LCD_96_64_H_
#define LCD_96_64_H_
#define uchar unsigned char
#define SET_LCD_RST	PORTC|=(1<<PC5)		//��λ
#define CLR_LCD_RST	PORTC&=~(1<<PC5)
#define SET_LCD_CS	PORTC|=(1<<PC3)		//Ƭѡ
#define CLR_LCD_CS	PORTC&=~(1<<PC3)
#define SET_LCD_DC	PORTC|=(1<<PC2)		//����/����
#define CLR_LCD_DC	PORTC&=~(1<<PC2)
#define SET_LCD_SDI	PORTC|=(1<<PC1)		//����
#define CLR_LCD_SDI	PORTC&=~(1<<PC1)
#define SET_LCD_SCK	PORTC|=(1<<PC4)		//ʱ��
#define CLR_LCD_SCK	PORTC&=~(1<<PC4)

#define NOP					0x00
#define Function_Set		0x20
#define Display_Control		0x08
#define Set_Y_Address		0x40
#define Set_X_Address		0x80
#define Set_V0		        0x80
#define Set_Test_Mode		0x30

#define DIS_PHONEANT_0      0				//����
#define DIS_PHONEANT_1      4				//�ź�1��
#define DIS_PHONEANT_2      5				//�ź�2��
#define DIS_PHONEANT_3      7				//�ź�3��
#define DIS_PHONEANT_4      8				//�ź�4��
#define DIS_PHONEANT_5      10
#define DIS_PHONE           20				//�绰
#define DIS_LOCK            30				//����
#define DIS_Gr              40				//Gr����
#define DIS_SMS             50				//���ŷ���
#define DIS_SHAKE           60				//�𶯷���
#define DIS_HOME            61				// ���ڷ���
#define DIS_HOME_ANT        65				//�������߷���
#define DIS_HOME_ANT_1      67				//��������1��
#define DIS_HOME_ANT_2      68				//��������2��
#define DIS_HOME_ANT_3      69				//��������3��
#define DIS_MUTE            70				//��������
#define DIS_ALAM            75				//���ӷ���
#define DIS_CELL_0          80				//��ص�������
#define DIS_CELL_1          94				//��ص���1��
#define DIS_CELL_2          95				//��ص���2��
#define DIS_CELL_3          93				//��ص���3��

#define DIS_ON              0xFF			//��ʾСͼ��
#define DIS_OFF				0x00			//�ر�Сͼ��

#include "font.h"

/*-----------------------------------------------------------------------
shiftout: ��SPI��ʽ�������1�ֽ�
���������data    ��Ҫ���з��͵��ֽ�
-----------------------------------------------------------------------*/
void shiftout(uchar data){
	uchar i=0;
	for (i=0;i<8;i++)
	{
		CLR_LCD_SCK;
		if (data&0x80)
		{
			SET_LCD_SDI;
		}
		else
		{
			CLR_LCD_SDI;
		}
		SET_LCD_SCK;
		data=data<<1;
	}
}
/*-----------------------------------------------------------------------
send_dat: ��������
���������	cmd		��Ҫ���͵��������ͣ�
			data    ��Ҫ���͵���������
-----------------------------------------------------------------------*/
void send_cmd(uchar cmd,uchar data)
{
	CLR_LCD_CS;
	CLR_LCD_DC;
	shiftout(cmd|data);
	SET_LCD_CS;	
}
/*-----------------------------------------------------------------------
send_dat: ��������
���������data    ��Ҫ���͵����ݣ�
-----------------------------------------------------------------------*/ 
void send_dat(uchar data)
{
	CLR_LCD_CS;
	SET_LCD_DC;
	shiftout(data);
	SET_LCD_CS;
}

/* ���� */
void cls(void)
{
	int i;
	send_cmd(Set_X_Address, 0);
	send_cmd(Set_Y_Address, 0);
	for(i=0;i<960;i++)
	send_dat(0x00);
	send_cmd(Set_X_Address, 0);
	send_cmd(Set_Y_Address, 0);
}
/* ��ʼ��LCD */
void LCDInit(void)
{
	CLR_LCD_RST;
	_delay_ms(10);
	SET_LCD_RST;
	_delay_ms(10);
	send_cmd(Function_Set,0x01);
	send_cmd(Set_V0,0b00000);			//�Աȶ�
	send_cmd(Set_Test_Mode,0x02);
	send_cmd(Function_Set,0x00);
	send_cmd(Display_Control,0x04);
	cls();
}
/*-----------------------------------------------------------------------
putch: ��LCD����ʾ�����ַ�
���������X��Y    ����ʾ�ַ�����ʼX��Y���ꣻ
          ch      ��Ҫ��ʾ���ַ�
-----------------------------------------------------------------------*/ 
void putch(uchar x,uchar y, unsigned int ch)
{
	unsigned char i;
	unsigned int temp=0;
	send_cmd(Set_X_Address,x);
	send_cmd(Set_Y_Address,y);
	temp=(ch-0x20)*5;
	for(i=0;i<5;i++)
	{
		send_dat(pgm_read_byte(FONT+temp+i));
	}	
}
/*-----------------------------------------------------------------------
putstr: ��LCD����ʾ�ַ���
���������X��Y    ����ʾ�ַ�����ʼX��Y���ꣻ
          *str    ��Ҫ��ʾ���ַ���
-----------------------------------------------------------------------*/ 
void putstr(uchar x, uchar y, char *str)
{
	while(*str!=0)
	{
		putch(x,y,*str++);
		x=x+6;
	}
}
/*-----------------------------------------------------------------------
LCD_set_XY: ���������ַ
���������X��Y    ��������Ļ��ַ��
-----------------------------------------------------------------------*/ 
void LCD_set_XY(uchar x,uchar y)
{
	send_cmd(Set_X_Address,x);
	send_cmd(Set_Y_Address,y);
}

/*-----------------------------------------------------------------------
LCD_write_chinese_string: ��LCD����ʾ����
���������X��Y    ����ʾ���ֵ���ʼX��Y���ꣻ
          ch_with �����ֵ���Ŀ��
          num     ����ʾ���ֵĸ�����  
          line    �����ֵ��������е���ʼ����
          row     ��������ʾ���м��
-----------------------------------------------------------------------*/ 
void LCD_write_chinese_string(uchar X, uchar Y,uchar ch_with,uchar num,uchar line,uchar row)
{
	unsigned char i,n;
	unsigned int temp=0;
	LCD_set_XY(X,Y);                             //���ó�ʼλ��
	for (i=0;i<num;)
	{
		for (n=0; n<ch_with*2; n++)              //дһ������
		{
			if (n==ch_with)                      //д���ֵ��°벿��
			{
				if (i==0) LCD_set_XY(X,Y-1);
				else
				LCD_set_XY((X+(ch_with+row)*i),Y-1);
			}
			temp=(line+i)*24+n;
			send_dat(pgm_read_byte(write_chinese+temp));
		}
		i++;
		LCD_set_XY((X+(ch_with+row)*i),Y);
	}
}
/*-----------------------------------------------------------------------
putdraw	: ��ʾ��Ļ�Ϸ�ͼ��
���������x: ��Ļ�Ϸ���ͼ���ַ
		  s: ͼ�꿪��״̬
-----------------------------------------------------------------------*/ 
void putdraw(uchar x,uchar s )
{
	send_cmd(Set_X_Address,x);
	send_cmd(Set_Y_Address,8);
	send_dat(s);
}

/*-----------------------------------------------------------------------
show_map	: ����Ļ����ʾͼ��
-----------------------------------------------------------------------*/ 
void show_map(void)
{
	unsigned int i,j,n=0,temp=0;
	for(i=0;i<8;i++)
	for(j=0;j<96;j++)
	{
		send_cmd(Set_Y_Address,i);
		send_cmd(Set_X_Address,j);
		temp=n++;
		send_dat(pgm_read_byte(BMP+temp));
	}
}






#endif /* LCD_96_64_H_ */