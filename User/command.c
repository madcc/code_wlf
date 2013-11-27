//command 

#include "command.h"
#include "menu.h"
#include "tempature.h"
#include "motor.h"
#include "utilityScripts.h"
#include <stdio.h>
CommandState cmdMode = WAIT_ON_TOOL;

//#define Debug_Cmd
#define Debug_Data_Cmd

extern bool endStopFlagTmpDebug;

circleBuf cmdBuf;
u16 cmdSdFailCnt;
u32 cmdSdByteCnt;
bool cmdSdReset;
bool cmdPaused;
bool cmdHeatShutdown;
uint8_t currentToolIndex = 0;
Timeout cmdHomingTimeout;
Timeout cmdDelayTimeout;
Timeout cmdToolWaitTimeout;
Timeout cmdBtnWaitTimeout;
u32 sdByteCnt;
uint8_t sdByte;
u32 cmdCnt;
//不把这两个函数放到头文件里去，这样就不会被其他 地方调用了
uint8_t pop8()/*{{{*/
{
	return circleBufPop(&cmdBuf);
}/*}}}*/
int16_t pop16() /*{{{*/
{
	union {
		// AVR is little-endian
		int16_t a;
		struct {
			uint8_t data[2];
		} b;
	} shared;
	shared.b.data[0] = circleBufPop(&cmdBuf);
	shared.b.data[1] = circleBufPop(&cmdBuf);
	return shared.a;
}/*}}}*/

