#include "diskio.h"
#include "MMC.h"
#include "SerialPort.h"


unsigned char disk_initialize(unsigned char drv)

{
	unsigned char state;

	if(drv)
	{
		 return STA_NOINIT;  //��֧�ִ���0�Ĳ���
	}

	state = SD_Init();      //SD���ĳ�ʼ��
	 
	 
	
	if(state == STA_NODISK)
    {
        return STA_NODISK;
    }
    else if(state != 0)
    {
	
		Serial_SendString("InitializeFail\r\n");
        return STA_NOINIT;  //�������󣺳�ʼ��ʧ��
    }
    else
    {
	
		//Serial_SendString("InitializeSuccess\r\n");
        return 0;           //��ʼ���ɹ�
    }


	
}



/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */
//Get disk status
DSTATUS disk_status (               
	BYTE drv		/* Physical drive nmuber (0..) */
)
{
    if(drv)
    {
        return STA_NOINIT;  //��֧�ִ���0����
    }

    //���SD���Ƿ����
    if(!SD_DET())
    {
        return STA_NODISK;
    }
    return 0;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */

DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..255) */
)
{
	unsigned char res=0;
	//count=count;      //��֪Ϊ��,count�ᱻ�Ż�Ϊ0,�ʼ����������
	//sector=sector;
    if ((drv) || (!count))
    {    
		Serial_Send(5+0x30);
        return RES_PARERR;  //��֧�ֵ����̲�����count���ܵ���0�����򷵻ز�������
    }
    if(!SD_DET())
    {
        return RES_NOTRDY;  //û�м�⵽SD������NOT READY����
    }

    
	
    if(count==1)            //1��sector�Ķ�����      
    {                                                
        res = SD_ReadSingleBlock(sector, buff);      
    }                                                
    else                    //���sector�Ķ�����     
    {                                                
        res = SD_ReadMultiBlock(sector, buff, count);
    }                                                
	/*
    do                           
    {                                          
        if(SD_ReadSingleBlock(sector, buff)!=0)
        {                                      
            res = 1;                           
            break;                             
        }                                      
        buff+=512;                             
    }while(--count);                                         
    */
    //������ֵ����SPI_SD_driver.c�ķ���ֵת��ff.c�ķ���ֵ
    if(res == 0x00)
    {
        return RES_OK;
    }
    else
    {
        return RES_ERROR;
    }
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	        /* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..255) */
)
{
	unsigned char res;
//	count = 1;
//	sector = 2;

    if (drv || !count)
    {    
        return RES_PARERR;  //��֧�ֵ����̲�����count���ܵ���0�����򷵻ز�������
    }

    if(!SD_DET())
    {
        return RES_NOTRDY;  //û�м�⵽SD������NOT READY����
    }


    // ��д����
    if(count == 1)
    {
        res = SD_WriteSingleBlock(sector, buff);
    }
    else
    {
        res = SD_WriteMultiBlock(sector, buff, count);
    }
    // ����ֵת��
    if(res == 0)
    {
        return RES_OK;
    }
    else
    {
        return RES_ERROR;
    }
}
#endif /* _READONLY */

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */

DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)

{
    DRESULT res;


    if (drv)
    {    
        return RES_PARERR;  //��֧�ֵ����̲��������򷵻ز�������
    }
    
    //FATFSĿǰ�汾���账��CTRL_SYNC��GET_SECTOR_COUNT��GET_BLOCK_SIZ��������
    switch(ctrl)
    {
    case CTRL_SYNC:
        SD_CS_L;
        if(SD_WaitReady()==0)
        {
            res = RES_OK;
        }
        else
        {
            res = RES_ERROR;
        }
        SD_CS_H;
        break;
        
    case GET_BLOCK_SIZE:
    case GET_SECTOR_SIZE:
        *(WORD*)buff = 512;
        res = RES_OK;
        break;

    case GET_SECTOR_COUNT:
        *(DWORD*)buff = SD_GetCapacity();
        res = RES_OK;
        break;
    default:
        res = RES_PARERR;
        break;
    }

    return res;
}


/*-----------------------------------------------------------------------*/
/* User defined function to give a current time to fatfs module      */
/* 31-25: Year(0-127 org.1980), 24-21: Month(1-12), 20-16: Day(1-31) */
/* 15-11: Hour(0-23), 10-5: Minute(0-59), 4-0: Second(0-29 *2) */ 

DWORD get_fattime (void)
{
 /*   struct tm t;
    DWORD date;
    t.tm_year=2009-1980;		//��ݸ�Ϊ1980����
    t.tm_mon=8;         	//0-11�¸�Ϊ1-12��
	t.tm_mday=3;
	t.tm_hour=15;
	t.tm_min=30;
    t.tm_sec=20;      	//��������Ϊ0-29
	date =t.tm_year;
	date=date<<7;
	date+=t.tm_mon;
	date=date<<4;
	date+=t.tm_mday;
	date=date<<5;
	date+=t.tm_hour;
	date=date<<5;
	date+=t.tm_min;
	date=date<<5;
	date+=t.tm_sec;
	date=1950466004;
*/
    return 1950466005;
}
