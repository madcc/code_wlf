//SD card operation
#include "sdCardOper.h"
#include <stdio.h>
#include <string.h>
#include "ff.h"
#include "SPI_SD_driver.h"
#include "menu.h"
#include "command.h"

FATFS sdCardFs;
FRESULT sdCardRes;
FIL sdCardFile;
DIR sdCardDir;
char sdPrintFileName[50];
UINT  sdCardByteReadNum; 
unsigned char sdCardDataBuf[512];
u32 sdCardOpenFileSize;
u8 sdCardNextByte;
bool sdCardIsCapturing;
bool sdCardIsPlaying;
bool sdCardHasNextByte;
u32 sdCardCapturedBytes;
FILINFO sdCardOpenFileInfo;

void sdCardReset(void)/*{{{*/
{
	//toFix
	sdCardIsPlaying=false;
	sdCardHasNextByte=false;
}/*}}}*/
SdErrorCode sdCardInit(void)/*{{{*/
{
	int i,fres;
	sdCardReset();
	for(i=0;i<3;i++)
	{
		if((fres=SD_Init())!=0)
			printf("wait init SD card\r\n");
		else
		{
			printf("sd card init ok\r\n");
			break;
		}
	}
	if(fres!=0)
		return SD_ERR_NO_CARD_PRESENT;

	f_mount(0,&sdCardFs);
	f_opendir(&sdCardDir,"/");//root directory
	return SD_SUCCESS;
}/*}}}*/
SdErrorCode sdCardStartPlayBack(char* filename)/*{{{*/
{
	u8 i;
	int fres;
	SdErrorCode result;
	sdCardReset();
	for(i=0;i<3;i++)
	{
		if((fres=SD_Init())!=0) //toFix,SdErrorCode 是两套
			printf("等待SD卡初始化\r\n");
		else
			break;
	}
	if(fres !=SD_SUCCESS)
	{
		printf("SD卡读取失败\r\n");
		return result;//toFix
	}
	sdCardCapturedBytes=0L;
	f_mount(0,&sdCardFs);
	if(f_open(&sdCardFile,filename,FA_OPEN_EXISTING|FA_READ)!=FR_OK)	
		return SD_ERR_FILE_NOT_FOUND;
	f_stat(filename,&sdCardOpenFileInfo);
	sdCardOpenFileSize=sdCardOpenFileInfo.fsize;
	sdCardIsPlaying=true;
	sdCardFetchNextByte();
	cmdPaused=false;
	return SD_SUCCESS;
}/*}}}*/
void sdCardFinishPlayback(void)/*{{{*/
{
	sdCardIsPlaying=false;
	f_close(&sdCardFile);//toFix
	f_mount(0,0);
}/*}}}*/
u8 sdCardPlaybackNext(void)/*{{{*/
{
	u8 rv=sdCardNextByte;
	sdCardFetchNextByte();
	return rv;
}  /*}}}*/
void sdCardFetchNextByte(void)/*{{{*/
{
	sdCardRes=f_read(&sdCardFile,sdCardDataBuf,1,&sdCardByteReadNum);
	sdCardNextByte=sdCardDataBuf[0];
	sdCardHasNextByte= sdCardByteReadNum > 0;

//	if()  toFix
//	{
//		
//	}
//	sdCardRes = f_read(&sdCardRile, sdCardDataBuf, 1, &sdCardByteReadNum);	
//	sdCardHasNextByte= sdCardByteRead > 0;
}/*}}}*/
bool sdCardOpenRoot(void)/*{{{*/
{
	f_mount(0,&sdCardFs);//挂载文件系统  //toFix
	f_opendir(&sdCardDir,"/");//root directory
	return true;
}/*}}}*/

bool sdMenuInitFiles(char * path) //初始化获得SD卡中文件名	/* {{{ */
//bool sdMenuInitFiles(char * path,char fileName[20][30],int * pFileCnt) // BUG
{
	FRESULT fres;
	FILINFO fno;
	char * fileName;
	uint8_t fileNameLen;
	DIR dir;
	fres=f_opendir(&dir,path);
	if(FR_OK==fres)
	{
		while(1)
		{
			fres=f_readdir(&dir,&fno);
			if((FR_OK!=fres)||(fno.fname[0]==0))
				break;
			if(fno.fname[0]=='.')
				continue;
			if(!(fno.fattrib&AM_DIR))
			{
				//fileName[fileCnt]=fno.fname;  _BUG
				//strcpy(fileName[fileCnt],fno.fname);
				fileName=fno.fname;
				fileNameLen=0;
				while((*fileName)!='\0')
				{
					fileNameLen++;
					fileName++;
				}
				fileName=fno.fname+fileNameLen-3;
				if(((*(fileName) =='s') ||(*(fileName) =='S')) &&
					   	((*(fileName+1) == '3') ) && 
						((*(fileName+2)=='g') ||(*(fileName+2) =='G')) )
				{
					strcpy(sdFileName[sdFileCnt],fno.fname);
					sdFileCnt++;
				}
			}
		}
		return true;
	}
	return false;
}/*}}}*/


