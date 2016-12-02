#ifndef _SD_H_
#define _SD_H_ 1

#include<avr/io.h>

//ATMEGA128 SPI����
#define DDR_SPI		DDRB
#define PORT_SPI	PORTB
#define SPI_PIN		PINB
#define SPI_SS		PORTB2
#define SPI_MOSI	PORTB3
#define SPI_MISO	PORTB4
#define SPI_SCK		PORTB5
#define SD_CS_L		PORT_SPI &= ~(1<<PORTB2)
#define SD_CS_H     PORT_SPI |= (1<<PORTB2)

#define SD_DET()            (1)//GET_BIT(PORTA,2)//����п�

/* Private define ------------------------------------------------------------*/
/* SD�����Ͷ��� */
#define SD_TYPE_MMC     0
#define SD_TYPE_V1      1
#define SD_TYPE_V2      2
#define SD_TYPE_V2HC    4


/* SD�������ݽ������Ƿ��ͷ����ߺ궨�� */
#define NO_RELEASE      0
#define RELEASE         1

/* SD��ָ��� */
#define CMD0    0       //����λ
#define CMD9    9       //����9 ����CSD����
#define CMD10   10      //����10����CID����
#define CMD12   12      //����12��ֹͣ���ݴ���
#define CMD16   16      //����16������SectorSize Ӧ����0x00
#define CMD17   17      //����17����sector
#define CMD18   18      //����18����Multi sector
#define ACMD23  23      //����23�����ö�sectorд��ǰԤ�Ȳ���N��block
#define CMD24   24      //����24��дsector
#define CMD25   25      //����25��дMulti sector
#define ACMD41  41      //����41��Ӧ����0x00
#define CMD55   55      //����55��Ӧ����0x01
#define CMD58   58      //����58����OCR��Ϣ
#define CMD59   59      //����59��ʹ��/��ֹCRC��Ӧ����0x00


#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */

unsigned char reading;   //��Ƕ�д,1:��

void SPI_Low(void);
void SPI_High(void);
void SPI_Init(void);
unsigned char SD_Write_Command(unsigned char cmd,unsigned long arg);
unsigned char SPI_RW(unsigned char dat);
unsigned char SD_Init(void);  //SD����ʼ��(SPI-MODE)
unsigned char SD_ReceiveData(unsigned char*data,unsigned int len,unsigned char release);
unsigned char SD_SendCommand(unsigned char cmd,unsigned long arg,unsigned char crc);
unsigned char SD_ReadSingleBlock(unsigned long sector,unsigned char*buffer);
unsigned char SD_ReadMultiBlock(unsigned long sector,unsigned char *buffer,unsigned char count);
unsigned char SD_WriteSingleBlock(unsigned long sector,const unsigned char *data);
unsigned char SD_WriteMultiBlock(unsigned long sector,const unsigned char*data,unsigned char count);
unsigned char SD_WaitReady(void);
unsigned long SD_GetCapacity(void);
unsigned char SD_GetCSD(unsigned char *csd_data);









#endif
