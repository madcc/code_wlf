#include "stm32f10x.h"
#include "flash_spi.h"
#include <stdio.h>
/***********************************************
**函数名:FLASH_SPI_Config
**功能:初始化串行FLASH的SPI接口
**注意事项:串行FLASH使用了SPI1接口
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

    /* PB.12 作片选*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_12);//预置为高

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);

    /* SPI2 configuration */
    SPI_Cmd(SPI2, DISABLE);              //必须先禁能,才能改变MODE
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //两线全双工
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;      //主
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;      //8位
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;        //CPOL=0 时钟悬空低
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;       //CPHA=0 数据捕获第1个
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;        //软件NSS
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;  //2分频=36M SST25VF说是50M没事
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;     //高位在前
    SPI_InitStructure.SPI_CRCPolynomial = 7;        //CRC7 我不解的是如果出错要如何处理
    
	SPI_Init(SPI2, &SPI_InitStructure);
	//SPI_SSOutputCmd(SPI2, ENABLE); //使能NSS脚可用 我这就一个SPI 器件
	SPI_Cmd(SPI2, ENABLE); 
}


/***************************************
**函数名:SPIByte
**功能:读写SPI总线
**注意事项:对于SPI来说，主机的读也需要先写，
**使用此函数，读的时候建议参数设置为0xff，写的时候则写参数.这里使用直接操作寄存器的办法实现SPI硬件层读写,是为了加快速写速度 在说LCD 的时候我用的就是库函数 比如

SPI_I2S_SendData   SPI_I2S_ReceiveData SPI_I2S_GetFlagStatus
***************************************/
static u8 SPI2Byte(u8 byte)
{
	/*等待发送寄存器空*/
	while((SPI2->SR & SPI_I2S_FLAG_TXE)==RESET);
	  /*发送一个字节*/
	SPI2->DR = byte;
	/* 等待接收寄存器有效*/
	while((SPI2->SR & SPI_I2S_FLAG_RXNE)==RESET);
	return(SPI2->DR);
}

//咱用模式0
/*****************************************
**函数名:SSTCmd1/2/4
**功能:写一个SST命令/写一个命令后接一个数据/写一个命令后再写3个数据
**注意事项:这是一个完整的单命令操作，不返回
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
	SPI2Byte(cmd); //首命令
	SPI2Byte(*addr++);
	SPI2Byte(*addr++);
	SPI2Byte(*addr);
	SST_DESELECT();
}


/****************************************
**函数名:SSTCmdb1b/SSTCmd4bs
**功能:写一个SST命令，返回1字节数据/写1个命令字，3个地址字，返回多个字节
**更多使用在读出上的
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
	SPI2Byte(cmd); //首命令
	SPI2Byte(*addr++);
	SPI2Byte(*addr++);
	SPI2Byte(*addr);
	for(;no>0;no--)
	{
	   *data++=SPI2Byte(0xff);
	}
	SST_DESELECT();
}


//命令时序复杂啊~~当然了我这为了求全都写出来了
//常用的芯片功能
/***************************************
SST25WREN 允许写功能
***************************************/
void SST25WREN(void)
{
	SSTCmd1(0x06);
}

/***********************************
SST25WRDI 屏蔽写功能
***********************************/
void SST25WRDI(void)
{
	SSTCmd1(0x04);
}

/**********************************
SST25BY 检测忙
**********************************/
u8 SST25BY(void)
{
	u8 sta;
	sta=SSTCmdb1b(0x05);
	return(sta&0x01);
}

/***********************************
SST25WPEN 允许软件写保护
注意事项:25的写入比较繁琐，建议在每次操作前都取消掉写保护，操作完成后则重新允许写保护
***********************************/
void SST25WPEN(void)
{
	u8 sta;
	sta=SSTCmdb1b(0x05)|0x1c;  //读出寄存器并加入保护位
	SSTCmd1(0x50);    //允许写Status Register
	SSTCmd2(0x01,sta);
}

