#ifndef COMMAND_H
#define COMMAND_H

#include "globalDef.h"
#include "circleBuf.h"
#include "timeout.h"
#include "sdCardOper.h"
#include "commandDefine.h"
#include "motor.h"

typedef enum {/*{{{*/
	READY,
	MOVING,
	DELAY,
	HOMING,
	WAIT_ON_TOOL,
	WAIT_ON_PLATFORM,
	WAIT_ON_BUTTON,
}CommandState;/*}}}*/

extern CommandState cmdMode;
extern circleBuf cmdBuf;
extern u16 cmdSdFailCnt;
extern u32 cmdSdByteCnt;
extern bool cmdSdReset;
extern bool cmdPaused;
extern bool cmdHeatShutdown;

void runCmdSlice(void);
void commandReset( void);
bool commandIsReady(void);
bool commandIsEmpty(void);
void commandPush(void);

#endif

