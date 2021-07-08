/*
 * debug.c
 *
 *  Created on: 30 Apr 2021
 *      Author: visa
 */

#include "debug.h"
#include "ecumain.h"
#include "inverter.h"
#include "input.h"
#include "configuration.h"
#include "freertosstats.h"

#include "usart.h"
#include "power.h"
#include "powernode.h"
#include "uartecu.h"
#include "taskpriorities.h"
#include "preoperation.h"
#include "adcecu.h"
#include "timerecu.h"

// freeRTOS
#include "semphr.h"

#include <stdio.h>
#include <stdarg.h>

typedef struct Debug_msg {
	char str[MAXDEBUGOUTPUT];
	//uint32_t msgval;
} Debug_msg;


#define DEBUGUART    UART2

#define DEBUGSTACK_SIZE 128*8
#define DEBUGTASKNAME  "DebugTask"
StaticTask_t xDEBUGTaskBuffer;
RAM_D1 StackType_t xDEBUGStack[ DEBUGSTACK_SIZE ];

TaskHandle_t DebugTaskHandle = NULL;

#define DebugQUEUE_LENGTH    30
#define DebugITEMSIZE		sizeof( Debug_msg )

static StaticQueue_t DebugStaticQueue;
uint8_t DebugQueueStorageArea[ DebugQUEUE_LENGTH * DebugITEMSIZE ];

QueueHandle_t DebugQueue;


#define DEBUGPROMPT    "DebugCmd: "


int UARTprintf(const char * format, ... )
{
	va_list ap;
	uint8_t buffer [128];
	int n;
	va_start(ap, format);
	n = vsnprintf ((char*)buffer, 128, format, ap);
	va_end(ap);
	if ( !UART_Transmit(DEBUGUART, (uint8_t*)buffer, n) )
	{
		return 0;
	}

	return UART_WaitTXDone( DEBUGUART, 100);
}


int UARTwritech( const char ch)
{
	if(!UART_Transmit(DEBUGUART, (uint8_t *)&ch, 1)) {
		return 0;
	}

	return UART_WaitTXDone( DEBUGUART, 100);
}



void UARTwrite( const char *str)
{
	UARTprintf(str);
}

void UARTwriteRaw( const char *str)
{
	if ( !UART_Transmit(DEBUGUART, (uint8_t*)str, strlen(str)) )
	{
		return;
	}

	UART_WaitTXDone( DEBUGUART, 100);
}


void UARTwritetwoline(const char *str, const char *str2)
{
	UARTprintf(str);
	UARTprintf(str2);
	UARTprintf("\r\n");
}


// Add message to uart message queue. Might be called from ISR so add a check.
bool DebugMsg( const char * msg)
{
	struct Debug_msg debugmsg;
	strncpy( debugmsg.str, msg, MAXDEBUGOUTPUT );
	if ( xPortIsInsideInterrupt() )
		return xQueueSendFromISR( DebugQueue, &debugmsg, 0 );
	else
		return xQueueSendToBack( DebugQueue, &debugmsg, 0); // send it to error state handler queue for display to user.
}

// print a message to debug output using printf format.
bool DebugPrintf( const char * format, ... )
{
	va_list ap;

	struct Debug_msg debugmsg;
	va_start(ap, format);
	vsnprintf ((char*)debugmsg.str, MAXDEBUGOUTPUT, format, ap);
	va_end(ap);

	if ( xPortIsInsideInterrupt() )
		return xQueueSendFromISR( DebugQueue, &debugmsg, 0 );
	else
		return xQueueSendToBack( DebugQueue, &debugmsg, 0); // send it to error state handler queue for display to user.
}

bool redraw;

// also handle printing debug messages here.
uint8_t uartWait( char *ch )
{
	if(!UART_Receive(DEBUGUART, (uint8_t *)ch, 1)) {
		return 0;
	}

	struct Debug_msg msg;

	redraw = false;

	while ( 1 )
	{
		if ( UART_WaitRXDone( DEBUGUART, 0 ) )
		{
			return 1;
		}

		if ( xQueueReceive(DebugQueue,&msg,10) )
		{
			UARTwrite("\r\n");
			UARTwrite(msg.str);
			// TODO could redraw prompt here instead so it always displays correctly.

			redraw = true;
		}
	}
}

