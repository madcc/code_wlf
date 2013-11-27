#include "stm32f10x.h"
#include "flash_spi.h"
#include <stdio.h>
/***********************************************
**������:FLASH_SPI_Config
**����:��ʼ������FLASH��SPI�ӿ�
**ע������:����FLASHʹ����SPI1�ӿ�
***********************************************/
void FLASH_SPI_Config(void)
{
    SPI_InitTypeDef SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

   /* SCK, MISO and MOSI B13=SCK,B14=MISO,B15=MOSI*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* PB.12 ��Ƭѡ*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_12);//Ԥ��Ϊ��

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);

    /* SPI2 configuration */
    SPI_Cmd(SPI2, DISABLE);              //�����Ƚ���,���ܸı�MODE
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //����ȫ˫��
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;      //��
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;      //8λ
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;        //CPOL=0 ʱ�����յ�
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;       //CPHA=0 ���ݲ����1��
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;        //���NSS
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;  //2��Ƶ=36M SST25VF˵��50Mû��
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;     //��λ��ǰ
    SPI_InitStructure.SPI_CRCPolynomial = 7;        //CRC7 �Ҳ�������������Ҫ��δ���
    
	SPI_Init(SPI2, &SPI_InitStructure);
	//SPI_SSOutputCmd(SPI2, ENABLE); //ʹ��NSS�ſ��� �����һ��SPI ����
	SPI_Cmd(SPI2, ENABLE); 
}


/***************************************
**������:SPIByte
**����:��дSPI����
**ע������:����SPI��˵�������Ķ�Ҳ��Ҫ��д��
**ʹ�ô˺���������ʱ�����������Ϊ0xff��д��ʱ����д����.����ʹ��ֱ�Ӳ����Ĵ����İ취ʵ��SPIӲ�����д,��Ϊ�˼ӿ���д�ٶ� ��˵LCD ��ʱ�����õľ��ǿ⺯�� ����

SPI_I2S_SendData   SPI_I2S_ReceiveData SPI_I2S_GetFlagStatus
***************************************/
static u8 SPI2Byte(u8 byte)
{
	/*�ȴ����ͼĴ�����*/
	while((SPI2->SR & SPI_I2S_FLAG_TXE)==RESET);
	  /*����һ���ֽ�*/
	SPI2->DR = byte;
	/* �ȴ����ռĴ�����Ч*/
	while((SPI2->SR & SPI_I2S_FLAG_RXNE)==RESET);
	return(SPI2->DR);
}

//����ģʽ0
/*****************************************
**������:SSTCmd1/2/4
**����:дһ��SST����/дһ��������һ������/дһ���������д3������
**ע������:����һ�������ĵ����������������
*****************************************/
void SSTCmd1(u8 cmd)
{
	SST_SELECT();
	SPI2Byte(cmd);
	SST_DESELECT();
}

void SSTCmd2(u8 cmd,u8 data)
{
	SST_SELECT();
	SPI2Byte(cmd);
	SPI2Byte(data);
	SST_DESELECT();
}

void SSTCmd4(u8 cmd,u8 *addr)
{
	SST_SELECT();
	SPI2Byte(cmd); //������
	SPI2Byte(*addr++);
	SPI2Byte(*addr++);
	SPI2Byte(*addr);
	SST_DESELECT();
}


/****************************************
**������:SSTCmdb1b/SSTCmd4bs
**����:дһ��SST�������1�ֽ�����/д1�������֣�3����ַ�֣����ض���ֽ�
**����ʹ���ڶ����ϵ�
****************************************/
u8 SSTCmdb1b(u8 cmd)
{
	u8 tmp;
	SST_SELECT();
	SPI2Byte(cmd);
	tmp=SPI2Byte(0xff);
	SST_DESELECT();
	return(tmp);
}
void SSTCmd4bs(u8 cmd,u8* addr,u8* data,u32 no)
{
	SST_SELECT();
	SPI2Byte(cmd); //������
	SPI2Byte(*addr++);
	SPI2Byte(*addr++);
	SPI2Byte(*addr);
	for(;no>0;no--)
	{
	   *data++=SPI2Byte(0xff);
	}
	SST_DESELECT();
}


