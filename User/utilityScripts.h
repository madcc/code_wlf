/*
 * =====================================================================================
 *
 *       Filename:  utilityScripts.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013/9/30 20:51:03
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  madcc09 (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __H_UTILITYSCRIPT_H__
#define __H_UTILITYSCRIPT_H__

#include <stdint.h>
#include "globalDef.h"
enum utilityScriptName
{
	US_HOME_AXES = 0,
	US_LEVEL_PLATE_STARTUP = 1,
	US_TOOLHEAD_CALIBRATE = 2,
	US_LEVEL_PLATE_SECOND
	};
bool utilityIsPlaying(void);
bool utilityPlaybackHasNext(void);
uint8_t utilityPlaybackNext(void);
bool utilityStartPlayback(uint8_t build);
void utilityFinishPlayback(void);
void utiltiyReset(void);
void utilityGetSecondLevelOffset(void);

extern bool utility_isPlaying;
#endif