#ifdef configSUPPORT_DYNAMIC_ALLOCATION

#else

#endif

static char str[40*50] = { 0 };

bool streql( const char * str1, const char * str2 )
{
	return ! ( strcmp(str1, str2) );
}

bool checkOn( const char * tkn )
{
	if ( streql(tkn, "closed") || streql(tkn, "on") || streql(tkn, "true") || streql(tkn, "enable")  || streql(tkn, "enabled") )
		return true;
	else
		return false;
}

bool checkOff( const char * tkn )
{
	if ( streql(tkn, "open") || streql(tkn, "off") || streql(tkn, "false") || streql(tkn, "disable") || streql(tkn, "disabled") )
		return true;
	else
		return false;
}


static void debugFanPWM( const int tokens, const int val1, const int val2 )
{
	if ( tokens != 3 )
	{
		UARTwrite("Please give left and right pwm duty.\r\n");
	} else
	{
		if ( val1 > 255 || val2 > 255 || val1 < 0 || val2 < 0 )
		{
			UARTwrite("invalid PWM duty cycles given");
		} else
		{
			snprintf(str, 80, "Requesting fan PWMs Left: %4d Right: %4d\r\n", val1, val2);
			UARTwrite(str);

			FanPWMControl( val1, val2 );
		}
	}
}

static void debugInverter( const char *tkn2, const char *tkn3, const int val2 )
{
	if ( streql(tkn2, "state") || streql(tkn2, "status") )
	{				  // PreOperation  PreOperation


		UARTprintf("Inverter handling enabled: %s\r\n", getEEPROMBlock(0)->InvEnabled? "Y":"N" );
		UARTwrite("-----------------------------------\r\n");
		UARTwrite("Inv  Current State  Requested State\r\n");
		UARTwrite("-----------------------------------\r\n");


		for ( int i=0;i<MOTORCOUNT;i++)
		{
			InverterState_t * invs = getInvState(i);
			char str[MAXDEBUGOUTPUT];
			snprintf(str, MAXDEBUGOUTPUT, "%4d %14s %14s HV:%s\r\n", i,
					getDeviceStatusStr(invs->InvState ),
					getDeviceStatusStr(invs->InvRequested ),
					invs->HighVoltageAvailable ? "Y" : "N"
			);
			UARTwrite(str);
		}

	}
	else if ( checkOn(tkn2) )
	{
		UARTwrite("Enabling inverters and saving config.\r\n");
		getEEPROMBlock(0)->MaxTorque = 5;
		getEEPROMBlock(0)->InvEnabled = true;
		if ( writeEEPROMCurConf() ) // enqueue write the data to eeprom.
		{
			vTaskDelay(20);
			while ( EEPROMBusy() )
			{
				vTaskDelay(20);
			}
			UARTwrite("Saved, power cycle to activate.\r\n");
		} else
			UARTwrite("Error saving config.\r\n");
	}
	else if ( checkOff(tkn2) )
	{
		UARTwrite("Disabling inverters and saving config.\r\n");
		getEEPROMBlock(0)->InvEnabled = false;
		if ( writeEEPROMCurConf() ) // enqueue write the data to eeprom.
		{
			vTaskDelay(100);
			while ( EEPROMBusy() )
			{
				vTaskDelay(100);
			}
			UARTwrite("Saved, power cycle to activate.\r\n");
		} else
			UARTwrite("Error saving config.\r\n");
	}
	else if ( streql(tkn2, "startup") )
	{
		UARTwrite("Sending inverter startup\r\n");
		for ( int i=0;i<MOTORCOUNT;i++)
		{
			InvStartupCfg( getInvState(i) );
		}
	}
	else if ( streql(tkn2, "reset") ) // should try and reset errors.
	{
		UARTprintf("Sending inverter reset to %d\r\n", val2);
		if ( val2 < 0 || val2 > MOTORCOUNT-1)
		{
			UARTwrite("Invalid inverter given.\r\n");
		}
	//	else
		InvReset(getInvState(val2));
	} else
	{
		UARTprintf("unknown inverter cmd\r\n");
	}
}

