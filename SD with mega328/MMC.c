#define F_CPU 16000000UL
#include <util/delay.h>
#include "MMC.h"



unsigned char SD_Type=0 ;

void SPI_Low(void)		
{//SPI����ģʽ
	SPCR = 0;
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0) | (1 << SPR1);
	SPSR &= ~(1 << SPI2X);
	//ʹ��SPI,������ʽ,MSB��ǰ,ģʽ0,128��Ƶ
}

void SPI_High(void)
{//SPI����ģʽ
	SPCR = 0;
	SPCR = (1 << SPE) | (1 << MSTR);
	SPSR |= (1 << SPI2X);
	//ʹ��SPI,������ʽ,MSB��ǰ,ģʽ0,4��Ƶ,2��Ƶ
}


void SPI_Init(void)
{//SPI��ʼ��
	PORT_SPI |= (1 << SPI_SS) | (1 << SPI_MISO);		//��SS��λ�������,MISO���������
	DDR_SPI &= ~((1 << SPI_SS) | (1 << SPI_MOSI) | (1 << SPI_MISO)| (1 << SPI_SCK));
	DDR_SPI |= (1 << SPI_SS) | (1 << SPI_MOSI) | (1 << SPI_SCK);
	//��SS SCK MOSI��Ϊ���
}


unsigned char SPI_RW(unsigned char dat)
{
	SPDR = dat;
	while(!(SPSR & (1 << SPIF)))
		;
	return (SPDR);
}

//****************************************************************************
//SD����ʼ��(SPI-MODE)
//****************************************************************************
unsigned char SD_Init(void)
{


	unsigned char i,temp;
    unsigned int retry; 
	

	SPI_Init();
	SPI_Low();

	SD_CS_L;
	_delay_ms(1);       //Wait MMC/SD ready...

	for(i=0;i<0xff;i++)
	{
		SPI_RW(0xff);   //send 74 clock at least!!!
	}

	retry = 0;

	do
	{
		temp=SD_Write_Command(0,0);//retry 200 times to send CMD0 command 
		retry++;
		if(retry==2000) return 0xff;//CMD0 Error! 

	}while(temp!=1);      //temp=0x01��˵��SD���������״̬��λ�ɹ�

	retry = 0;
	do
	{
		temp=SD_Write_Command(1,0);
		retry++;
		if(retry >= 2000) return 0xff;  //retry 100 times to send CMD1 command 
	}while(temp!=0);    //temp=0˵��SD���Խ��в��� ����ɹ�

	retry = 0;
	SD_Write_Command(16,512);     //����һ�ζ�дBLOCK�ĳ���Ϊ512���ֽ� CMD16
	SD_CS_H;   //MMC_CS_PIN=1;  //set MMC_Chip_Select to high �ر�Ƭѡ
    return(0); //All commands have been taken.


}


//****************************************************************************
//Send a Command to MMC/SD-Card
//Return: the second byte of response register of MMC/SD-Card
//****************************************************************************
unsigned char SD_Write_Command(unsigned char cmd,unsigned long arg)
{
	unsigned char tmp;
	unsigned char retry=0;
	
	SD_CS_H;   //�ر�SD��
	SPI_RW(0xff);
	SD_CS_L;
	SPI_RW(cmd|0x40);   //��ͷ����
	SPI_RW(arg>>24);  //��ߵ�8λ
	SPI_RW(arg>>16);   //send 6 Byte Command to MMC/SD-Card
	SPI_RW(arg>>8);
	SPI_RW(arg&0xff);
	SPI_RW(0x95);      //������RESET��Ч��CRCЧ����
		//get 8 bit response 

	do
	{
		tmp = SPI_RW(0XFF);
		retry++;
	}while((tmp==0xff)&&(retry<100));

	if(reading==0) SD_CS_H;
	else
	SD_CS_L;
	return(tmp);

		
}