//����ʱ���Ӱ�~~��Ȼ������Ϊ����ȫ��д������
//���õ�оƬ����
/***************************************
SST25WREN ����д����
***************************************/
void SST25WREN(void)
{
	SSTCmd1(0x06);
}

/***********************************
SST25WRDI ����д����
***********************************/
void SST25WRDI(void)
{
	SSTCmd1(0x04);
}

/**********************************
SST25BY ���æ
**********************************/
u8 SST25BY(void)
{
	u8 sta;
	sta=SSTCmdb1b(0x05);
	return(sta&0x01);
}

/***********************************
SST25WPEN �������д����
ע������:25��д��ȽϷ�����������ÿ�β���ǰ��ȡ����д������������ɺ�����������д����
***********************************/
void SST25WPEN(void)
{
	u8 sta;
	sta=SSTCmdb1b(0x05)|0x1c;  //�����Ĵ��������뱣��λ
	SSTCmd1(0x50);    //����дStatus Register
	SSTCmd2(0x01,sta);
}

//����������λ��������дλ
void SST25WriteEn(void)
{
	u8 sta;
	sta=SSTCmdb1b(0x05)&(~0x1c); //�����Ĵ�������������λ
	SSTCmd1(0x50);     //����д�Ĵ���Status Register
	SSTCmd2(0x01,sta);    //д�Ĵ���
	SSTCmd1(0x06);     //����д
}

/********************************�Ĵ���Status Register**********************************/

//��������ʵ��д������

/**********************************
SST25ReadID ��ȡSST��ID ������� �Ǻǲ��ö�˵~��Ȼ�����Ķ�д�����϶��ò���
**********************************/
u16 SST25ReadID(void)
{
	u8 id[3];
	u8 addr[3]={0,0,0};
	
	SSTCmd4bs(0x9F,addr,id,3);
	return((id[0]<<8)+id[1]);
}

/**********************************
SST25ChipErase ˢ��CHIP
**********************************/
void SST25ChipErase(void)
{
	SST25WriteEn();
	SSTCmd1(0x60);
	while(SST25BY());
	SST25WPEN();
}

/***********************************
SST25SectorErase ˢ���� �õ���4kb��С �����ַ��0~4095 ֮����ô��֮��ĵ�ַ����ˢ��

��Ȼ�Ҹ� 4096 �Ļ�4096��4096+4095 ֮�䶼��ˢ��
***********************************/
void SST25SectorErase(u32 addr)
{
	u8 ad[3];
	ad[0]=(addr>>16)&0xff;
	ad[1]=(addr>>8)&0xff;
	ad[2]=addr&0xff;
		
	SST25WriteEn();
	
	SST_SELECT();
	SPI2Byte(0x20);
	SPI2Byte(ad[0]);
	SPI2Byte(ad[1]);
	SPI2Byte(ad[2]);
	SST_DESELECT();
	
	while(SST25BY());
	// SST25WPEN();
}

/**********************************
SST25ByteProgram дһ���ֽ�*ע���ڴ�ǰҪ����ȡ��д����,ʵ��дӦʹ��AAI,�˺�����AAI�е��ã�����д�������ֽ�
**********************************/
static void SST25ByteProgram(u32 addr,u8 byte)
{
	u8 ad[3];
	ad[0]=(addr>>16)&0xff;
	ad[1]=(addr>>8)&0xff;
	ad[2]=addr&0xff;
	
	SST_SELECT();
	SPI2Byte(0x02);
	SPI2Byte(ad[0]);
	SPI2Byte(ad[1]);
	SPI2Byte(ad[2]);
	SPI2Byte(byte);
	SST_DESELECT();
	while(SST25BY());
} 