int32_t pop32() /*{{{*/
{
	union {
		// AVR is little-endian
		int32_t a;
		struct {
			uint8_t data[4];
		} b;
	} shared;
	shared.b.data[0] = circleBufPop(&cmdBuf);
	shared.b.data[1] = circleBufPop(&cmdBuf);
	shared.b.data[2] = circleBufPop(&cmdBuf);
	shared.b.data[3] = circleBufPop(&cmdBuf);
	return shared.a;
}/*}}}*/
bool processExtruderCommandPacket(void) /*{{{*/
{
    uint8_t	id = circleBufPop(&cmdBuf);
		uint8_t command = circleBufPop(&cmdBuf);
		uint8_t length = circleBufPop(&cmdBuf);
		
//		int32_t x = 0;
//        int32_t y = 0;
//        int32_t z = 0;
        int32_t a = 0;
        int32_t b = 0;
//        int32_t us = 1000000;
//        uint8_t relative = 0x02;
    bool enable = false;
		int32_t value;
		switch (command) 
		{
		case SLAVE_CMD_SET_TEMP:	
			value=pop16();// 注释掉下面的，但是别忘了 pop;
			setTargetTemp=value;
//			board.getExtruderBoard(id).getExtruderHeater().set_target_temperature(pop16());
//			if(board.getPlatformHeater().isHeating()){//FIXME
//				board.getExtruderBoard(id).getExtruderHeater().Pause(true);}
#ifdef Debug_Data_Cmd
			printf("SLAVE_CMD_SET_TEMP %d\n",value);
#endif
			return true;
		// can be removed in process via host query works OK
 		case SLAVE_CMD_PAUSE_UNPAUSE:
			//pause(!command::isPaused());
			if(cmdPaused)
				cmdPaused=false;
			else
				cmdPaused=true;
#ifdef Debug_Data_Cmd
			printf("SLAVE_CMD_PAUSE_UNPAUSE %d\n",value);
#endif
			return true;
		case SLAVE_CMD_TOGGLE_FAN: //FIXME
			value=circleBufPop(&cmdBuf);
			//board.getExtruderBoard(id).setFan((pop8() & 0x01) != 0);
#ifdef Debug_Data_Cmd
			printf("SLAVE_CMD_TOGGLE_FAN %d\n",value);
#endif
			return true;
		case SLAVE_CMD_TOGGLE_VALVE: //FIXME 
			value=circleBufPop(&cmdBuf);
			//board.setValve((pop8() & 0x01) != 0);
#ifdef Debug_Data_Cmd
			printf("SLAVE_CMD_TOGGLE_VALVE %d\n",value);
#endif
			return true;
		case SLAVE_CMD_SET_PLATFORM_TEMP:
			value=pop16();
			setPlatformTargetTemp=value;
			extruderSetHeatPause(true);
//			board.setUsingPlatform(true);
//			board.getPlatformHeater().set_target_temperature(pop16());
//			// pause extruder heaters if active
//			board.getExtruderBoard(0).getExtruderHeater().Pause(true);
//			board.getExtruderBoard(1).getExtruderHeater().Pause(true);
#ifdef Debug_Data_Cmd
			printf("SLAVE_CMD_SET_PLATFORM_TEMP %d\n",value);
#endif
			return true;
        // not being used with 5D
		case SLAVE_CMD_TOGGLE_MOTOR_1:
			//DEBUG_PIN1.setValue(true);
			enable = circleBufPop(&cmdBuf) & 0x01 ? true:false;
			cmdMode = MOVING;
			axisSetEnablePin(3,enable); // 应该就是放在这里 
			//steppers::enableAxis(4, enable); //这里是4 那么应该用不到 
			b = 360;
#ifdef Debug_Data_Cmd
			printf("SLAVE_CMD_TOGGLE_MOTOR_1 %d\n",value);
#endif
			return true;
        // not being used with 5D
		case SLAVE_CMD_TOGGLE_MOTOR_2: 
			//DEBUG_PIN1.setValue(true);
			enable = circleBufPop(&cmdBuf) & 0x01 ? true:false;
			//steppers::enableAxis(3, enable);
			axisSetEnablePin(3,enable); //错了，这里3,4应该对应到喷嘴了 _note ? 
			a = 160;
#ifdef Debug_Data_Cmd
			printf("SLAVE_CMD_TOGGLE_MOTOR_2 %d\n",value);
#endif
			return true;
		case SLAVE_CMD_SET_MOTOR_1_PWM:
			value=circleBufPop(&cmdBuf);
#ifdef Debug_Data_Cmd
			printf("SLAVE_CMD_SET_MOTOR_1_PWM %d\n",value);
#endif
			return true;
		case SLAVE_CMD_SET_MOTOR_2_PWM:
			value=circleBufPop(&cmdBuf);
#ifdef Debug_Data_Cmd
			printf("SLAVE_CMD_SET_MOTOR_2_PWM %d\n",value);
#endif
			return true;
		case SLAVE_CMD_SET_MOTOR_1_DIR:
			value=circleBufPop(&cmdBuf);
#ifdef Debug_Data_Cmd
			printf("SLAVE_CMD_SET_MOTOR_1_DIR %d\n",value);
#endif
			return true;
		case SLAVE_CMD_SET_MOTOR_2_DIR:
			value=circleBufPop(&cmdBuf);
#ifdef Debug_Data_Cmd
			printf("SLAVE_CMD_SET_MOTOR_2_DIR %d\n",value);
#endif
			return true;
		case SLAVE_CMD_SET_MOTOR_1_RPM:
			value=pop32();
#ifdef Debug_Data_Cmd
			printf("SLAVE_CMD_SET_MOTOR_1_RPM %d\n",value);
#endif
			return true;
		case SLAVE_CMD_SET_MOTOR_2_RPM:
			value=pop32();
#ifdef Debug_Data_Cmd
			printf("SLAVE_CMD_SET_MOTOR_2_RPM %d\n",value);
#endif
			return true;
		}
	return false;
}/*}}}*/