static void debugMotor( const char *tkn2, const char *tkn3, const int32_t motor, const int32_t value1  )
{

	static int32_t speed[4] = {0};
	static int32_t torque[4] = {0};
	if ( checkOn( tkn2 )  )
	{
		UARTwrite("Setting front power enabled.\r\n");

		setDevicePower( Front1, true );
		setDevicePower( Front2, true );
		vTaskDelay(100);

		if ( !getNodeDevicePower(Front1) )
		{
			UARTwrite("Front1 not powered.\r\n");
			return;
		}

		if ( !getNodeDevicePower(Front2) )
		{
			UARTwrite("Front2 not powered.\r\n");
			return;
		}

#if 0
		uint32_t AnalogueNodesOnline = getAnalogueNodesOnline() ;
		// anode 1
		if ( AnalogueNodesOnline & ( 0x1 << ANode1Bit ) )
		{
			sprintf(str, "Anode1: TorqueR: %lu Regen: %d AppsL: %lu\r\n", ADCState.APPSL, ADCState.Regen);
			UARTwrite(str);
		}
#endif

		// check if analogue nodes online

//		ANode1Bit

		UARTwrite("Setting inverters operational.\r\n");

		uint16_t curreq = PedalTorqueRequest();
		if ( curreq == 0 )
		{
			vTaskDelay(100);
			invRequestState( OPERATIONAL );
			UARTwrite("1.\r\n");
			vTaskDelay(100);
			InverterAllowTorqueAll(true);
			UARTwrite("2.\r\n");
			vTaskDelay(100);
			//setTestMotors(true);

			UARTwrite("3.\r\n");
			vTaskDelay(100);
			UARTwrite("Setting motors enabled till next input.\r\n");

			bool quit = false;

			char ch = 0;

			if(!UART_Receive(DEBUGUART, (uint8_t *)&ch, 1)) {
				UARTwrite("UART error\r\n");
				return;
			}

			struct Debug_msg msg;

			uint16_t oldrequest = 0xffff;
			int16_t speed = 500;
			uint32_t lasttime = 0;

			while ( !quit )
			{
				if ( UART_WaitRXDone( DEBUGUART, 0 ) )
				{
					UARTwrite("Done.\r\n");
					// reset request to 0 here.
					return;
				}

				// print other pending messages.
				if ( xQueueReceive(DebugQueue,&msg,10) )
				{
					UARTwrite("\r\n");
					UARTwrite(msg.str);
				}

				uint16_t requestRaw = ADCState.APPSR;//PedalTorqueRequest();
				if ( abs(oldrequest-requestRaw) > 50 || gettimer() - lasttime > 1000) // only update if value changes.
				{
					oldrequest = requestRaw;
					lasttime = gettimer();
					uint32_t percR = 0;


					if ( requestRaw > 300 )
						percR = ( (100000/( 2100-300 )) * requestRaw-300) / 100;

					if ( percR > 1000 )
						percR = 1000;

					int32_t maxNm = 5;

					int32_t requestNm = ((percR*maxNm)*0x4000)/1000;

					if ( requestNm > 0 )
						InverterSetTorqueInd( 0, requestNm, 500);
					else
						InverterSetTorqueInd( 0, 0, 0);

					UARTprintf("Pedal pos: r%d%%, requestNm %d ( raw %d ) speed %d, maxNm %d, MC0: %s MC1: %s MC2: %s MC3: %s\r\n ",
							percR/10, requestNm/0x4000, requestNm, speed, maxNm, getDeviceStatusStr(getInvState(0)->InvState ),
							getDeviceStatusStr(getInvState(1)->InvState),getDeviceStatusStr(getInvState(2)->InvState ),
							getDeviceStatusStr(getInvState(3)->InvState )
							);
				}
			}
		} else
		{
			UARTprintf("APPS request not 0, not enabling test [curreq %dnm, ped pos l%d r%d , brakes r%d f%d].\r\n", curreq, ADCState.Torque_Req_L_Percent, ADCState.Torque_Req_R_Percent, ADCState.BrakeR, ADCState.BrakeF);
		}
	}
	else if ( checkOff( tkn2 ) )
	{
		// check if pedal is 0, only allow enabling if no input.
		InverterAllowTorqueAll( false );
		setTestMotors(false);
		invRequestState( BOOTUP );
		UARTwrite("Setting torque disabled.\r\n");
	}
	else if ( streql( tkn2, "speed" )  )
	{
		if ( value1 >= 0 )
			speed[motor] = value1;
		InverterSetTorqueInd( motor, torque[motor], speed[motor]);
		UARTwrite("Setting speed request, keeping previous torque.\r\n");
	}
	else if ( streql( tkn2, "torque" )  )
	{
		if ( value1 >= 0 )
			torque[motor] = value1;
		InverterSetTorqueInd( motor, torque[motor], speed[motor]);
		UARTwrite("Setting torque request, keeping previous speed.\r\n");
	}

	for ( int i=0;i<MOTORCOUNT;i++)
	{
		UARTprintf("Current Params motor:%2d   speed:%5d torque:%5d\r\n", i, speed[i], torque[i]);
	}
}


