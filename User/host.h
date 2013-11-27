/*
 * =====================================================================================
 *
 *       Filename:  host.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013/1/21 16:55:56
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  madcc (good lucky), madcc09@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef HOST_H
#define HOST_H

#include "globalDef.h"
#include "circleBuf.h"
#include "sdCardOper.h"

/// The host can be in any of these four states.
typedef enum HostStateEnum {/*{{{*/
        HOST_STATE_READY            = 0,
        HOST_STATE_BUILDING         = 1,
        HOST_STATE_BUILDING_FROM_SD = 2,
        HOST_STATE_ERROR            = 3,
        HOST_STATE_CANCEL_BUILD		= 4,
        HOST_STATE_BUILDING_ONBOARD = 5,
        HOST_STATE_HEAT_SHUTDOWN = 6,
}HostState;/*}}}*/

extern HostState hostCurrentState;
extern char hostBuildName[];
extern char hostMachineName[];
extern const int MAX_FILE_LEN;

void hostStartOnboardBuild(uint8_t build);
void hostStopBuild(void);
void hostHandleBuildStartNotification(circleBuf* buf);
SdErrorCode hostStartBuildFromSD(void);
void runHostSlice(void);

#endif

