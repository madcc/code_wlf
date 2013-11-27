//
#ifndef SDCARD_OPER_H
#define SDCARD_OPER_H

#include "globalDef.h"
#include "ff.h"

extern FATFS sdCardFs;
extern FRESULT sdCardRes;
extern FIL sdCardFile;
extern DIR sdCardDir;

extern char sdPrintFileName[50];
//extern UINT sdCardByteReadNum; 
extern unsigned char sdCardDataBuf[512];
extern u32 sdCardOpenFileSize;

extern bool sdCardIsCapturing;
extern bool sdCardIsPlaying;
extern bool sdCardHasNextByte;
extern u32 sdCardCapturedBytes;
extern FILINFO sdCardOpenFileInfo;
/// This enumeration lists all the SD card call error/success codes.
/// Any non-zero value is an error condition.
typedef enum {/*{{{*/
  SD_SUCCESS              = 0,  ///< Operation succeeded
  SD_ERR_NO_CARD_PRESENT  = 1,  ///< No SD card is inserted in the slot
  SD_ERR_INIT_FAILED      = 2,  ///< SD card initialization failed
  SD_ERR_PARTITION_READ   = 3,  ///< Couldn't read the card's partition table
  SD_ERR_OPEN_FILESYSTEM  = 4,  ///< Couldn't open the FAT16 filesystem --
                                ///<  check that it's real FAT16
  SD_ERR_NO_ROOT          = 5,  ///< No root directory found
  SD_ERR_CARD_LOCKED      = 6,  ///< Card is locked, writing forbidden
  SD_ERR_FILE_NOT_FOUND   = 7,  ///< Could not find specific file
  SD_ERR_GENERIC          = 8,  ///< General error
  SD_ERR_VOLUME_TOO_BIG	  = 10  ///< the SD card filesystem is too large
} SdErrorCode;/*}}}*/

SdErrorCode sdCardInit(void);
void sdCardReset(void);
SdErrorCode sdCardStartPlayBack(char* filename);
void sdCardFinishPlayback(void);
u8 sdCardPlaybackNext(void);
void sdCardFetchNextByte(void);
bool sdCardOpenRoot(void);
bool sdMenuInitFiles(char * path);

#endif