static void debugShutdown( const char *tkn2, const char *tkn3 )
{
	if ( checkOn(tkn2) )
	{
		UARTwrite("Setting shutdown circuit closed.\r\n");
		ShutdownCircuitSet(true);
	}
	else if ( checkOff(tkn2) )
	{
		UARTwrite("Setting shutdown circuit open.\r\n");
		ShutdownCircuitSet(false);
	}
	else
	{
		UARTwrite("Current state of shutdown switches:\r\n");

		UARTwritetwoline("ECU          ", ShutdownCircuitState()?"Closed":"Open");
		UARTwritetwoline("BOTS         ", Shutdown.BOTS?"Closed":"Open");
		UARTwritetwoline("Inertia      ", Shutdown.InertiaSwitch?"Closed":"Open");
		UARTwritetwoline("BSPD After   ", Shutdown.BSPDAfter?"Closed":"Open");
		UARTwritetwoline("BSPD Before  ", Shutdown.BSPDBefore?"Closed":"Open");
		UARTwritetwoline("Cockpit      ", Shutdown.CockpitButton?"Closed":"Open");
		UARTwritetwoline("Left         ", Shutdown.LeftButton?"Closed":"Open");
		UARTwritetwoline("Right        ", Shutdown.RightButton?"Closed":"Open");
		UARTwritetwoline("BMS          ", Shutdown.BMS?"Closed":"Open"); // BMSReason
		UARTwritetwoline("IMD          ", Shutdown.IMD?"Closed":"Open");
		UARTwritetwoline("AIR          ", Shutdown.AIROpen?"Closed":"Open");
	}
}