void runCmdSlice(void)
{
	// get command from SD card if building from SD
	if (sdCardIsPlaying) /*{{{*/
	{
		//printf("sdCardHasNextByte  %d  \r\n",sdCardHasNextByte);
		while (circleBufGetRemainCap(&cmdBuf) > 0 && sdCardHasNextByte) 
		{
			sdByteCnt++;
			//printf("sdByteCnt  %d  \r\n",sdByteCnt);
			sdByte=sdCardPlaybackNext();
			//printf("sdByte  %d \r\n",sdByte);
			circleBufPush(&cmdBuf,sdByte);
		}
		if( !sdCardHasNextByte && circleBufIsEmpty(&cmdBuf))
		{
			sdCardFinishPlayback();
			//printf("执行sdCardFinishPlayback\r\n");
		}
	}/*}}}*/
	// get command from onboard script if building from onboard 
	if(utilityIsPlaying())/*{{{*/
	{		
		while (circleBufGetRemainCap(&cmdBuf) > 0 && utilityPlaybackHasNext())
		{
			circleBufPush(&cmdBuf,utilityPlaybackNext());
		}
		if(!utilityPlaybackHasNext() && circleBufIsEmpty(&cmdBuf))
		{
			utilityFinishPlayback();
		}
	}/*}}}*/
	// don't execute commands if paused or shutdown because of heater failure
	// 如果命令暂停或者 加热失败，则直接返回
	if (cmdPaused || cmdHeatShutdown) {	return; }

	if (cmdMode == HOMING) 
	{
		if (!(motorIsRunning || motorIsHoming)) 
			cmdMode = READY;
		else if (timeoutHasElapsed(&cmdHomingTimeout)) 
		{
			motorAbort();
			cmdMode = READY;
		}
	}
	if (cmdMode == MOVING) 
	{
		if (!(motorIsRunning  || motorIsHoming))
			cmdMode = READY; 
	}
	if (cmdMode == DELAY)
	{
		// check timers
		if (timeoutHasElapsed(&cmdDelayTimeout))
		{
			cmdMode = READY;
		}
	}
	if (cmdMode == WAIT_ON_TOOL) /*{{{*/
	{
		int i;
		if(timeoutHasElapsed(&cmdToolWaitTimeout))
		{
			//Motherboard::getBoard().errorResponse("Extruder Timeout    when attempting to  heat");
			printf("Extruder Timeout    when attempting to  heat");
			showErrMsg("Extruder Timeout     when attempting to heat");
			cmdMode = READY;		
		}
		else if(extruderHasReachedTargetTemp())
		{
			cmdMode = READY;
			for(i=0;i<STEPPER_COUNT;i++)  //呃，放在这里不好 
				axisSetEnablePin(i,true);
		}
		// if platform is done heating up, unpause the extruder heaters
		else if(platformHasReachedTargetTemp())
		{
			extruderSetHeatPause(false); //平台温度达到，开始加热喷头
		}
	}/*}}}*/
	if (cmdMode == WAIT_ON_PLATFORM) /*{{{*/
	{
		if(timeoutHasElapsed(&cmdToolWaitTimeout))
		{
			//		Motherboard::getBoard().errorResponse("Platform Timeout    when attempting to  heat");
			printf("Platform Timeout    when attempting to  heat");
			cmdMode = READY;		
		}
		else if(platformHasReachedTargetTemp())
		{
			extruderSetHeatPause(false);
			cmdMode = READY;
		}
	}/*}}}*/
	if (cmdMode == WAIT_ON_BUTTON) /*{{{*/
	{
		if (timeoutHasElapsed(&cmdBtnWaitTimeout))
		{
			cmdMode = READY;
		}
		else 
		{
			//cmdMode = READY; 由msgMenu去处理button
		}
	}/*}}}*/

	if (cmdMode == READY) 
	{
		int pt[STEPPER_COUNT];//toFix
		int32_t a ; 
		int32_t b ; 
		int32_t dda; 
		uint8_t axes ;
		uint8_t i;
		// process next command on the queue.
		if (cmdBuf.length>0)
		{
			uint8_t command = *circleBufGetData(&cmdBuf,0);//command_buffer[0];
			timeoutStart(&usrInputTimeout,usrInputTime);
		cmdCnt++;
		printf("c %6d ",cmdCnt);
			if (command == HOST_CMD_QUEUE_POINT_ABS) 
			{
				// check for completion
				if (cmdBuf.length >= 17) 
				{
					circleBufPop(&cmdBuf); // remove the command code
					cmdMode = MOVING;
					pt[0] = pop32();
					pt[1] = pop32();
					pt[2] = pop32();
					dda = pop32();
					motorSetTarget(pt,dda);//steppers::setTarget(Point(x,y,z),dda);
#ifdef Debug_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_QUEUE_POINT_ABS\r\n");
#endif
#ifdef Debug_Data_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_QUEUE_POINT_ABS ");
					printf("%d %d %d %d\r\n",pt[0],pt[1],pt[2],dda);
#endif
				}
			} else if (command == HOST_CMD_QUEUE_POINT_EXT) 
			{
				// check for completion
				if (cmdBuf.length >= 25) 
				{
					circleBufPop(&cmdBuf); // remove the command code
					cmdMode = MOVING;
					pt[0] = pop32();
					pt[1]= pop32();
					pt[2]= pop32();
					pt[3]= pop32();
					b= pop32();
					dda = pop32();
					motorSetTarget(pt,dda);//toFix
					//steppers::setTarget(Point(x,y,z,a,b),dda);
#ifdef Debug_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_QUEUE_POINT_EXT\r\n");
#endif
#ifdef Debug_Data_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_QUEUE_POINT_EXT ");
					printf("%d %d %d %d %d\r\n",pt[0],pt[1],pt[2],pt[3],dda);
#endif
				}
			} else if (command == HOST_CMD_QUEUE_POINT_NEW) 
			{
				// check for completion
				if (cmdBuf.length >= 26) 
				{
					uint8_t relative ;
					int32_t us; 
					circleBufPop(&cmdBuf); // remove the command code
					cmdMode = MOVING;
					pt[0] = pop32();
					pt[1]= pop32();
					pt[2]= pop32();
//					if(pt[0]>5000)
//						pt[0]=5000;
//					if(pt[0]<-5000)
//						pt[0]=-5000;
//					if(pt[1]>2000)
//						pt[1]=2000;
//					if(pt[1]<-2000)
//						pt[1]=-2000;
#define PT2_THRESHOLD 5000
					if(pt[2]>PT2_THRESHOLD)
						pt[2]=PT2_THRESHOLD;
					if(pt[2]<-PT2_THRESHOLD)
						pt[2]=-PT2_THRESHOLD;
					pt[3]= pop32();
					b= pop32();
					us = pop32();
					relative = pop8();
					motorSetTargetNew(pt,us,relative);
					//steppers::setTargetNew(Point(x,y,z,a,b),us,relative);
#ifdef Debug_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_QUEUE_POINT_NEW ");
					printf("%d %d %d %d %d %d\r\n",pt[0],pt[1],pt[2],pt[3],us,relative);
#endif
#ifdef Debug_Data_Cmd
					if(endStopFlagTmpDebug)
						printf("x\r\n");
					LED_BLINK(1);
					printf("HOST_CMD_QUEUE_POINT_NEW ");
					printf("%d %d %d %d %d %d %d\r\n",pt[0],pt[1],pt[2],pt[3],us,relative,b);
#endif
				}
			} else if (command == HOST_CMD_CHANGE_TOOL) 
			{
				if (cmdBuf.length >= 2) 
				{
					circleBufPop(&cmdBuf); // remove the command code
					currentToolIndex = circleBufPop(&cmdBuf);
					//toFix
					//steppers::changeToolIndex(currentToolIndex);
#ifdef Debug_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_CHANGE_TOOL\r\n");
#endif
#ifdef Debug_Data_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_CHANGE_TOOL\r\n");
#endif
				}
			} else if (command == HOST_CMD_ENABLE_AXES) /*{{{*/
			{
				if (cmdBuf.length >= 2) 
				{
					bool enable;
					circleBufPop(&cmdBuf); // remove the command code
					axes = circleBufPop(&cmdBuf);
					enable = (axes & 0x80) != 0;
					for (i = 0; i < STEPPER_COUNT; i++) 
					{
						if ((axes & (1<<i)) != 0) 
							//steppers::enableAxis(i, enable);
							axisSetEnablePin(i,enable);
					}
#ifdef Debug_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_ENABLE_AXES\r\n");
#endif
#ifdef Debug_Data_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_ENABLE_AXES ");
					printf("%d\r\n",axes);
#endif
				}/*}}}*/
			} else if (command == HOST_CMD_SET_POSITION) 
			{
				// check for completion
				if (cmdBuf.length >= 13) 
				{
					circleBufPop(&cmdBuf); // remove the command code
					pt[0] = pop32();
					pt[1]= pop32();
					pt[2] = pop32();//toFix,motorDefinePosition 怎么知道pt有没有第四个参数，会怎么对pt[3]处理？
					//steppers::definePosition(Point(x,y,z));
					motorDefinePosition(pt);
#ifdef Debug_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_SET_POSITION\r\n");
#endif
#ifdef Debug_Data_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_SET_POSITION ");
					printf("%d %d %d\r\n",pt[0],pt[1],pt[2]);
#endif
				}
			} else if (command == HOST_CMD_SET_POSITION_EXT) 
			{
				// check for completion
				if (cmdBuf.length >= 21) 
				{
					circleBufPop(&cmdBuf); // remove the command code
					pt[0] = pop32();
					pt[1] = pop32();
					pt[2] = pop32();
					pt[3] = pop32();
					//a = pop32();
					b = pop32();
					motorDefinePosition(pt);//toFix
					//steppers::definePosition(Point(x,y,z,a,b));
#ifdef Debug_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_SET_POSITION_EXT\r\n");
#endif
#ifdef Debug_Data_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_SET_POSITION_EXT ");
					printf("%d %d %d %d\r\n",pt[0],pt[1],pt[2],pt[3]);
#endif
				}
			} else if (command == HOST_CMD_DELAY) 
			{
				if (cmdBuf.length >= 5) 
				{
					uint32_t microseconds ;
					cmdMode = DELAY;
					circleBufPop(&cmdBuf); // remove the command code
					// parameter is in milliseconds; timeouts need microseconds
					microseconds = pop32() * 1000;
					timeoutStart(&cmdDelayTimeout,microseconds);//delay_timeout.start(microseconds);
#ifdef Debug_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_DELAY\r\n");
#endif
#ifdef Debug_Data_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_DELAY\r\n");
#endif
				}
			} else if (command == HOST_CMD_PAUSE_FOR_BUTTON) 
			{
				if (cmdBuf.length >= 5) 
				{
					uint16_t timeout_seconds;
					uint8_t button_mask;
					uint8_t button_timeout_behavior;
					circleBufPop(&cmdBuf); // remove the command code
					button_mask = circleBufPop(&cmdBuf);
					timeout_seconds = pop16();
					button_timeout_behavior = circleBufPop(&cmdBuf);
//					cmdMode = WAIT_ON_BUTTON;//toFix
#ifdef Debug_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_PAUSE_FOR_BUTTON\r\n");
#endif
#ifdef Debug_Data_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_PAUSE_FOR_BUTTON\r\n");
#endif
				}
			} else if (command == HOST_CMD_DISPLAY_MESSAGE) 
			{
			//	MessageScreen* scr = Motherboard::getBoard().getMessageScreen();
				if (cmdBuf.length >= 6) 
				{
					uint8_t options ;
					uint8_t xpos ;
					uint8_t ypos ;
					uint8_t timeout_seconds;
					uint8_t idx=0;
					char msg[50];
					circleBufPop(&cmdBuf); // remove the command code
					options = circleBufPop(&cmdBuf);
					xpos = circleBufPop(&cmdBuf);
					ypos = circleBufPop(&cmdBuf);
					timeout_seconds = circleBufPop(&cmdBuf);
					do
					{
						msg[idx++]=circleBufPop(&cmdBuf);
					}while(msg[idx-1]!='\0');
					addMsg(msg,options,xpos,ypos);
					//timeoutStart(&cmdBtnWaitTimeout,300*1000000);
					timeoutInit(&cmdBtnWaitTimeout);
#ifdef Debug_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_DISPLAY_MESSAGE\r\n");
#endif
#ifdef Debug_Data_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_DISPLAY_MESSAGE %d %d %d %s\r\n",options,xpos,ypos,msg);
#endif
				}

			} else if (command == HOST_CMD_FIND_AXES_MINIMUM ||
					command == HOST_CMD_FIND_AXES_MAXIMUM) 
			{
				if (cmdBuf.length >= 8) 
				{
					uint8_t flags ;
					uint32_t feedrate ;
					uint16_t timeout_s ;
					bool direction ;
					circleBufPop(&cmdBuf); // remove the command
					flags = pop8();
					feedrate = pop32(); // feedrate in us per step
					timeout_s = pop16();
					direction = command == HOST_CMD_FIND_AXES_MAXIMUM;
					cmdMode = HOMING;
					timeoutStart(&cmdHomingTimeout,timeout_s*1000L*1000L);//homing_timeout.start(timeout_s * 1000L * 1000L);
//					//timeoutStart(&cmdHomingTimeout,2*1000L*1000L);//homing_timeout.start(timeout_s * 1000L * 1000L);
//					//steppers::startHoming(command==HOST_CMD_FIND_AXES_MAXIMUM,flags,feedrate);
					motorStartHoming(command==HOST_CMD_FIND_AXES_MAXIMUM,flags,feedrate);
#ifdef Debug_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_FIND_AXES_MINIMUM \r\n");
#endif
#ifdef Debug_Data_Cmd
					LED_BLINK(1);
					if(command == HOST_CMD_FIND_AXES_MINIMUM)
						printf("HOST_CMD_FIND_AXES_MINIMUM %d %d %d\r\n",command,flags,feedrate);
					else
						printf("HOST_CMD_FIND_AXES_MAXIMUM %d %d %d\r\n",command,flags,feedrate);
#endif
				}
			} else if (command == HOST_CMD_WAIT_FOR_TOOL) 
			{
				if (cmdBuf.length >= 6) 
				{
					uint16_t toolPingDelay ;
					uint16_t toolTimeout ;
					cmdMode = WAIT_ON_TOOL;
					circleBufPop(&cmdBuf);
					currentToolIndex = circleBufPop(&cmdBuf);
					toolPingDelay = (uint16_t)pop16();
					toolTimeout = (uint16_t)pop16();
					//tool_wait_timeout.start(toolTimeout*1000000L);//toFix
#ifdef Debug_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_WAIT_FOR_TOOL\r\n");
#endif
#ifdef Debug_Data_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_WAIT_FOR_TOOL\r\n");
#endif
				}
			} else if (command == HOST_CMD_WAIT_FOR_PLATFORM) 
			{
				// FIXME: Almost equivalent to WAIT_FOR_TOOL
				if (cmdBuf.length >= 6) 
				{
					uint8_t currentToolIndex ;
					uint16_t toolPingDelay ;
					uint16_t toolTimeout ;
					cmdMode = WAIT_ON_PLATFORM;
					circleBufPop(&cmdBuf);
					currentToolIndex = circleBufPop(&cmdBuf);
					toolPingDelay = (uint16_t)pop16();
					toolTimeout = (uint16_t)pop16();
					//tool_wait_timeout.start(toolTimeout*1000000L);//toFix
#ifdef Debug_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_WAIT_FOR_PLATFORM\r\n");
#endif
#ifdef Debug_Data_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_WAIT_FOR_PLATFORM\r\n");
#endif
				}
			} else if (command == HOST_CMD_STORE_HOME_POSITION) 
			{

				// check for completion
				if (cmdBuf.length >= 2) 
				{
//					uint16_t offset ;
//					uint32_t position ;
					circleBufPop(&cmdBuf);
					axes = pop8();
					// Go through each axis, and if that axis is specified, read it's value,
					// then record it to the eeprom.
//					for (i = 0; i < STEPPER_COUNT; i++) 
//					{
//						if ( axes & (1 << i) ) 
//						{
//							offset = eeprom_offsets::AXIS_HOME_POSITIONS + 4*i;
//							position = steppers::getPosition()[i];
//							cli();
//							eeprom_write_block(&position, (void*) offset, 4);
//							sei();
//						}
//					}
#ifdef Debug_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_STORE_HOME_POSITION\r\n");
#endif
#ifdef Debug_Data_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_STORE_HOME_POSITION\r\n");
#endif
				}
			} 
			else if (command == HOST_CMD_RECALL_HOME_POSITION) 
			{
				// check for completion
				if (cmdBuf.length >= 2) 
				{
					//uint16_t offset ;
					circleBufPop(&cmdBuf);
					axes = pop8();

					//pt[0]=0;
					//pt[1]=3000;
					//pt[2]=0;
					//pt[3]=0;
//					pt[0]=12000;
//					pt[1]=9000;
					pt[0]=9000;
					pt[1]=6000;
					pt[2]=0;
					pt[3]=0;
					motorDefinePosition(pt);
#ifdef Debug_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_RECALL_HOME_POSITION\r\n");
#endif
#ifdef Debug_Data_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_RECALL_HOME_POSITION\r\n");
#endif
				}

			}else if (command == HOST_CMD_SET_POT_VALUE)
			{
				if (cmdBuf.length >= 2) 
				{
					uint8_t axis ;
					uint8_t value ;
					circleBufPop(&cmdBuf); // remove the command code
					axis = pop8();
					value = pop8();
//					steppers::setAxisPotValue(axis, value);
//					steppers::setAxisPotValue(axis, value);
#ifdef Debug_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_SET_POT_VALUE\r\n");
#endif
#ifdef Debug_Data_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_SET_POT_VALUE\r\n");
#endif
				}
			}else if (command == HOST_CMD_SET_RGB_LED)
			{
				if (cmdBuf.length >= 2) 
				{
					uint8_t red  ;
					uint8_t green ;
					uint8_t blue ;
					uint8_t blink_rate ;
					uint8_t effect;
					circleBufPop(&cmdBuf); // remove the command code

					red = pop8();
					green = pop8();
					blue = pop8();
					blink_rate = pop8();

					effect = pop8();

					//RGB_LED::setLEDBlink(blink_rate);
					//RGB_LED::setCustomColor(red, green, blue);
#ifdef Debug_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_SET_RGB_LED\r\n");
#endif
#ifdef Debug_Data_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_SET_RGB_LED\r\n");
#endif

				}
			}else if (command == HOST_CMD_SET_BEEP)
			{
				if (cmdBuf.length >= 2)
				{
					uint8_t frequency;
					uint8_t beep_length ;
					uint8_t effect ;
					circleBufPop(&cmdBuf); // remove the command code
					frequency= pop16();
					beep_length = pop16();
					effect = pop8();
					//Piezo::setTone(frequency, beep_length);
#ifdef Debug_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_SET_BEEP\r\n");
#endif
#ifdef Debug_Data_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_SET_BEEP\r\n");
#endif
				}			
			}else if (command == HOST_CMD_TOOL_COMMAND) 
			{
				if (cmdBuf.length >= 4) 
				{ // needs a payload
					uint8_t payload_length =*circleBufGetData(&cmdBuf,3);// command_buffer[3];
					if (cmdBuf.length >= 4+payload_length) 
					{
						circleBufPop(&cmdBuf); // remove the command code
						processExtruderCommandPacket(); 
#ifdef Debug_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_TOOL_COMMAND\r\n");
#endif
#ifdef Debug_Data_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_TOOL_COMMAND\r\n");
#endif
					}
				}
			}
			else if (command == HOST_CMD_SET_BUILD_PERCENT)
			{
				if (cmdBuf.length >= 2)
				{
					uint8_t percent ;
					uint8_t ignore ;
					circleBufPop(&cmdBuf); // remove the command code
					percent = pop8();
					ignore = pop8(); // remove the reserved byte
					//interface::setBuildPercentage(percent);
#ifdef Debug_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_SET_BUILD_PERCENT\r\n");
#endif
#ifdef Debug_Data_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_SET_BUILD_PERCENT\r\n");
#endif
				}
			} else if (command == HOST_CMD_QUEUE_SONG ) //queue a song for playing
			{
				/// Error tone is 0,
				/// End tone is 1,
				/// all other tones user-defined (defaults to end-tone)
				if (cmdBuf.length >= 2)
				{
					uint8_t songId;
					circleBufPop(&cmdBuf); // remove the command code
					songId = pop8();
//					if(songId == 0)
//						Piezo::errorTone(4);
//					else if (songId == 1 )
//						Piezo::doneTone();
//					else
//						Piezo::errorTone(2);
#ifdef Debug_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_QUEUE_SONG \r\n");
#endif
#ifdef Debug_Data_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_QUEUE_SONG \r\n");
#endif
				}

			} else if ( command == HOST_CMD_RESET_TO_FACTORY) 
			{
				/// reset EEPROM settings to the factory value. Reboot bot.
				if (cmdBuf.length >= 1)
				{
					uint8_t options ;
					circleBufPop(&cmdBuf); // remove the command code
					options = pop8();
					//eeprom::factoryResetEEPROM();
					//Motherboard::getBoard().reset(false);
#ifdef Debug_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_RESET_TO_FACTORY\r\n");
#endif
#ifdef Debug_Data_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_RESET_TO_FACTORY\r\n");
#endif
				}
			} else if ( command == HOST_CMD_BUILD_START_NOTIFICATION) 
			{
				if (cmdBuf.length >= 1)
				{
					int buildSteps ;
					circleBufPop(&cmdBuf); // remove the command code
					buildSteps = pop32();
					hostHandleBuildStartNotification(&cmdBuf);
#ifdef Debug_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_BUILD_START_NOTIFICATION\r\n");
#endif
#ifdef Debug_Data_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_BUILD_START_NOTIFICATION\r\n");
#endif
				}
			} else if ( command == HOST_CMD_BUILD_END_NOTIFICATION) 
			{
				if (cmdBuf.length >= 1)
				{
					uint8_t flags ;
					circleBufPop(&cmdBuf); // remove the command code
					flags = circleBufPop(&cmdBuf);
					hostHandleBuildStopNotification(flags);
#ifdef Debug_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_BUILD_END_NOTIFICATION\r\n");
#endif
#ifdef Debug_Data_Cmd
					LED_BLINK(1);
					printf("HOST_CMD_BUILD_END_NOTIFICATION\r\n");
#endif
				}

			} else 
			{
				//错误指令，或者指令 已经乱了，或未知指令，直接pop？
				printf("error instruct; %d\r\n",command);
				circleBufPop(&cmdBuf); // remove the command code
					LED_BLINK(1);
			}
		}
		else
			cmdPaused=true;
		//printf("current cmd count:  %d\r\n",cmdCnt);
		//printf("%d %d %d %d\r\n",LastCntIntr2OnceStepTmp[0],LastCntIntr2OnceStepTmp[1],LastCntIntr2OnceStepTmp[2],LastCntIntr2OnceStepTmp[3]);
	}

}

void commandReset(void)/*{{{*/
{
	circleBufInit(&cmdBuf);
	cmdMode=READY;
	cmdCnt=0;
	sdByteCnt=0;
//	sdFailCnt=0;
}/*}}}*/
bool commandIsReady(void);
bool commandIsEmpty(void);
void commandPush(void);