/***********************************
SST25Write д����ֽ�
***********************************/
void SST25Write(u32 addr,u8* p_data,u32 no)
{
	u8 ad[3];
	u32 cnt;
	if(no==0)
	   return;
	  
	SST25WriteEn();
	
	if(no==1) //no<2��Ӧʹ����ͨ���ֽڷ�ʽ
	{
	   SST25ByteProgram(addr,*p_data);
	// SST25WPEN();
	} 
	else
	{
	   cnt=no;
	  
	   ad[2]=(addr>>16)&0xff;
	   ad[1]=(addr>>8)&0xff;
	   ad[0]=addr&0xff;
	  
	   SST_SELECT();
	   SPI2Byte(0xad);
	   SPI2Byte(ad[2]);
	   SPI2Byte(ad[1]);
	   SPI2Byte(ad[0]);
	   SPI2Byte(*p_data++);
	   SPI2Byte(*p_data++);
	   SST_DESELECT();
	   cnt-=2;
	   while(SST25BY()); //��æ
	  
		//�м��˫�ֽ�д
	   for(;cnt>1;cnt-=2)
	   {
	    SST_SELECT();
	    SPI2Byte(0xad);
	    SPI2Byte(*p_data++);
	    SPI2Byte(*p_data++);
	    SST_DESELECT();
	    while(SST25BY()); //��æ
	   }
	   SST25WRDI(); //WRDI�����˳�AAIдģʽ ��νAAI ���ǵ�ַ�Զ���
	  
	  //��������һ���ֽ�(noΪ������
	   if(cnt==1)
	   {
	    SST25WriteEn();
	    SST25ByteProgram(addr+no-1,*p_data);
	   }
	}
	SST25WPEN();//WP����
}

//�����õ����±�����

/*************************************
SST25Read ���ٶ� ���ں�����5��оƬ���ɵ��ô˺�����
*************************************/
void SST25Read(u32 addr,u8* p_data,u32 no)
{
	SST_SELECT();
	
	SPI2Byte(0x0b);
	SPI2Byte(addr>>16);
	SPI2Byte(addr>>8);
	SPI2Byte(addr);
	SPI2Byte(0xff);
	
	for(;no>0;no--)
	*p_data++=SPI2Byte(0xff);
	SST_DESELECT();
}

/****************************************
SST25ReadL ���ٶ�
****************************************/
void SST25ReadL(u32 addr,u8* p_data,u32 no)
{
	u8 ad[3];
	ad[2]=(addr>>16)&0xff;
	ad[1]=(addr>>8)&0xff;
	ad[0]=addr&0xff;
	
	SSTCmd4bs(0x03,ad,p_data,no);
}

//����SST,��ȫ��ˢ��,�����д�����ݼ��--����ʱ��̫����,��Ϊֻд����128�ֽ�����
void TestSst(void)
{
	u8 i;
	u32 addr=0;
	u8 db_sst1[64];
	
	//ˢ��������У���Ƿ�ˢ���ɹ�
	SST25SectorErase(0);
	for(addr=0;addr<64;addr+=64)
	{
		SST25Read(addr,db_sst1,64);//��64���ֽ�
		for(i=0;i<64;i++)
		{
			if(db_sst1[i]!=0xff)
			{
				printf("FLASH������\r\n");
				while(1);
			}
		}
	}
	
	//д��һ��������У���Ƿ�ɹ�
	for(i=0;i<64;i++)
		db_sst1[i]=i;
	//д��һ������,һ��������4K,Ҳ����64*64
	for(addr=0;addr<64;addr+=64)
	{
		SST25Write(addr,db_sst1,64);//д��64���ֽ�
		SST25Read(addr,db_sst1,64);//��64���ֽ�
		for(i=0;i<64;i++)
		{
			if(db_sst1[i]!=i)
			{
				printf("FLASH������\r\n");
				while(1);
			}
		}
	}
	printf("FLASH������\r\n");	
}