static void debugSensors( const char *tkn2 )
{
	{
		UARTwrite("Current state of sensors:\r\n");

#if 0
		uint32_t AnalogueNodesOnline = getAnalogueNodesOnline() ;
		// anode 1
		if ( AnalogueNodesOnline & ( 0x1 << ANode1Bit ) )
		{
			sprintf(str, "Anode1: TorqueR: %lu Regen: %d AppsL: %lu\r\n", ADCState.APPSL, ADCState.Regen);
			UARTwrite(str);
		}

		if ( AnalogueNodesOnline & ( 0x1 << ANode9Bit ) )
		{
			sprintf(str, "Anode9: ");
			UARTwrite(str);
		}

		if ( AnalogueNodesOnline & ( 0x1 << ANode10Bit ) )
		{
			sprintf(str, "Anode10: ");
			UARTwrite(str);
		}

		if ( AnalogueNodesOnline & ( 0x1 << ANode11Bit ) )
		{
			sprintf(str, "Anode11: ");
			UARTwrite(str);
		}

		if ( AnalogueNodesOnline & ( 0x1 << ANode12Bit ) )
		{
			sprintf(str, "Anode12: ");
			UARTwrite(str);
		}

		if ( AnalogueNodesOnline & ( 0x1 << ANode13Bit ) )
		{
			sprintf(str, "Anode13: ");
			UARTwrite(str);
		}

		if ( AnalogueNodesOnline & ( 0x1 << ANode14Bit ) )
		{
			sprintf(str, "Anode14: ");
			UARTwrite(str);
		}

		if ( AnalogueNodesOnline & ( 0x1 << ANode15Bit ) )
		{
			sprintf(str, "Anode15: ");
			UARTwrite(str);
		}

		if ( AnalogueNodesOnline & ( 0x1 << ANode16Bit ) )
		{
			sprintf(str, "Anode16: ");
			UARTwrite(str);
		}

		if ( AnalogueNodesOnline & ( 0x1 << ANode17Bit ) )
		{
			sprintf(str, "Anode17: ");
			UARTwrite(str);
		}
#endif

	}
}


static void debugPower( const char *tkn2, const char *tkn3 )
{
	DevicePower device = None;
	bool state = false;
	bool badcmd = false;

	if ( strlen(tkn2) == 0 ) // we need some sub commands, otherwise show help
	{
		UARTwrite("Power command: Help\r\n");
	}
	else if ( streql(tkn2, "status") || streql(tkn2, "state") )
	{
		UARTwrite("------------------------\r\n");
		UARTwrite("Power        Exp Act Err\r\n");
		UARTwrite("------------------------\r\n");

		uint8_t listsize = getDevicePowerListSize();

		for ( int i=1;i<=listsize;i++)
		{
			snprintf(str, 80, "%-12s %-4s%-4s%-4lu\r\n",
						getDevicePowerNameLong(i),
						getNodeDeviceExpectedPower(i)?"On":"Off",
						getNodeDevicePower(i)?"On":"Off",
						powerErrorOccurred(i)
					//    powerErrorOccurred(i)?"Yes":"No"
			);
			UARTwrite(str);
		}
	}
	else if ( streql(tkn2, "all") )
	{
		if ( streql(tkn3, "reset") )
		{
			UARTwrite("Power error reset for all\r\n");
			for ( int i=1; i <= AccuFan; i++ )
				resetDevicePower( i );
		}
		else
		{
			if ( checkOn( tkn3 ) )
			{
				state = true;
			}
			else if ( checkOff( tkn3 ) )
			{
				state = false;
			}
			else
			{
				badcmd = true;
			}

			if ( !badcmd )
			{
				UARTwrite("Manual power request for all power set ");
				UARTwrite(state? "on":"off");
				UARTwrite("\r\n");

				for ( int i=1; i <= AccuFan; i++ )
					setDevicePower( i, state );

			}
			else
			{
				badcmd = true;
			}
		}
	}
	else
	{
		if ( streql(tkn2, "none") )
			device = None;
		else if ( streql(tkn2, "buzzer") )
			device = Buzzer;
		else if ( streql(tkn2, "back1") )
			device = Back1;
		else if ( streql(tkn2, "telemetry") )
			device = Telemetry;
		else if ( streql(tkn2, "front1") )
			device = Front1;
		else if ( streql(tkn2, "inverters") )
			device = Inverters;
		else if ( streql(tkn2, "ecu") )
			device = ECU;
		else if ( streql(tkn2, "front2") )
			device = Front2;
		else if ( streql(tkn2, "leftfans") )
			device = LeftFans;
		else if ( streql(tkn2, "rightfans") )
			device = RightFans;
		else if ( streql(tkn2, "leftpump") )
			device = LeftPump;
		else if ( streql(tkn2, "rightpump") )
			device = RightPump;
		else if ( streql(tkn2, "ivt") )
			device = IVT;
		else if ( streql(tkn2, "current") )
			device = Current;
		else if ( streql(tkn2, "tsal") )
			device = TSAL;
		else if ( streql(tkn2, "brake") )
			device = Brake;
		else if ( streql(tkn2, "accu") )
			device = Accu;
		else if ( streql(tkn2, "accufan") )
			device = AccuFan;

		if ( streql(tkn3, "reset") )
		{
			UARTwrite("Power error reset for ");
			UARTwrite(getDevicePowerNameLong(device));
			UARTwrite("\r\n");
			resetDevicePower(device);
		} else
		{
			if ( checkOn( tkn3 ) )
			{
				state = true;
			}
			else if ( checkOff( tkn3 ) )
			{
				state = false;
			}
			else
			{
				device = None;
			}

			if ( device != None )
			{
				UARTwrite("Manual power request for ");
				UARTwrite(getDevicePowerNameLong(device));
				UARTwrite(" set ");
				UARTwrite(state? "on":"off");
				UARTwrite("\r\n");
				setDevicePower( device, state );
			}
			else
			{
				badcmd = true;
			}
		}
	}


	if ( badcmd )
	{
		UARTwrite("Invalid power request given: Help\r\n");
	}
}

