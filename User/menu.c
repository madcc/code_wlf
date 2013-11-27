/*
 * =====================================================================================
 *
 *       Filename:  menu.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013/9/30 16:20:48
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  madcc09 (), 
 *   Organization:  
 *
 * =====================================================================================
 */
///=====//tofix
#include "globalDef.h"
#include "motor.h"
#include "menu.h"
#include "button.h"
#include "utilityScripts.h"
#include "host.h"
#include "sdCardOper.h"
#include "lcdSerial.h"
#include "timeout.h"
#include "tempature.h"
#include "command.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ÿ���һ���˵���������Ҫ�޸ģ�
// FirstMenuItemNum
// MenuItem[]�е� pChildMenu, pParentMenu; ע���޸� �����˵��е�pChildMenu;
// ���� SaveMaxLevel 
#define MainMenuItemNum 4
#define WelcomeMenuItemNum 10
#define PreheatMenuItemNum 6
#define SplashMenuItemNum 4
#define SdMenuItemNum 1
#define UtilityMenuItemNum 15

MenuItem sdMenu[];
MenuItem utilityMenu[];
MenuItem mainMenu[];

#define MIN(x,y)\
	( (x)<(y) ?  x:y )

#define OutputScreen(x)\
	{lcdSerialClear();\
	lcdSerialSetCursor(0,0);\
	lcdSerialWriteString(x);}

bool menu_readyFail=false;
enum Menu_SuccessState{menu_Success,menu_Fail,menu_SecondFail};
uint8_t menu_levelSuccess;
uint8_t menu_filamentSuccess;

MenuItem * pCurItem; // ��ǰѡ�еĲ˵���
MenuItem * pDispStartItem; //LCD����ʾ��һ�ж�Ӧ�Ĳ˵���
MenuItem * pCurMenuStartItem;// ��ǰ�˵� ʵ�ʵĵ�һ��
// SaveMaxLevel : ��Ҫջ���� �����˵�ʵ�ʺ���ʾ�ĵ�һ�������Ӳ˵�����ʱ
// ��֪�����ʾ ��SaveMaxLevel �˵������
#define SaveMaxLevel 10
MenuItem * pArrCurMenuStart[SaveMaxLevel];
MenuItem * pArrDispStart[SaveMaxLevel];
uint8_t arrCntSaveMenu=0;
bool needsRedraw=false;
uint8_t cursor=0;
static char message[100];
enum MenuState{NormalMenu=0,SdPrintHeat,SdPrintCancel,SdPrintFinish,
	UtilityHomeAxes,UtilityLevelPlate,UtilityCancel,CancelMenuState};
enum MenuState curMenuState=NormalMenu;
bool isCancel=false;