//先消除保护位，再允许写位
void SST25WriteEn(void)
{
	u8 sta;
	sta=SSTCmdb1b(0x05)&(~0x1c); //读出寄存器并消除保护位
	SSTCmd1(0x50);     //允许写寄存器Status Register
	SSTCmd2(0x01,sta);    //写寄存器
	SSTCmd1(0x06);     //允许写
}

/********************************寄存器Status Register**********************************/

//就是这样实现写保护。

/**********************************
SST25ReadID 读取SST的ID 这个功能 呵呵不用多说~当然单纯的读写操作肯定用不上
**********************************/
u16 SST25ReadID(void)
{
	u8 id[3];
	u8 addr[3]={0,0,0};
	
	SSTCmd4bs(0x9F,addr,id,3);
	return((id[0]<<8)+id[1]);
}

/**********************************
SST25ChipErase 刷除CHIP
**********************************/
void SST25ChipErase(void)
{
	SST25WriteEn();
	SSTCmd1(0x60);
	while(SST25BY());
	SST25WPEN();
}

/***********************************
SST25SectorErase 刷扇区 用的是4kb大小 假如地址在0~4095 之间那么这之间的地址都会刷除

当然我给 4096 的话4096到4096+4095 之间都会刷掉
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
SST25ByteProgram 写一个字节*注意在此前要调用取消写保护,实际写应使用AAI,此函数在AAI中调用，用于写奇数个字节
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
SST25Write 写多个字节
***********************************/
void SST25Write(u32 addr,u8* p_data,u32 no)
{
	u8 ad[3];
	u32 cnt;
	if(no==0)
	   return;
	  
	SST25WriteEn();
	
	if(no==1) //no<2则应使用普通单字节方式
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
	   while(SST25BY()); //判忙
	  
		//中间的双字节写
	   for(;cnt>1;cnt-=2)
	   {
	    SST_SELECT();
	    SPI2Byte(0xad);
	    SPI2Byte(*p_data++);
	    SPI2Byte(*p_data++);
	    SST_DESELECT();
	    while(SST25BY()); //判忙
	   }
	   SST25WRDI(); //WRDI用于退出AAI写模式 所谓AAI 就是地址自动加
	  
	  //如果有最后一个字节(no为奇数）
	   if(cnt==1)
	   {
	    SST25WriteEn();
	    SST25ByteProgram(addr+no-1,*p_data);
	   }
	}
	SST25WPEN();//WP保护
}

//我们用的是下边这种

/*************************************
SST25Read 高速读 对于后续带5的芯片，可调用此函数读
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
SST25ReadL 低速读
****************************************/
void SST25ReadL(u32 addr,u8* p_data,u32 no)
{
	u8 ad[3];
	ad[2]=(addr>>16)&0xff;
	ad[1]=(addr>>8)&0xff;
	ad[0]=addr&0xff;
	
	SSTCmd4bs(0x03,ad,p_data,no);
}

//测试SST,先全部刷除,再逐个写入数据检查--这样时间太长了,改为只写入检测128字节数据
void TestSst(void)
{
	u8 i;
	u32 addr=0;
	u8 db_sst1[64];
	
	//刷除扇区并校验是否刷除成功
	SST25SectorErase(0);
	for(addr=0;addr<64;addr+=64)
	{
		SST25Read(addr,db_sst1,64);//读64个字节
		for(i=0;i<64;i++)
		{
			if(db_sst1[i]!=0xff)
			{
				printf("FLASH检查出错\r\n");
				while(1);
			}
		}
	}
	
	//写入一个扇区并校验是否成功
	for(i=0;i<64;i++)
		db_sst1[i]=i;
	//写入一个扇区,一个扇区是4K,也就是64*64
	for(addr=0;addr<64;addr+=64)
	{
		SST25Write(addr,db_sst1,64);//写入64个字节
		SST25Read(addr,db_sst1,64);//读64个字节
		for(i=0;i<64;i++)
		{
			if(db_sst1[i]!=i)
			{
				printf("FLASH检查出错\r\n");
				while(1);
			}
		}
	}
	printf("FLASH检查完成\r\n");	
}