//run a small state machine on incoming uart charecters to seperate escape co
uint16_t processUARTchar( const uint8_t ch, uint8_t * state )
{

	// didn't receive a char, skip.
	if ( ch == 0 )
	{
		return ch; // nothing to do this loop, return untouched.
	}

	// 27/91/67 = left
	// 27/91/68 = right
	// ch 8 == backspace.
	switch ( *state )
	{
	case 0 :
		if ( ch == 27 ) // escape code
		{
			*state = 1;
			return 0;
		}

		break;

	case 1 :
		if ( ch == 91) // [
		{
			*state = 2;
			return 0;
		}  else
		{
			*state = 0;
			break;
		}

	case 2 :
		switch ( ch<<8 )
		{
		case KEY_LEFT :
		case KEY_RIGHT :
		case KEY_UP :
		case KEY_DOWN :
			*state = 0;
			uint16_t retch = ch << 8;
			return retch;
		}
		state = 0;
		return 0;
		break;
	}

	return ch;

}

void debugConfig( void )
{
	bool quit = false;

	uint8_t state = 0;
	uint16_t ch = 0;

	UARTwrite("Running config menu.\r\n");

	while ( !quit )
	{
		// just to be on safe side then.
		volatile uint16_t read = uartWait((char*)&ch);

		read = processUARTchar( (uint8_t) ch, &state );

		if ( read == 0 )
			continue;

		if ( read == 'q' )
			quit = true;
		else if ( read == KEY_LEFT )
			UARTprintf("Left\r\n");
		else if ( read == KEY_RIGHT )
			UARTprintf("Right\r\n");
		else if ( read == KEY_UP )
			UARTprintf("Up\r\n");
		else if ( read == KEY_DOWN )
			UARTprintf("Down\r\n");
		else if ( read == KEY_ENTER )
		{
			UARTprintf("Enter\r\n");
		}
		else if ( read == 'c' )
		{
			ConfigInput( 0xFFFF );
			read = 0;
		}
		else
			UARTwritech(ch);

		ConfigInput( read );

	}

	UARTwrite("Done.\r\n");
}

void debugESCCodeInput( void )
{
	bool quit = false;

	uint8_t state = 0;
	uint16_t ch = 0;

	UARTwrite("Reading keys.\r\n");

	while ( !quit ) {
		// just to be on safe side then.
		volatile uint16_t read = uartWait((char*)&ch);

		read = processUARTchar( (uint8_t) ch, &state );

		if ( read == 0 )
			continue;

		if ( read == 'q' )
			quit = true;
		else if ( read == KEY_LEFT )
			UARTprintf("Left\r\n");
		else if ( read == KEY_RIGHT )
			UARTprintf("Right\r\n");
		else if ( read == KEY_UP )
			UARTprintf("Up\r\n");
		else if ( read == KEY_DOWN )
			UARTprintf("Down\r\n");
		else
			UARTwritech(ch);

	}

	UARTwrite("Done.\r\n");
}