/*******************************************************************************
* Function Name  : SD_SendCommand
* Description    : ��SD������һ������
* Input          : unsigned char cmd   ����
*                  unsigned long arg  �������
*                  unsigned char crc   crcУ��ֵ
* Output         : None
* Return         : unsigned char r1 SD�����ص���Ӧ
*******************************************************************************/
unsigned char SD_SendCommand(unsigned char cmd,unsigned long arg,unsigned char crc)
{
    unsigned char r1 ;
    unsigned char Retry=0 ;
    

    SPI_RW(0XFF);
    //Ƭѡ���õͣ�ѡ��SD��
    SD_CS_L;   //PC0 = 0
    
    //����
    SPI_RW(cmd|0x40);
    //�ֱ�д������
    SPI_RW(arg>>24);
    SPI_RW(arg>>16);
    SPI_RW(arg>>8);
    SPI_RW(arg);
    SPI_RW(crc);
    
    //�ȴ���Ӧ����ʱ�˳�
    while((r1=SPI_RW(0xFF))==0xFF)
    {
        Retry++;
        if(Retry>200)
        {
        break ;
        }
    }

    //�ر�Ƭѡ
    SD_CS_H;
    //�������϶�������8��ʱ�ӣ���SD�����ʣ�µĹ���
    SPI_RW(0xFF);
    
    //����״ֵ̬
    return r1 ;
}

/*******************************************************************************
* Function Name  : SD_ReceiveData
* Description    : ��SD���ж���ָ�����ȵ����ݣ������ڸ���λ��
* Input          : unsigned char *data(��Ŷ������ݵ��ڴ�>len)
*                  unsigned int len(���ݳ��ȣ�
*                  unsigned char release(������ɺ��Ƿ��ͷ�����CS�ø� 0�����ͷ� 1���ͷţ�
* Output         : None
* Return         : unsigned char
*                  0��NO_ERR
*                  other��������Ϣ
*******************************************************************************/
unsigned char SD_ReceiveData(unsigned char*data,unsigned int len,unsigned char release)
{
    unsigned int retry ;
    unsigned char r1 ;
    
    // ����һ�δ���
    SD_CS_L;
    //�ȴ�SD������������ʼ����0xFE
    retry=0 ;
    do 
    {
        r1=SPI_RW(0xFF);
        retry++;
        //2000�εȴ���û��Ӧ���˳�����
        if(retry>200)
        {
            SD_CS_H;
            return 1 ;
        }
    }
    while(r1!=0xFE);
    //��ʼ��������
    while(len--)
    {
        *data=SPI_RW(0xFF);
        data++;
    }
    //������2��αCRC��dummy CRC��
    SPI_RW(0xFF);
    SPI_RW(0xFF);
    //�����ͷ����ߣ���CS�ø�
    if(release==RELEASE)
    {
        //�������
        SD_CS_H;
        SPI_RW(0xFF);
    }
    return 0 ;
}


/*******************************************************************************
* Function Name  : SD_ReadSingleBlock
* Description    : ��SD����һ��block
* Input          : unsigned long sector ȡ��ַ��sectorֵ���������ַ��
*                  unsigned char *buffer ���ݴ洢��ַ����С����512byte��
* Output         : None
* Return         : unsigned char r1
*                   0�� �ɹ�
*                   other��ʧ��
*******************************************************************************/
unsigned char SD_ReadSingleBlock(unsigned long sector,unsigned char*buffer)
{
    unsigned char r1 ;
    
    //����Ϊ����ģʽ
    SPI_High();
    
    //�������SDHC����sector��ַת��byte��ַ
    sector=sector<<9 ;
    
    r1=SD_SendCommand(CMD17,sector,0);
    //������
    
    if(r1!=0x00)
    {
        return r1 ;
    }
    
    r1=SD_ReceiveData(buffer,512,RELEASE);
    if(r1!=0)
    {
        return r1 ;
        //�����ݳ���
    }
    else 
    {
        return 0 ;
    }
}

/*******************************************************************************
* Function Name  : SD_ReadMultiBlock
* Description    : ��SD���Ķ��block
* Input          : unsigned long sector ȡ��ַ��sectorֵ���������ַ��
*                  unsigned char *buffer ���ݴ洢��ַ����С����512byte��
*                  unsigned char count ������count��block
* Output         : None
* Return         : unsigned char r1
*                   0�� �ɹ�
*                   other��ʧ��
*******************************************************************************/
unsigned char SD_ReadMultiBlock(unsigned long sector,unsigned char *buffer,unsigned char count)
{
    unsigned char r1 ;
    
    //����Ϊ����ģʽ
    SPI_High();
    
    //�������SDHC����sector��ַת��byte��ַ
    sector=sector<<9 ;
    //SD_WaitReady();
    //�����������
    r1=SD_SendCommand(CMD18,sector,0);
    //������
    if(r1!=0x00)
    {
        return r1 ;
    }
    //��ʼ��������
    do 
    {
        if(SD_ReceiveData(buffer,512,NO_RELEASE)!=0x00)
        {
            break ;
        }
        buffer+=512 ;
    }
    while(--count);
    
    //ȫ��������ϣ�����ֹͣ����
    SD_SendCommand(CMD12,0,0);
    //�ͷ�����
    SD_CS_H;
    SPI_RW(0xFF);
    
    if(count!=0)
    {
        return count ;
        //���û�д��꣬����ʣ�����
    }
    else 
    {
        return 0 ;
    }
}

