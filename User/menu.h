/*
 * =====================================================================================
 *
 *       Filename:  menu.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013/9/30 16:17:58
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  madcc09 (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __H_MENU_H__
#define __H_MENU_H__

#include <stdint.h>
#include "globalDef.h"
#define LCD_MaxItem 4
typedef struct str_MenuItem
{
	//uint8_t itemIdx;
	uint8_t itemCnt;
	char * dispString;
	void (*itemFunc)();
	struct str_MenuItem *pChildMenu;
	struct str_MenuItem *pParentMenu;
//	struct str_MenuItem *const pChildMenu;
//	struct str_MenuItem *const pParentMenu;
}MenuItem;

extern MenuItem finishPrintMenu[];
void menuInit(void);
//void showEnterMsg(void);
void showCurItem(void);
void showChildItem(void); //显示下一级菜单
void showParentItem(void);// 返回显示上一级菜单 
void showItems(void);

void commonMenuFunc(void);
void welcomeMenuFunc(void);
void utilityMenuFunc(void);
void sdMenuFunc(void);
void sdMenuDrawFile(void);
void filamentMenuFunc(void);
void filamentMenuUpdate(void);
void filamentStartMotor(void);
void filamentStopMotor(void);
void heatingMenuUpdate(void);
void addMsg(char str[],uint8_t options,uint8_t xpos,uint8_t ypos);
void menuUpdate(void);
void showErrMsg(char* str);

extern MenuItem * pCurItem;
extern MenuItem * pDispStartItem;
extern MenuItem * pCurMenuStartItem;
extern int sdFileCnt;
//extern char** sdFileName;  //编译不过
//extern char * sdFileName[];	//编译不过
//extern char * sdFileName[30];  //编译不过
extern char sdFileName[20][30];

/// states for Welcome Menu
enum FilamentStates{/*{{{*/
    FILAMENT_HEATING,
    FILAMENT_EXPLAIN2,
    FILAMENT_EXPLAIN3,
    FILAMENT_EXPLAIN4,
    FILAMENT_HEAT_BAR,
    FILAMENT_WAIT,
    FILAMENT_START,
    FILAMENT_TUG,
    FILAMENT_STOP,
    FILAMENT_OK,
    FILAMENT_DONE,
    FILAMENT_EXIT
};/*}}}*/

enum FilamentScript{/*{{{*/
	FILAMENT_RIGHT_FOR,
	FILAMENT_LEFT_FOR,
    FILAMENT_RIGHT_REV,
    FILAMENT_LEFT_REV,	
	FILAMENT_STARTUP_SINGLE,
	FILAMENT_STARTUP_DUAL,
	};/*}}}*/



#endif