static void DebugTask(void *pvParameters)
{
	uint8_t charcount = 0;

	UARTwrite("\r\nBooting ECU b10033...\r\n\r\n");

	redraw = false;

	UARTwrite(DEBUGPROMPT);

	char ch = 0;

	uint8_t escstate = false;

	while (1) {
		// just to be on safe side then.
		volatile uint16_t read = uartWait(&ch);

		if ( redraw )
		{
			// we received debug output during keyboard wait, resend prompt and current imput to help.
			UARTwrite("\r\n");
			// uartwrite("\x1b[k");
			UARTwrite(DEBUGPROMPT);
			UARTwrite(str);
		}

		// didn't receive a char, skip.
		if ( read == 0 )
		{
			continue; // nothing to do this loop, return to start.
		}

		read = processUARTchar( ch, &escstate );

		bool endline = false;

		// 27 = esc -- second keycode coming.
		// 27/91/67 = left
		// 91/68 = right
		// ch 8 == backspace.

		if ( read == 0 )
		{
			continue; // nothing to do this loop, return to start.
		}

		if ( read == 8 )
		{
			if ( charcount > 0 )
			{
				str[charcount] = 0;
				--charcount;
				UARTwritech(read);
				UARTwritech(' ');
				UARTwritech(read);
			}
		} else
		if ( !( read == '\n' || read == '\r') )
		{
			if ( read >= 32) // only process printable charecters.
			{
				str[charcount] = read;
				str[charcount+1] = 0;
				UARTwritech(read);
				++charcount;
			}
		} else
		{
			endline = true;
			UARTwrite("\r\n");
		}

#define TOKENLENGTH   12


		if ( charcount == 60 || endline )
		{
			if ( charcount > 0 )
			{
				char tkn1[TOKENLENGTH] = "";
				char tkn2[TOKENLENGTH] = "";
				char tkn3[TOKENLENGTH] = "";
				char tkn4[TOKENLENGTH] = "";
				char tkn5[TOKENLENGTH] = "";

				uint8_t tokens = 0;

				int val1;
				int val2;
				int val3;
				int val4;

				// parse the input string into tokens to be processed.

				char *s=str;

				if (*(s += strspn(s, " ")) != '\0') { // find the first non space, and move string pointer to it. Check if we have reached end of string.
					size_t tknlen = strcspn(s, " ");  // if not at end of string, find the next space, getting the span of token.
					if ( tknlen < TOKENLENGTH )
					{
						strncpy(tkn1, s, tknlen);
						tkn1[tknlen] = '\0';

						s += tknlen;
						tokens++;
					} else
					{
						strncpy(tkn1, "too long", tknlen);
					}
				}

				if (*(s += strspn(s, " ")) != '\0') {
					size_t tknlen = strcspn(s, " ");
					if ( tknlen < TOKENLENGTH )
					{
						strncpy(tkn2, s, tknlen);
						tkn2[tknlen] = '\0';

						s += tknlen;
						tokens++;
					}
				}

				if ( strlen(tkn2) >0 )
				{
					val1 = strtol(tkn2, NULL, 10);
				} else val1 = 0;


				if (*(s += strspn(s, " ")) != '\0') {
					size_t tknlen = strcspn(s, " ");
					if ( tknlen < TOKENLENGTH )
					{
						strncpy(tkn3, s, tknlen);
						tkn3[tknlen] = '\0';
						s += tknlen;
						tokens++;
					}
				}

				if ( strlen(tkn3)>0 )
				{
					val2 = strtol(tkn3, NULL, 10);
				} else val2 = 0;

				if (*(s += strspn(s, " ")) != '\0') {
					size_t tknlen = strcspn(s, " ");
					if ( tknlen < TOKENLENGTH )
					{
						strncpy(tkn4, s, tknlen);
						tkn4[tknlen] = '\0';
						s += tknlen;
						tokens++;
					}
				}

				if ( strlen(tkn4)>0 )
				{
					val3 = strtol(tkn4, NULL, 10);
				} else val3 = 0;


				if (*(s += strspn(s, " ")) != '\0') {
					size_t tknlen = strcspn(s, " ");
					if ( tknlen < TOKENLENGTH )
					{
						strncpy(tkn5, s, tknlen);
						tkn5[tknlen] = '\0';
						tokens++;
					}
				}

				if ( strlen(tkn5)>0 )
				{
					val4 = strtol(tkn5, NULL, 10);
				} else val4 = 0;



				if ( streql(tkn1, "esckey" ) )
				{
					debugESCCodeInput();
				} else

				if ( streql(tkn1, "config" ) )
				{
					debugConfig();
				} else
				if ( streql(tkn1, "inverter" ) )
				{
					debugInverter( tkn2, tkn3, val2 );
				}
				else if ( streql(tkn1, "motor") )
				{
					debugMotor(tkn2, tkn3, val2, val3);
				}
				else if ( streql(tkn1, "shutdown") )
				{
					debugShutdown(tkn2, tkn3);
				}
				else if ( streql(tkn1, "fanpwm") )
				{
					debugFanPWM(tokens, val1, val2 );
				}
				else if ( streql(tkn1, "power") )
				{
					debugPower(tkn2, tkn3);
				}
				else if ( streql(tkn1, "sensors") )
				{
					debugSensors(tkn2);
				}
				else if ( streql(tkn1, "uarttest") )
				{
					UART_Transmit(UART1, (uint8_t *)"This is a test.", 15);
				} else if ( streql( str, "stats") )
				{
					// print stats.
					UARTwrite("\r\nRuntime statistics output:\r\n");
					UARTwrite("Taskname        Runtime         Percentage\r\n");
					vTaskGetRunTimeStatsNoDyn( str );

					UARTwriteRaw(str);

					UARTwrite("\r\n");

				} else

				if ( streql( str, "list") )
				{
					// print list.
					UARTwrite("\r\nTask List\r\n");
					UARTwrite("Name            Stat    Prio    StackF  TaskNo\r\n");
					vTaskListNoDyn( str );
					UARTwriteRaw(str);
					UARTwrite("\r\n");
				} else

				if ( streql(tkn1, "help" ) )
				{
					UARTwrite("\r\ECU Debug Help.\r\n\r\n");

					UARTwrite("List of available commands:\r\n");

					UARTwrite("power state|device on|off");
					UARTwrite("fanpwm <leftduty> <rightduty>");
					UARTwrite("shutdown state|on|off");
					UARTwrite("motor");
					UARTwrite("inverter reset");
					UARTwrite("sensors");
					UARTwrite("config");
					UARTwrite("esckey");
					UARTwrite("list");
					UARTwrite("stats");

				} else

				{
					UARTwrite("Unknown command: ");
					UARTwrite(tkn1);
					UARTwrite("\r\n");
				}

			}

			charcount = 0;
			str[0] = 0;
			// print prompt to request further input.
			UARTwrite(DEBUGPROMPT);

		}
	}

	vTaskDelete(NULL);
}


int initDebug( void )
{

	DebugQueue = xQueueCreateStatic( DebugQUEUE_LENGTH,
							  DebugITEMSIZE,
							  DebugQueueStorageArea,
							  &DebugStaticQueue );

	vQueueAddToRegistry(DebugQueue, "DebugQueue" );

	DebugTaskHandle = xTaskCreateStatic(
						  DebugTask,
						  DEBUGTASKNAME,
						  DEBUGSTACK_SIZE,
						  ( void * ) 1,
						  DEBUGTASKPRIORITY,
						  xDEBUGStack,
						  &xDEBUGTaskBuffer );

	return 0;
}