#define SdFile_MaxCnt 20
char sdFileName[SdFile_MaxCnt][30]; //����ļ�����
int sdFileCnt;
bool isSdMenuFileInit=false;
int sdFileDispStartIdx=0;
int sdFileCurIdx=0;
//��Щ������Ӧ����ͷ�ļ�����������Ϊ���ᱻ�ⲿ��������
void msgMenuUpdate(void);
void msgMenuFunc(void);
void stopSdPrint(void);
void stopUtilityScript(void);
bool cancelFaceDispFunc(void);
void showCancelFace(void);
void mainMenuFunc(void);
MenuItem splashMenu[]= //{{{
{
	{SplashMenuItemNum,"                    ",NULL,NULL,NULL},
	{SplashMenuItemNum,"     The Printer    ",NULL,NULL,NULL},
	{SplashMenuItemNum,"     -----------    ",NULL,NULL,NULL},
	{SplashMenuItemNum,"  Firmware version  ",NULL,NULL,NULL}
};
//}}}
MenuItem preheatMenu[]=		//{{{
{
	{PreheatMenuItemNum,"Start Preheat!",NULL,NULL,NULL},
	{PreheatMenuItemNum,"Stop Preheat!",NULL,NULL,NULL},
	{PreheatMenuItemNum,"Right Tool",NULL,NULL,NULL},
	{PreheatMenuItemNum,"Left Tool",NULL,NULL,NULL},
	{PreheatMenuItemNum,"Platform",NULL,NULL,NULL},
	{PreheatMenuItemNum,"Extruder",NULL,NULL,NULL},
};
//}}}
MenuItem welcomeMenu[]=  //{{{
{
	{WelcomeMenuItemNum,"Welcome!            I'm the Replicator. Press the red M to  get started!        ",welcomeMenuFunc,NULL,NULL},
	{WelcomeMenuItemNum, "A blinking 'M' meansI'm waiting and willcontinue when you   press the button... ",welcomeMenuFunc,NULL,NULL},
	{WelcomeMenuItemNum, "A solid 'M' means   I'm working and willupdate my status    when I'm finished...",welcomeMenuFunc,NULL,NULL},
	{WelcomeMenuItemNum,  "Our next steps will get me set up to    print! First, we'll restore my build... ",welcomeMenuFunc,NULL,NULL},
	{WelcomeMenuItemNum,    "platform so it's    nice and level. It'sprobably a bit off  from shipping...    ",welcomeMenuFunc,NULL,NULL},
	{WelcomeMenuItemNum,   "Aaah, that feels    much better.        Let's go on and loadsome plastic!       ",welcomeMenuFunc,NULL,NULL},
	{WelcomeMenuItemNum, "We'll try again!                                                                ",welcomeMenuFunc,NULL,NULL},
	{WelcomeMenuItemNum,  "We'll keep going    and load some       plastic! For help goto makerbot.com/help",welcomeMenuFunc,NULL,NULL},
	{WelcomeMenuItemNum,  "Awesome!            We'll go to the SD  card Menu and you   can select a print! ",welcomeMenuFunc,NULL,NULL},
	{WelcomeMenuItemNum,     "We'll go to the mainmenu. If you need   help go to:         makerbot.com/help   ",welcomeMenuFunc,NULL,NULL},
};
//}}}
MenuItem filamentMenu[]=/*{{{*/
{
	{2,"load",filamentMenuFunc,NULL,utilityMenu},
	{2,"unload",filamentMenuFunc,NULL,utilityMenu},
};/*}}}*/
MenuItem utilityMenu[]=/*{{{*/
{
	{UtilityMenuItemNum,"Monitor Mode",utilityMenuFunc,NULL,mainMenu}, //0
	{UtilityMenuItemNum,"JogMode",utilityMenuFunc,NULL,mainMenu}, //1
	{UtilityMenuItemNum,"Calibrate Axes",utilityMenuFunc,NULL,mainMenu},	//2
	{UtilityMenuItemNum,"Home Axes",utilityMenuFunc,NULL,mainMenu},	//3
	{UtilityMenuItemNum,"filament options",utilityMenuFunc,filamentMenu,mainMenu},//4
	{UtilityMenuItemNum,"Run Startup Script",utilityMenuFunc,welcomeMenu,mainMenu},	//5
	{UtilityMenuItemNum,"Disable steppers",utilityMenuFunc,NULL,mainMenu},	//6
	{UtilityMenuItemNum,"enable steppers",utilityMenuFunc,NULL,mainMenu},	//7
	{UtilityMenuItemNum,"Level build palate",utilityMenuFunc,NULL,mainMenu},	//8
	{UtilityMenuItemNum,"blink leds",utilityMenuFunc,NULL,mainMenu},	//9
	{UtilityMenuItemNum,"stop blinking",utilityMenuFunc,NULL,mainMenu},	//10
	{UtilityMenuItemNum,"Preheat setting",utilityMenuFunc,preheatMenu,mainMenu},	//11
	{UtilityMenuItemNum,"general setting",utilityMenuFunc,NULL,mainMenu},	//12
	{UtilityMenuItemNum,"Restore Default",utilityMenuFunc,NULL,mainMenu},	//13
	{UtilityMenuItemNum,"Calibrate Nozzles",utilityMenuFunc,NULL,mainMenu},	//14
};/*}}}*/
//MenuItem heatingMenu[]=/*{{{*/
//{
//	{1,"heating",heatingMenuUpdate,NULL,mainMenu},
//};/*}}}*/
MenuItem mainMenu[]=  //{{{
{
	{MainMenuItemNum,"3D printer",mainMenuFunc,NULL,NULL},
	{MainMenuItemNum,"Print from SD",mainMenuFunc,sdMenu,NULL},
	{MainMenuItemNum,"Preheat",mainMenuFunc,NULL,NULL},
	{MainMenuItemNum,"Utilities",mainMenuFunc,utilityMenu,NULL},
};
//}}}
MenuItem sdMenu[]=/*{{{*/
{
	{SdMenuItemNum,"sd file",sdMenuFunc,NULL,mainMenu},
};/*}}}*/
//MenuItem msgMenu[]=/*{{{*/
//{
//	{1,"message",msgMenuFunc,NULL,NULL},
//};/*}}}*/
MenuItem finishPrintMenu[]=/*{{{*/
{
	{1,"print finished!",commonMenuFunc,NULL,mainMenu},
};/*}}}*/
void menuInit()  //{{{
{
	memset(pArrCurMenuStart,0,SaveMaxLevel);
	memset(pArrDispStart,0,SaveMaxLevel);
	arrCntSaveMenu=0;
	pCurItem=&mainMenu[0];
	pCurMenuStartItem=&mainMenu[0];
	pDispStartItem=&mainMenu[0];
}
//}}}
void menuUpdate(void) //��ʱ���µ�ǰĿ¼	/*{{{*/
{
	if(NormalMenu == curMenuState)
	{
		if(filamentMenu == pCurMenuStartItem)
			filamentMenuUpdate();
//	else if(heatingMenu == pCurItem)
//		heatingMenuUpdate();
	}
	else if(SdPrintHeat == curMenuState)
		heatingMenuUpdate();
	else if(UtilityLevelPlate == curMenuState)
		msgMenuUpdate();
}/*}}}*/
void jumptoMenu(MenuItem * newMenu)//�����µ�Ŀ¼�������menu stack/*{{{*/
{
	MenuItem * tempMenu;
	int tempCnt;
	pCurItem=newMenu;
	pCurMenuStartItem=newMenu; //��������fixme
	arrCntSaveMenu=0;
	tempMenu=pCurItem->pParentMenu;
	if(!tempMenu)
		return;
	while(tempMenu)
	{
		arrCntSaveMenu++;
		tempMenu=tempMenu->pParentMenu;
	}
	tempCnt=arrCntSaveMenu;
	tempMenu=pCurItem;
	while(tempMenu)
	{
		pArrCurMenuStart[tempCnt]=tempMenu;
		tempCnt--;
		tempMenu=tempMenu->pParentMenu;
	}
}/*}}}*/
void pushMenuStack(MenuItem * pMenu)/*{{{*/
{
	if(NULL==pMenu)
		return;
	if(arrCntSaveMenu >= SaveMaxLevel)
		return;
	pArrCurMenuStart[arrCntSaveMenu]=pCurMenuStartItem;
	pArrDispStart[arrCntSaveMenu]=pDispStartItem;
	arrCntSaveMenu++;

	pCurItem=pMenu;
	pCurMenuStartItem=pCurItem;
	pDispStartItem=pCurItem;
}/*}}}*/
void popMenuStack(void)/*{{{*/
{
	if(arrCntSaveMenu <1 )
		return;
	arrCntSaveMenu--;
	pCurItem=pArrCurMenuStart[arrCntSaveMenu];//FIXME ��������
	pCurMenuStartItem=pCurItem;
	pDispStartItem=pArrDispStart[arrCntSaveMenu];

}/*}}}*/
void showCurItem()  //��ʾ��ǰ�˵� //{{{
{
	showItems();
}
//}}}
void showChildItem() //��ʾ��һ���˵�  //{{{
{
	if(NULL==pCurItem->pChildMenu) //
		return;
	if(arrCntSaveMenu >= SaveMaxLevel)
		return;
	pArrCurMenuStart[arrCntSaveMenu]=pCurMenuStartItem;
	pArrDispStart[arrCntSaveMenu]=pDispStartItem;
	arrCntSaveMenu++;

	pCurItem=pCurItem->pChildMenu;
	pCurMenuStartItem=pCurItem;
	pDispStartItem=pCurItem;
	showItems();
}
//}}}
void showParentItem() //��ʾ�����˵� //{{{
{
	if(NULL==pCurItem->pParentMenu)
		return;
	if(arrCntSaveMenu <=0)
		return;
	arrCntSaveMenu--;

	pCurItem=pCurItem->pParentMenu;
	pCurMenuStartItem=pArrCurMenuStart[arrCntSaveMenu];
	pDispStartItem=pArrDispStart[arrCntSaveMenu];
	showItems();
}
//}}}
void showItems() //{{{
{
	int i;
	MenuItem * pItem;
	lcdSerialClear();
		pItem=pDispStartItem;
	for(i=0;i<MIN(pCurItem->itemCnt,LCD_MaxItem);i++)
	{
		lcdSerialSetCursor(2,i);
		lcdSerialWriteString(pItem->dispString);
		if(pCurItem==pItem)
		{
			lcdSerialSetCursor(0,i);
			lcdSerialWriteString("->");
		}
		pItem++;
	}
}
//}}}
void showErrMsg(char* str)/*{{{*/
{
	lcdSerialClear();
	lcdSerialSetCursor(0,0);
	lcdSerialWriteString("error !");
	lcdSerialSetCursor(2,1);
	lcdSerialWriteString(str);
	delay_ms(1000);
	showCurItem();
}/*}}}*/
void commonMenuFunc() //ͨ�ò˵����������� //{{{
{
	switch(btnCode)
	{
		case BC_Down:
			if(pCurItem - pCurMenuStartItem <(pCurItem->itemCnt-1)) // pCurItem��û��ָ�����һ��
			{
				if(pCurItem - pDispStartItem ==(LCD_MaxItem-1)) //��ʾ���LCD���������������� 
				{
					pDispStartItem++;
				}
				pCurItem++;
			}
			showCurItem();
			break;
		case BC_Left:
			if(NULL!=pCurItem->pParentMenu)
				showParentItem();
			break;
		case BC_Center:
//			if(NULL!=(pCurItem->itemFunc))  // _note: ������������Ϊ itemFunc ��������������������᲻��ѭ����������
//				(*(pCurItem->itemFunc))();
//			break;
		case BC_Right:
			if(NULL!=pCurItem->pChildMenu)
				showChildItem();
			break;
		case BC_Up:
			if(pCurItem > pCurMenuStartItem)
			{
				if(pCurItem == pDispStartItem)
				{
					pDispStartItem--;
				}
				pCurItem--;
			}
			showCurItem();
			break;
		default:break;
	}
	btnCode=BC_NULL;
}
//}}}
void commonMenuDispFunc() //ֻ�������¼�����ʾ�ģ����Ҳ��޸�btnCode/*{{{*/
{
	switch(btnCode)
	{
		case BC_Down:
			if(pCurItem - pCurMenuStartItem <(pCurItem->itemCnt-1)) // pCurItem��û��ָ�����һ��
			{
				if(pCurItem - pDispStartItem ==(LCD_MaxItem-1)) //��ʾ���LCD���������������� 
				{
					pDispStartItem++;
				}
				pCurItem++;
			}
			showCurItem();
			break;
		case BC_Up:
			if(pCurItem > pCurMenuStartItem)
			{
				if(pCurItem == pDispStartItem)
				{
					pDispStartItem--;
				}
				pCurItem--;
			}
			showCurItem();
			break;
		default:break;
	}
}/*}}}*/
void welcomeMenuFunc()  //{{{
{
	enum WelcomeState{Welcome_Start=0,Welcome_Btn1,Welcome_Btn2,Welcome_Explain,Welcome_Level,
				Welcome_Level_Action,Welcome_Level_OK,Welcome_Load_Plastic,
				Welcome_Load_Action,Welcome_Ready,Welcome_Load_SD,
				Welcome_Print_SD,Welcome_Done};
	static enum WelcomeState welcomeState;
	if(CancelMenuState == curMenuState)/*{{{*/
	{
		if(btnCode== BC_Up || btnCode== BC_Down)
			cancelFaceDispFunc();
		if(btnCode ==BC_Center)
		{
			if(isCancel)
			{
				curMenuState=NormalMenu;
				showParentItem();
			}
			else
			{
				curMenuState=NormalMenu;
				showCurItem();
			}
		}
		btnCode=BC_NULL;
	}/*}}}*/
	else if(NormalMenu== curMenuState)/*{{{*/
	{
		switch(btnCode)
		{
			case BC_Center:
				welcomeState++;
				break;
			case BC_Left:
				break;
			default:
				break;
		}
		btnCode=BC_NULL;
		lcdSerialSetCursor(0,0);
		switch(welcomeState)
		{
			case Welcome_Start:
				lcdSerialWriteString(welcomeMenu[0].dispString);
				break;
			case Welcome_Btn1:
				lcdSerialWriteString(welcomeMenu[1].dispString);
				break;
			case Welcome_Btn2:
				lcdSerialWriteString(welcomeMenu[2].dispString);
				break;
			case Welcome_Explain:
				lcdSerialWriteString(welcomeMenu[3].dispString);
				break;
			case Welcome_Level:
				lcdSerialWriteString(welcomeMenu[4].dispString);
				break;
			case Welcome_Level_Action:
				welcomeState++;
				//tofix
				//			if(menu_levelSuccess == menu_Fail)
				//				hostStartOnboardBuild(US_LEVEL_PLATE_SECOND);
				//			else
				//				hostStartOnboardBuild(US_LEVEL_PLATE_STARTUP);
				break;
			case Welcome_Level_OK:
				lcdSerialWriteString(welcomeMenu[5].dispString);
				break;
			case Welcome_Load_Plastic:
				lcdSerialWriteString(welcomeMenu[6].dispString);
				break;
			case Welcome_Load_Action:
				welcomeState++;

			case Welcome_Ready:
				lcdSerialWriteString(welcomeMenu[7].dispString);
				break;
			case Welcome_Load_SD:
				lcdSerialWriteString(welcomeMenu[8].dispString);
				break;
			case Welcome_Done:
				lcdSerialWriteString(welcomeMenu[9].dispString);
				break;
			default:
				break;
		}
	}/*}}}*/
}
//}}}
void mainMenuFunc(void)/*{{{*/
{
	commonMenuDispFunc();
	if(BC_Center ==btnCode)
	{
		if(pCurItem == &mainMenu[1])
		{
			if(!isSdMenuFileInit)
			{
				if(!sdMenuInitFiles("/"))
				{
					lcdSerialWriteString("sd file error");
					btnCode=BC_NULL;
					return;
				}
				isSdMenuFileInit=true;
				sdFileDispStartIdx=0;
				sdFileCurIdx=0;
				btnCode=BC_NULL; // ��һ�ν����ֻ��ʾ��ǰsd�������ļ������������switch(btnCode)
			}	
			pushMenuStack(sdMenu);
			sdMenuDrawFile();
		}
		else if(pCurItem == &mainMenu[3])
		{
			showChildItem();
		}
	}
	btnCode=BC_NULL;
}/*}}}*/
bool isFilamentForward=true;
static enum FilamentStates filamentState=FILAMENT_HEATING;
static Timeout filamentTimeout;
void filamentMenuUpdate(void)	/*{{{*/
{
	static char heater_error[]= "My extruders are    not heating up.     Check my            connections!        ";
	int curTemp,heatIndex,i;
	static int lastHeatIndex=0;
	static uint8_t toggleCnt=0;
	static bool toggleBlink=true;
	if(FILAMENT_WAIT==filamentState)
	{
		if(extruderHasReachedTargetTemp())
		{
			filamentStartMotor();
			filamentState= FILAMENT_STOP;
		}
		else if(timeoutHasElapsed(&filamentTimeout))
		{
			OutputScreen(heater_error);
			filamentState = FILAMENT_DONE;
		}
		else	//����״��ʾ��ǰ�¶�/*{{{*/
		{
			curTemp=getCurExtruderTempature();
			heatIndex=(abs(curTemp)*20)/setTargetTemp;
			if(lastHeatIndex > heatIndex)
			{
				lcdSerialSetCursor(0,3);
				lcdSerialWriteString("                    ");
				lastHeatIndex=0;
			}
			lcdSerialSetCursor(lastHeatIndex,3);
			for(i=lastHeatIndex;i<heatIndex;i++)
				lcdSerialWrite(0xFF);
			lastHeatIndex= heatIndex;
			toggleCnt++;
			if(toggleCnt>6)
			{
				if(toggleBlink)
				{
					lcdSerialWriteString(" ");
					toggleBlink=false;
				}
				else
				{
					lcdSerialWrite(0xFF);
					toggleBlink=true;
				}
				toggleCnt=0;
			}
		}/*}}}*/
	}
}/*}}}*/
void filamentMenuFunc()/*{{{*/
{
	static char  explain_one[] = "Press down on the   grey rings at top ofthe extruders and   pull the black...   ";
	static char  explain_two[] = "guide tubes out. Nowfeed filament from  the back through thetubes until it...   ";
	static char  explain_oneS[]= "Press down on the   grey ring at top of the extruder and    pull the black...   ";
	static char  explain_twoS[]= "guide tube out.  Nowfeed filament from  the back through thetube until it...   ";
	static char  explain_thre[]= "pops out in front.  I'm heating up my   extruder so we can  load the filament...";
	static char  explain_four[]= "This might take a   few minutes.        And watch out, the  nozzle will get HOT!";
	static char  heating_bar[] = "Heating Progress:                                                               ";
	static char  heating[] 	= "I'm heating up my   extruder!           Please wait.                            ";
	static char  ready_right[] = "OK I'm ready!       First we'll load theright extruder.     Push filament in... ";
	static char  ready_single[]= "OK I'm ready!       Pop the guide tube  off and push the    filament down...    "; 
	static char  ready_rev[]   = "OK I'm ready!       Pop the guide tube  off and pull        filament gently...  ";
    static char  ready_left[]  = "Great! Now we'll    load the left       extruder. Push      filament down...    ";
    static char  tug[]         = "through the grey    ring until you feel the motor tugging   the plastic in...   ";
    static char  stop[]        = "When filament is    extruding out of thenozzle, Press 'M'   to stop extruding.  "; 
    static char  stop_exit[]   = "When filament is    extruding out of thenozzle, Press 'M'   to exit             "; 
    static char  stop_reverse[]= "When my filament is released,           Press 'M' to exit.                      ";
    static char  tryagain[]    = "OK! I'll keep my    motor running. You  may need to push    harder...           ";
    static char  go_on[]       = "We'll keep going.   If you're having    trouble, check out  makerbot.com/help   ";  
    static char  finish[]      = "Great!  I'll stop   running my extruder.Press M to continue.                    ";  
    static char  go_on_left[]  = "We'll keep going.   Lets try the left   extruder. Push      filament down...    ";
	int interval=300*1000000;
	commonMenuDispFunc();
	if(BC_Center==btnCode)
	{
		switch(filamentState)
		{
			case FILAMENT_HEATING:
				if(pCurItem == pCurMenuStartItem)
					isFilamentForward=true;
				else
					isFilamentForward=false;
				setTargetTemp=220;
				extruderStartHeat();
				OutputScreen(explain_one);
				timeoutStart(&filamentTimeout,300*1000000);
				break;
			case FILAMENT_EXPLAIN2:
				OutputScreen(explain_two);
				break;
			case FILAMENT_EXPLAIN3:
				OutputScreen(explain_thre);
				break;
			case FILAMENT_EXPLAIN4:
				OutputScreen(explain_four);
//				if(axisPosition[2] < 1000)
//				{
//					axisPosition[2]=60000;
//					interval=5000000;
//					motorSetTarget(axisPosition,interval,0x1f);
//				};
				break;
			case FILAMENT_HEAT_BAR:
				OutputScreen(heating_bar);
				break;
			case FILAMENT_WAIT://tofix :startup ���������ʲô��
				OutputScreen(heating_bar);
				break;
			case FILAMENT_START:
				if(isFilamentForward)
					OutputScreen(ready_single)
				else
					OutputScreen(ready_rev)
				break;
			case FILAMENT_TUG:
				OutputScreen(tug);
				break;
			case FILAMENT_STOP:
				if(isFilamentForward)
					OutputScreen(stop_exit)
				else
					OutputScreen(stop_reverse)
				break;
			case FILAMENT_OK:
				filamentStopMotor();
				extruderStopHeat();
				break;
			case FILAMENT_DONE:
			case FILAMENT_EXIT:
				filamentStopMotor();
				//if(filamentSuccess==SUCCESS))
				OutputScreen(finish);
				break;

		}
		if(filamentState < FILAMENT_EXIT)
			filamentState++;
		else
			filamentState=FILAMENT_HEATING;
	}
	if(BC_Left == btnCode)
	{
		if(NULL != (pCurItem->pParentMenu))
			showParentItem();
	}
	btnCode= BC_NULL;
}/*}}}*/
void filamentStartMotor(void)/*{{{*/
{
	int32_t interval=300*1000000;
	int32_t steps=interval/6250;
	int pt[STEPPER_COUNT];
	pt[0]=pt[1]=pt[2]=0;
	if(isFilamentForward)
		steps*=-1;
	pt[3]=steps;
	motorSetTargetNew(pt,interval,0x1f);
}/*}}}*/
void filamentStopMotor(void)/*{{{*/
{
	int i;
	motorAbort();
	for(i=0;i<STEPPER_COUNT;i++)
		axisSetEnablePin(i,false);
}/*}}}*/
//void utilityMenuUpdate(void)/*{{{*/
//{
//	if(UtilityLevelPlate == curMenuState)
//		msgMenuUpdate();
//}/*}}}*/
void utilityMenuFunc()/*{{{*/
{
	if(UtilityLevelPlate == curMenuState)/*{{{*/
	{
		msgMenuFunc(); // �� msgMenuUpdate �����ж����utilityIsPlaying()���������޸�curMenuState=NormalMenu;
	}/*}}}*/
	else if(UtilityCancel == curMenuState)
	{
		if(BC_Up==btnCode || BC_Down==btnCode)
			cancelFaceDispFunc();
		if(BC_Center==btnCode)
		{
			if(isCancel)
			{
				stopUtilityScript();
				curMenuState=NormalMenu;
				showCurItem(); //��ʾ utility����Ŀ¼
			}
			else
			{
				curMenuState=UtilityLevelPlate;
				needsRedraw=true;//��ȡ����������menuUpdate()ʱ������ʾ֮ǰ������
				lcdSerialSetCursor(0,0);//��Ȼ ����ʾ�� yes �ĺ��棬��Ϊ msgMenuUpdate�����޸�����
			}
		}
	}
	else if(NormalMenu== curMenuState)/*{{{*/
	{
		commonMenuDispFunc();//����һ��İ���
		switch(btnCode)
		{
			case BC_Center:
				if(NULL!=(pCurItem->pChildMenu))
					showChildItem();
				else if(pCurItem == &utilityMenu[3]) //home axes
					hostStartOnboardBuild(US_HOME_AXES);
				else if(pCurItem == &utilityMenu[8]) //level build palate
				{
					hostStartOnboardBuild(US_LEVEL_PLATE_STARTUP);
					lcdSerialClear();
					lcdSerialSetCursor(0,0);
					lcdSerialWriteString("start level plate");
					curMenuState=UtilityLevelPlate;
				}
				else if(pCurItem == &utilityMenu[14]) //calibrate nozzles
					hostStartOnboardBuild(US_TOOLHEAD_CALIBRATE);
				break;
			case BC_Left:
				if(NULL!=(pCurItem->pParentMenu))
					showParentItem();
				break;
		}
	}/*}}}*/
	btnCode=BC_NULL;

}/*}}}*/
void sdMenuFunc()  //{{{
{
	if(curMenuState == SdPrintCancel)
	{
		if(btnCode== BC_Up || btnCode== BC_Down)
			cancelFaceDispFunc();
		if(btnCode ==BC_Center)
		{
			if(isCancel)
			{
				stopSdPrint();
				curMenuState=NormalMenu;
				showParentItem();
//				sdMenuDrawFile();
			}
			else
				curMenuState=SdPrintHeat;
		}
		btnCode=BC_NULL;
	}
	else if(SdPrintHeat == curMenuState)
	{
		if(btnCode==BC_Left)
		{
			showCancelFace();
			curMenuState=SdPrintCancel;
		}
		btnCode=BC_NULL;
	}
	else if(SdPrintFinish == curMenuState)
	{
		if(BC_Center==btnCode)
		{
			curMenuState=NormalMenu;
			showParentItem();
		}
		btnCode=BC_NULL;
	}
	else if(NormalMenu==curMenuState) //���ﲻ����else ,��Ϊ��cancelFace �� �� ���أ���Ҫ����ʾsd files
	{			//������else�� ���� sdMenuDrawFile �ŵ� cancelFace����
		SdErrorCode e;
			
		switch(btnCode)
		{
			case BC_Center:
				if(HOST_STATE_READY !=hostCurrentState)
				{
					showErrMsg("already building!");
					btnCode=BC_NULL;
					return;
				}
				if(sdFileCnt == 0)
					return;
				strcpy(sdPrintFileName,sdFileName[sdFileCurIdx]);
				e=hostStartBuildFromSD();
				if(SD_SUCCESS !=e)
				{
					showErrMsg("sd read error");
					btnCode=BC_NULL;
					return;
				}
				extruderStartHeat();
				platformStartHeat();
				cmdMode=READY;
				curMenuState=SdPrintHeat;
				heatingMenuUpdate();
				btnCode=BC_NULL;
				return;
				break;
			case BC_Down:
				if(sdFileCurIdx < (sdFileCnt-1))
				{
					if(sdFileCurIdx - sdFileDispStartIdx ==(LCD_MaxItem-1))
						sdFileDispStartIdx++;
					sdFileCurIdx++;
				}
				break;
			case BC_Up:
				if(sdFileCurIdx > 0)
				{
					if(sdFileCurIdx == sdFileDispStartIdx)
						sdFileDispStartIdx--;
					sdFileCurIdx--;
				}
				break;
			case BC_Left:
				if(NULL!=(pCurItem->pParentMenu))
					showParentItem();
				break;
			default:
				break;
		}
		btnCode=BC_NULL;
		sdMenuDrawFile();
	}
}
//}}}
void sdMenuDrawFile()  //{{{
{
	int i,fc;
	fc=sdFileDispStartIdx;
	lcdSerialClear();
	if(sdFileCnt == 0)
	{
		lcdSerialWriteString("no s3g file");
		return;
	}
	for(i=0;i<MIN(sdFileCnt,LCD_MaxItem);i++)
	{
		lcdSerialSetCursor(2,i);
		lcdSerialWriteString(sdFileName[fc]);
		if(fc==sdFileCurIdx)
		{
			lcdSerialSetCursor(0,i);
			lcdSerialWriteString("->");
		}
		fc++;
	}
}
//}}}
void heatingMenuUpdate(void)/*{{{*/
{
	char tempStr[20];
	int curTemp;
	if(!sdCardIsPlaying)
	{
		curMenuState=SdPrintFinish;
		lcdSerialClear();
		lcdSerialSetCursor(0,0);
		lcdSerialWriteString("Print Finish!");
		return;
	}
	curTemp=getCurExtruderTempature();
	lcdSerialClear();
	lcdSerialSetCursor(0,0);
	sprintf(tempStr,"Ex:%03d/%03d",curTemp,setTargetTemp);
	lcdSerialWriteString(tempStr);
	curTemp=getCurPlatformTempature();
	sprintf(tempStr,"PF:%03d/%03d",curTemp,setPlatformTargetTemp);
	lcdSerialSetCursor(0,1);
	lcdSerialWriteString(tempStr);
//	if(BC_NULL!=btnCode)
//	{
//		if(BC_Left == btnCode)
//			showParentItem();
//		btnCode=BC_NULL;// ��ֹ�ڸý��水�°������һֱ����Ӧ
//	}
}/*}}}*/
//addMsg,msgMenu �������Ϊ��Щ��������Ҫ��LCD��ʾ��Ϣ��HOST_CMD_DISPLAY_MESSAGE, ͬʱ����Ҫ WAIT_ON_BUTTON�ȴ��������� 
void addMsg(char str[],uint8_t options,uint8_t xpos,uint8_t ypos)/*{{{*/
{
	char * letter=str;
//	if(pCurItem != msgMenu)
//		pushMenuStack(msgMenu);
	while(*letter !='\0')
	{
		message[cursor++]=*letter;
		letter++;
	}
	if(cursor >= (LCD_SCREEN_WIDTH*LCD_SCREEN_HEIGHT+1))
		message[LCD_SCREEN_WIDTH*LCD_SCREEN_HEIGHT] = '\0';
	else
		message[cursor]='\0';
	if((options &(1<<0)) ==0) //�Ƿ���Ҫ����
		lcdSerialClear();
	lcdSerialSetCursor(xpos,ypos);//tofixme? 
	if((options &(1<<1))!=0) //�ַ����Ƿ��Ѿ����
	{
		needsRedraw=true;
	}
	else
		needsRedraw=false;
	if((options &(1<<2))!=0) //�Ƿ���Ҫ�ȴ�����
	{
		cmdMode=WAIT_ON_BUTTON;
	}
	if(pCurMenuStartItem == &utilityMenu[0])
		curMenuState=UtilityLevelPlate;
}/*}}}*/
void msgMenuUpdate(void)/*{{{*/
{
	if(needsRedraw)
	{
		lcdSerialWriteString(message);
		cursor=0;//�ƺ�ֻ�ܷ��������ˡ�����
		needsRedraw=false;
	}
	if(!utilityIsPlaying())
		{
			curMenuState=NormalMenu;
			showCurItem();
		}
}/*}}}*/
void msgMenuFunc(void)/*{{{*/
{
	if(btnCode == BC_Center)
	{
		cmdMode=READY;
	}
	else if(BC_Left==btnCode)
	{
		showCancelFace();
		curMenuState=UtilityCancel;
	}
	btnCode=BC_NULL;
}/*}}}*/
void showCancelFace(void)/*{{{*/
{
	lcdSerialClear();
	lcdSerialSetCursor(2,0);
	lcdSerialWriteString("really cancel?");
	lcdSerialSetCursor(0,1);
	lcdSerialWriteString("->no");
	lcdSerialSetCursor(2,2);
	lcdSerialWriteString("yes");
	isCancel=false;
}	/*}}}*/
bool cancelFaceDispFunc(void)/*{{{*/
{
	if(btnCode==BC_Down && isCancel==false)
	{
		lcdSerialSetCursor(0,1);
		lcdSerialWriteString("  ");
		lcdSerialSetCursor(0,2);
		lcdSerialWriteString("->");
		isCancel=true;
	};
	if(btnCode==BC_Up && isCancel==true)
	{
		lcdSerialSetCursor(0,1);
		lcdSerialWriteString("->");
		lcdSerialSetCursor(0,2);
		lcdSerialWriteString("  ");
		isCancel=false;
	}
	btnCode=BC_NULL;
}/*}}}*/
void stopSdPrint(void)/*{{{*/
{
	commandReset();
	sdCardFinishPlayback();
	extruderStopHeat();
	platformStopHeat();
	motorDisable();
	cmdPaused=true;
}/*}}}*/
void stopUtilityScript(void)/*{{{*/
{
	commandReset();
	utilityFinishPlayback();
	extruderStopHeat();
	platformStopHeat();
	motorDisable();
	cmdPaused=true;
}/*}}}*/