/*******************************************************************************
* Function Name  : SD_WriteSingleBlock
* Description    : д��SD����һ��block
* Input          : unsigned long sector ������ַ��sectorֵ���������ַ��
*                  unsigned char *buffer ���ݴ洢��ַ����С����512byte��
* Output         : None
* Return         : unsigned char r1
*                   0�� �ɹ�
*                   other��ʧ��
*******************************************************************************/
unsigned char SD_WriteSingleBlock(unsigned long sector,const unsigned char *data)
{
    unsigned char r1 ;
    unsigned int i ;
    unsigned int retry ;
    
    //����Ϊ����ģʽ
    SPI_High();
    
    //�������SDHC����������sector��ַ������ת����byte��ַ
    if(SD_Type!=SD_TYPE_V2HC)
    {
        sector=sector<<9 ;
    }
    
    r1=SD_SendCommand(CMD24,sector,0x00);
    if(r1!=0x00)
    {
        return r1 ;
        //Ӧ����ȷ��ֱ�ӷ���
    }
    
    //��ʼ׼�����ݴ���
    SD_CS_L;
    //�ȷ�3�������ݣ��ȴ�SD��׼����
    SPI_RW(0xff);
    SPI_RW(0xff);
    SPI_RW(0xff);
    //����ʼ����0xFE
    SPI_RW(0xFE);
    
    //��һ��sector������
    for(i=0;i<512;i++)
    {
        SPI_RW(*data++);
    }
    //��2��Byte��dummy CRC
    SPI_RW(0xff);
    SPI_RW(0xff);
    
    //�ȴ�SD��Ӧ��
    r1=SPI_RW(0xff);
    if((r1&0x1F)!=0x05)
    {
        SD_CS_H;
        return r1 ;
    }
    
    //�ȴ��������
    retry=0 ;
    while(!SPI_RW(0xff))
    {
        retry++;
        //�����ʱ��д��û����ɣ������˳�
        if(retry>0xfffe)
        {
            SD_CS_H;
            return 1 ;
            //д�볬ʱ����1
        }
    }
    
    //д����ɣ�Ƭѡ��1
    SD_CS_H;
    SPI_RW(0xff);
    
    return 0 ;
}

/*******************************************************************************
* Function Name  : SD_WaitReady
* Description    : �ȴ�SD��Ready
* Input          : None
* Output         : None
* Return         : unsigned char
*               0�� �ɹ�
*           other��ʧ��
*******************************************************************************/

unsigned char SD_WaitReady(void)
{
    unsigned char r1 ;
    unsigned char retry ;
    retry=0 ;
    do 
    {
        r1=SPI_RW(0xFF);
        if(retry==255)//������쳣������ѭ����
        {
            return 1 ;
        }
    }
    while(r1!=0xFF);
    
    return 0 ;
}


