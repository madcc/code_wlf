/*
 * =====================================================================================
 *
 *       Filename:  host.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013/1/21 16:56:05
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  madcc (good lucky), madcc09@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include "host.h"
#include "sdCardOper.h"
#include "utilityScripts.h"
#include "command.h"
#include "stdlib.h"
#include <string.h>

const int MAX_FILE_LEN = MAX_PACKET_PAYLOAD-1;
char hostBuildName[MAX_FILE_LEN];
char hostMachineName[]="Replicator";

HostState hostCurrentState;
// start build from utility script
//caller: WelcomeNotifyButtonPressed,lcdOper.c
void hostStartOnboardBuild(uint8_t build)/*{{{*/
{
	if(utilityStartPlayback(build))
		hostCurrentState=HOST_STATE_BUILDING_ONBOARD;
	commandReset();
	motorAbort();
}/*}}}*/
void hostStopBuild(void)/*{{{*/
{
   // if building from repG, try to send a cancel msg to repG before reseting 
	if(hostCurrentState == HOST_STATE_BUILDING)
	{	
		hostCurrentState = HOST_STATE_CANCEL_BUILD;
		//toFix
		//cancel_timeout.start(1000000); //look for commands from repG for one second before resetting
	}
	//do_host_reset = true; // indicate reset after response has been sent  //toFix
}/*}}}*/

    //set build name and build state
void hostHandleBuildStartNotification(circleBuf* buf) /*{{{*/
{
	uint8_t idx = 0;
	char newName[MAX_FILE_LEN];
	switch (hostCurrentState)
	{
		case HOST_STATE_BUILDING_FROM_SD:
			do {
				newName[idx++] =circleBufPop(buf);// buf.pop();		
			} while (newName[idx-1] != '\0');
			if(strcmp(newName, "RepG Build"))
				strcpy(hostBuildName, newName);
			break;
		case HOST_STATE_READY:
			hostCurrentState= HOST_STATE_BUILDING;
		case HOST_STATE_BUILDING_ONBOARD:
		case HOST_STATE_BUILDING:
			do {
				hostBuildName[idx++] = circleBufPop(buf);//buf.pop();		
			} while (hostBuildName[idx-1] != '\0');
			break;
	}
}/*}}}*/

    // set build state to ready
void hostHandleBuildStopNotification(uint8_t stopFlags) /*{{{*/
{
	//uint8_t flags = stopFlags;
	hostCurrentState= HOST_STATE_READY;
}/*}}}*/

SdErrorCode hostStartBuildFromSD(void)/*{{{*/
{
	SdErrorCode e;
	commandReset();
	e=sdCardStartPlayBack(sdPrintFileName);
	if(SD_SUCCESS !=e)
		return e;
	else
	{
		hostCurrentState=HOST_STATE_BUILDING_FROM_SD;
		return e;
	}
}/*}}}*/
void runHostSlice(void)/*{{{*/
{
	if(HOST_STATE_BUILDING_FROM_SD == hostCurrentState)
	{
		if(!sdCardIsPlaying)
			hostCurrentState=HOST_STATE_READY;
	}
	if(HOST_STATE_BUILDING_ONBOARD == hostCurrentState)
	{
		if(!utility_isPlaying)
			hostCurrentState = HOST_STATE_READY;
	}
}/*}}}*/