/*******************************************************************************
* Function Name  : SD_WriteMultiBlock
* Description    : д��SD����N��block
* Input          : unsigned long sector ������ַ��sectorֵ���������ַ��
*                  unsigned char *buffer ���ݴ洢��ַ����С����512byte��
*                  unsigned char count д���block��Ŀ
* Output         : None
* Return         : unsigned char r1
*                   0�� �ɹ�
*                   other��ʧ��
*******************************************************************************/
unsigned char SD_WriteMultiBlock(unsigned long sector,const unsigned char*data,unsigned char count)
{
    unsigned char r1 ;
    unsigned int i ;
    
    //����Ϊ����ģʽ
    SPI_High();
    
    //�������SDHC����������sector��ַ������ת����byte��ַ
    if(SD_Type!=SD_TYPE_V2HC)
    {
        sector=sector<<9 ;
    }
    //���Ŀ�꿨����MMC��������ACMD23ָ��ʹ��Ԥ����
    if(SD_Type!=SD_TYPE_MMC)
    {
        r1=SD_SendCommand(ACMD23,count,0x00);
    }
    //�����д��ָ��
    r1=SD_SendCommand(CMD25,sector,0x00);
    if(r1!=0x00)
    {
        return r1 ;
        //Ӧ����ȷ��ֱ�ӷ���
    }
    
    //��ʼ׼�����ݴ���
    SD_CS_L;
    //�ȷ�3�������ݣ��ȴ�SD��׼����
    SPI_RW(0xff);
    SPI_RW(0xff);
    SPI_RW(0xff);
    //--------������N��sectorд���ѭ������
    do 
    {
        //����ʼ����0xFC �����Ƕ��д��
        SPI_RW(0xFC);
        
        //��һ��sector������
        for(i=0;i<512;i++)
        {
            SPI_RW(*data++);
        }
        //��2��Byte��dummy CRC
        SPI_RW(0xff);
        SPI_RW(0xff);
        
        //�ȴ�SD��Ӧ��
        r1=SPI_RW(0xff);
        if((r1&0x1F)!=0x05)
        {
            SD_CS_H;
            //���Ӧ��Ϊ��������������ֱ���˳�
            return r1 ;
        }
        
        //�ȴ�SD��д�����
        if(SD_WaitReady()==1)
        {
            SD_CS_H;
            //�ȴ�SD��д����ɳ�ʱ��ֱ���˳�����
            return 1 ;
        }
        
        //��sector���ݴ������
    }
    while(--count);
    
    //��������������0xFD
    r1=SPI_RW(0xFD);
    if(r1==0x00)
    {
        count=0xfe ;
    }
    
    if(SD_WaitReady())
    {
        while(1){}
    }
    
    //д����ɣ�Ƭѡ��1
    SD_CS_H;
    SPI_RW(0xff);
    
    return count ;
    //����countֵ�����д����count=0������count=1
}

/*******************************************************************************
* Function Name  : SD_GetCSD
* Description    : ��ȡSD����CSD��Ϣ�������������ٶ���Ϣ
* Input          : unsigned char *cid_data(���CID���ڴ棬����16Byte��
* Output         : None
* Return         : unsigned char
*                  0��NO_ERR
*                  1��TIME_OUT
*                  other��������Ϣ
*******************************************************************************/
unsigned char SD_GetCSD(unsigned char *csd_data)
{
    unsigned char r1 ;
    
    //��CMD9�����CSD
    r1=SD_SendCommand(CMD9,0,0xFF);
    if(r1!=0x00)
    {
        return r1 ;
        //û������ȷӦ�����˳�������
    }
    //����16���ֽڵ�����
    SD_ReceiveData(csd_data,16,RELEASE);
    
    return 0 ;
}


/*******************************************************************************
* Function Name  : SD_GetCapacity
* Description    : ��ȡSD��������
* Input          : None
* Output         : None
* Return         : unsigned long capacity
*                   0�� ȡ��������
*******************************************************************************/
unsigned long SD_GetCapacity(void)
{
    unsigned char csd[16];
    unsigned long Capacity ;
    unsigned char r1 ;
    unsigned int i ;
    unsigned int temp ;
    
    //ȡCSD��Ϣ������ڼ��������0
    if(SD_GetCSD(csd)!=0)
    {
        return 0 ;
    }

    //���ΪSDHC�����������淽ʽ����
    if((csd[0]&0xC0)==0x40)
    {
        Capacity=((((unsigned long)csd[8])<<8)+(unsigned long)csd[9]+1)*(unsigned long)1024 ;
    }
    else 
    {
        //�������Ϊ���ϰ汾
        ////////////formula of the capacity///////////////
        //
        //  memory capacity = BLOCKNR * BLOCK_LEN
        //
        //	BLOCKNR = (C_SIZE + 1)* MULT
        //
        //           C_SIZE_MULT+2
        //	MULT = 2
        //
        //               READ_BL_LEN
        //	BLOCK_LEN = 2
        /**********************************************/
        //C_SIZE
        i=csd[6]&0x03 ;
        i<<=8 ;
        i+=csd[7];
        i<<=2 ;
        i+=((csd[8]&0xc0)>>6);
        
        //C_SIZE_MULT
        r1=csd[9]&0x03 ;
        r1<<=1 ;
        r1+=((csd[10]&0x80)>>7);
        
        //BLOCKNR
        r1+=2 ;
        temp=1 ;
        while(r1)
        {
            temp*=2 ;
            r1--;
        }
        Capacity=((unsigned long)(i+1))*((unsigned long)temp);
        
        // READ_BL_LEN
        i=csd[5]&0x0f ;
        //BLOCK_LEN
        temp=1 ;
        while(i)
        {
            temp*=2 ;
            i--;
        }
        //The final result
        Capacity*=(unsigned long)temp ;
        //Capacity /= 512;
    }
    return (unsigned long)Capacity ;
}


