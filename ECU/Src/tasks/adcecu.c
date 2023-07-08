/*
 * adcecu.c
 *
 *      Author: Visa
 */

#include "ecumain.h"

#include "adc.h"
#include "eeprom.h"
#include "errors.h"
#include "input.h"
#include "adcecu.h"
#include "i2c-lcd.h"
#include "semphr.h"
#include "analognode.h"
#include <limits.h>
#include "taskpriorities.h"
#include "timerecu.h"
#include "debug.h"
#include "power.h"
#include "brake.h"

#define UINTOFFSET	360

#ifdef STMADC
volatile uint32_t ADCloops;
volatile uint32_t ADC3loops;

volatile bool ADC1read = false;
volatile bool ADC3read = false;

//variables that need to be accessible in ISR's

//setup to place ADC buffers in compatible RAM region, ensure linker file is setup correctly for D1

#if defined( __ICCARM__ )
  #define DMA_BUFFER \
      _Pragma("location=\".dma_buffer\"")
#else
  #define DMA_BUFFER \
      __attribute__((section(".dma_buffer")))
#endif

// ADC conversion buffer, should be aligned in memory for faster DMA?
DMA_BUFFER ALIGN_32BYTES (static uint32_t aADCxConvertedData[ADC_CONVERTED_DATA_BUFFER_SIZE]);
DMA_BUFFER ALIGN_32BYTES (static uint32_t aADCxConvertedDataADC3[ADC_CONVERTED_DATA_BUFFER_SIZE_ADC3]);
#endif

#ifdef HPF19
#include "adc_hpf19.h"
#endif

#ifdef HPF20
#include "adc_hpf20.h"
#endif

volatile char minmaxADC;

volatile ADCState_t ADCState;
volatile ADCState_t ADCStateNew;

volatile ADCStateSensors_t ADCStateSensors;

volatile ADCInterpolationTables_t ADCInterpolationTables;

void ReadADC1(bool half);
void ReadADC3(bool half);
int getSteeringAnglePWM( void );

TaskHandle_t ADCTaskHandle = NULL;

#define ADCSTACK_SIZE 128*2
#define ADCTASKNAME  "ADCTask"
StaticTask_t xADCTaskBuffer;
StackType_t xADCStack[ ADCSTACK_SIZE ];


#ifdef STMADC
//quick and dirty hack till rework adc back to one shot dma

SemaphoreHandle_t xADC1 = NULL;
SemaphoreHandle_t xADC3 = NULL;

#endif

static SemaphoreHandle_t waitStr = NULL;
SemaphoreHandle_t ADCUpdate = NULL;

char ADCWaitStr[20] = "";

char * getADCWait( void)
{
	static char ADCWaitStrRet[20] = "";
	xSemaphoreTake(waitStr, portMAX_DELAY);
	strcpy(ADCWaitStrRet, ADCWaitStr);
	xSemaphoreGive(waitStr);
	return ADCWaitStrRet;
}

uint32_t curanaloguenodesOnline = 0;
uint32_t analoguenodesOnline = 0;
uint32_t lastanaloguenodesOnline = 0;

uint32_t getAnalogueNodesOnline( void )
{
	return curanaloguenodesOnline;
}

// Task shall monitor analogue node timeouts, and read data from local ADC/PWM to get all analogue values ready for main loop.
void ADCTask(void *argument)
{
	/* pxQueueBuffer was not NULL so xQueue should not be NULL. */

	 xEventGroupSync( xStartupSync, 0, 1, portMAX_DELAY );

#ifdef STMADC
	xADC1 = xSemaphoreCreateBinary();
	xADC3 = xSemaphoreCreateBinary();
#endif

	DeviceState.ADC = OPERATIONAL;

	DeviceState.Sensors = OFFLINE; // not seen anything yet, so assume offline.

	uint32_t count = 0;

	uint32_t lastseenall = 0;

	uint32_t analoguenodesOnlineSince = 0;

	ADCWaitStr[0] = 0;

	while( 1 )
	{
		lastanaloguenodesOnline = analoguenodesOnline;
		// read the values from last cycle.
		xTaskNotifyWait( pdFALSE, ULONG_MAX, &analoguenodesOnline, 0 );

		// block and wait for main cycle.
		xEventGroupSync( xCycleSync, 0, 1, portMAX_DELAY );

		// copy received critical sensor data from last cycle to
		xSemaphoreTake(ADCUpdate, portMAX_DELAY);
		memcpy(&ADCState, &ADCStateNew, sizeof(ADCState));
		ADCState.Oldest = getOldestANodeCriticalData();
		xSemaphoreGive(ADCUpdate);

		uint32_t curtime = gettimer();

		if ( DeviceState.CriticalSensors == OPERATIONAL )
		{
			if ( curtime - CYCLETIME*2 - 1  > ADCState.Oldest ) // TODO investigate, sometimes getting upto 39 ms
			{
				DebugPrintf("Oldest ANode data %d old at (%lu)", curtime-ADCState.Oldest, curtime);
			}
		}

		count++;

		if ( lastanaloguenodesOnline != analoguenodesOnline )
		{
			char str[40];
			snprintf(str, 40, "Analogue diff %lu", count);
//			DebugMsg(str);
		}

#ifdef STMADC
		// start the adc reads and then wait on their incoming queues.
		if ( HAL_ADCEx_MultiModeStart_DMA(&hadc1, (uint32_t *)aADCxConvertedData, ADC_CONVERTED_DATA_BUFFER_SIZE)  != HAL_OK)
		{
			DeviceState.ADC = INERROR;
		}

		HAL_StatusTypeDef adc3status = HAL_ADC_Start_DMA(&hadc3,(uint32_t *)aADCxConvertedDataADC3,ADC_CONVERTED_DATA_BUFFER_SIZE_ADC3);

		// start ADC conversion
		//  return HAL_ADC_Start_DMA(&hadc1,(uint32_t *)aADCxConvertedData,ADC_CONVERTED_DATA_BUFFER_SIZE);
		if ( adc3status != HAL_OK)
		{
			DeviceState.ADC = INERROR;
		}

	//	if( == pdTRUE )

		if ( DeviceState.ADC != INERROR )
		{
			xSemaphoreTake( xADC1, portMAX_DELAY );
			xSemaphoreTake( xADC3, portMAX_DELAY );
			ReadADC1(false);
			ReadADC3(false);
		}
#endif

	    getSteeringAnglePWM();

	    xSemaphoreTake(waitStr, portMAX_DELAY);

		analoguenodesOnlineSince |= analoguenodesOnline; // cumulatively add

		if ( ( curanaloguenodesOnline & AnodeCriticalBit ) == AnodeCriticalBit )
		{
			DeviceState.CriticalSensors = OPERATIONAL;
			// we've received all the SCS data
		} else
		{
			DeviceState.CriticalSensors = INERROR;
		}

		if ( ( analoguenodesOnlineSince & ANodeAllBit ) == ANodeAllBit ) // all expected nodes reported in.
		{
			if ( DeviceState.Sensors != OPERATIONAL )
			{
				DebugMsg("Analogue nodes all online");
			}
			DeviceState.Sensors = OPERATIONAL;
			setAnalogNodesStr( analoguenodesOnlineSince );
			ADCWaitStr[0] = 0;
			analoguenodesOnlineSince = 0;
			curanaloguenodesOnline = analoguenodesOnline;
			lastseenall = gettimer();
		} else if ( gettimer() - lastseenall > NODETIMEOUT ) // only update status to rest of code every timeout val.
		{
			lastseenall = gettimer();
			setAnalogNodesStr( analoguenodesOnlineSince );
			strcpy(ADCWaitStr, getAnalogNodesStr());

			// update the currently available nodes
			curanaloguenodesOnline = analoguenodesOnlineSince;

			if ( analoguenodesOnlineSince == 0 )
			{
				if ( DeviceState.Sensors != OFFLINE )
				{
					DebugMsg("Analogue node timeout");
				}
				DeviceState.Sensors = OFFLINE; // can't see any nodes, so offline.
			}
			else
			{
				if ( DeviceState.Sensors != INERROR )
				{
					DebugPrintf("Analogue nodes partially online (%lu)", curanaloguenodesOnline);
				}
				DeviceState.Sensors = INERROR; // haven't seen all needed, so in error.
			}

			analoguenodesOnlineSince = 0;
		}

#ifndef NOBRAKELIGHTCONTROL
		if ( getBrakeLight() )
		{
			setDevicePower( Brake,  true);
		} else
		{
			setDevicePower( Brake,  false);
		};
#endif

		xSemaphoreGive(waitStr);

		DeviceState.ADCSanity = CheckADCSanity();
	}

	vTaskDelete(NULL);
}


bool SetupADCInterpolationTables( eepromdata * data )
{
    // calibrated input range for steering, from left lock to center to right lock.
    // check if this can be simplified?

	if ( checkversion(data->VersionString) )
	{

		int i = 0;

	#ifdef HPF19
		for (;data->ADCSteeringInput[i]!=0;i++)
		{
			SteeringInput[i] = data->ADCSteeringInput[i];
			SteeringOutput[i] = data->ADCSteeringOutput[i];
		}
		SteeringSize = i;
	#endif

		BrakeRInput[2] = data->ADCBrakeRPresInput[0];
		BrakeRInput[3] = data->ADCBrakeRPresInput[1];

		BrakeROutput[2] = data->ADCBrakeRPresOutput[0];
		BrakeROutput[3] = data->ADCBrakeRPresOutput[1];

		BrakeFInput[2] = data->ADCBrakeFPresInput[0];
		BrakeFInput[3] = data->ADCBrakeFPresInput[1];

		BrakeFOutput[2] = data->ADCBrakeFPresOutput[0];
		BrakeFOutput[3] = data->ADCBrakeFPresOutput[1];


		i = 0;

		int TravMin = data->ADCTorqueReqLInput[0];
		int TravMax = data->ADCTorqueReqLInput[1];
		int TravMinOffset = 10;
		int TravMaxOffset = 98;

		if ( data->ADCTorqueReqLInput[3] != 0 )
		{
			TravMinOffset = data->ADCTorqueReqLInput[2];
			TravMaxOffset = data->ADCTorqueReqLInput[3];

		}

		TorqueReqLInput[2] = (TravMax-TravMin)/100*TravMinOffset+TravMin;
		TorqueReqLInput[3] = (TravMax-TravMin)/100*TravMaxOffset+TravMin;

#ifdef TORQUEERRORCHECK
		uint32_t absolutemax = TravMax*1.1;
		if ( absolutemax > 0xFFFF )
			absolutemax + 64000;

		TorqueReqLInput[4] = absolutemax;
		TorqueReqLInput[5] = TorqueReqLInput[4]+1;
#endif

		TravMin = data->ADCTorqueReqRInput[0];
		TravMax = data->ADCTorqueReqRInput[1];
		TravMinOffset = 10;
		TravMaxOffset = 98;

		if ( data->ADCTorqueReqRInput[3] != 0 )
		{
			TravMinOffset = data->ADCTorqueReqRInput[2];
			TravMaxOffset = data->ADCTorqueReqRInput[3];
		}

		TorqueReqRInput[2] = (TravMax-TravMin)/100*TravMinOffset+TravMin;
		TorqueReqRInput[3] = (TravMax-TravMin)/100*TravMaxOffset+TravMin;

#ifdef TORQUEERRORCHECK
		absolutemax = TravMax*1.1;
		if ( absolutemax > 0xFFFF )
			absolutemax + 64000;

		TorqueReqRInput[4] = absolutemax;
		TorqueReqRInput[5] = TorqueReqRInput[4]+1;
#endif

		TravMin = data->ADCBrakeTravelInput[0];
		TravMax = data->ADCBrakeTravelInput[1];
		TravMinOffset = 10;
		TravMaxOffset = 98;

		if ( data->ADCBrakeTravelInput[3] != 0 )
		{
			TravMinOffset = data->ADCBrakeTravelInput[2];
			TravMaxOffset = data->ADCBrakeTravelInput[3];
		}

	// regen
		BrakeTravelInput[2] = (TravMax-TravMin)/100*TravMaxOffset+TravMin;
		BrakeTravelInput[3] = (TravMax-TravMin)/100*TravMaxOffset+TravMin;

#ifdef TORQUEERRORCHECK
		absolutemax = TravMax*1.1;
		if ( absolutemax > 0xFFFF )
			absolutemax + 64000;

		BrakeTravelInput[4] = absolutemax;
		BrakeTravelInput[5] = BrakeTravelInput[4]+1;
#endif

		if (data->pedalcurves[i].PedalCurveInput[1] != 0)
		{
			TorqueCurveCount = 0;

			i = 0;

			for (;data->pedalcurves[i].PedalCurveInput[1]!=0;i++) // first number could be 0, but second will be non zero.
			{
				TorqueCurveCount++;

				int j=0;
				do {
					TorqueInputs[i][j]=data->pedalcurves[i].PedalCurveInput[j];
					TorqueOutputs[i][j]=data->pedalcurves[i].PedalCurveOutput[j];
					j++;

				} while ( data->pedalcurves[i].PedalCurveInput[j] != 0);
	//			if ( j < 3 ) j = 0;
				TorqueCurveSize[i] = j;
			}
		}

	#ifdef HPF19
		i = 0;
		for (;data->CoolantInput[i]!=0;i++)
		{
			CoolantInput[i] = data->CoolantInput[i];
			CoolantOutput[i] = data->CoolantInput[i];
		}
		CoolantSize = i;


		uint16_t DrivingModeInput[] = { 0 , 1022, 1023, 1024,4500, 13500, 23500, 33000, 40000, 49000, 57500, 65534, 65535 };
		int16_t DrivingModeOutput[] = { 1 , 1,    0,     1,   1,    2,    3,      4,     5,     6,    7,      8,     0 };

	#endif


	#ifdef HPF19
		ADCInterpolationTables.Steering.Input = SteeringInput;
		ADCInterpolationTables.Steering.Output = SteeringOutput;

		// replace with exact size.
		ADCInterpolationTables.Steering.Elements = SteeringSize; // calculate elements from memory size of whole array divided by size of on element.
	#endif

		ADCInterpolationTables.BrakeR.Input = BrakeRInput;
		ADCInterpolationTables.BrakeR.Output = BrakeROutput;

		ADCInterpolationTables.BrakeR.Elements = BrakeRSize;

		ADCInterpolationTables.BrakeF.Input = BrakeFInput;
		ADCInterpolationTables.BrakeF.Output = BrakeFOutput;

		ADCInterpolationTables.BrakeF.Elements = BrakeFSize;

		ADCInterpolationTables.Regen.Input = BrakeTravelInput;
		ADCInterpolationTables.Regen.Output = BrakeTravelOutput;

		ADCInterpolationTables.Regen.Elements = BrakeTravelSize;

		ADCInterpolationTables.AccelL.Input = TorqueReqLInput;
		ADCInterpolationTables.AccelL.Output = TorqueReqLOutput;

		ADCInterpolationTables.AccelL.Elements = TorqueReqLSize;

		ADCInterpolationTables.AccelR.Input = TorqueReqRInput;
		ADCInterpolationTables.AccelR.Output = TorqueReqROutput;

		ADCInterpolationTables.AccelR.Elements = TorqueReqRSize;

		ADCInterpolationTables.TorqueCurve.Input = TorqueInputs[0];
		ADCInterpolationTables.TorqueCurve.Output = TorqueOutputs[0];

		ADCInterpolationTables.TorqueCurve.Elements = TorqueCurveSize[0];

		ADCInterpolationTables.Coolant.Input = CoolantInput;
		ADCInterpolationTables.Coolant.Output = CoolantOutput;

		ADCInterpolationTables.Coolant.Elements = CoolantSize;

		ADCInterpolationTables.ModeSelector.Input = DrivingModeInput;
		ADCInterpolationTables.ModeSelector.Output = DrivingModeOutput;

		ADCInterpolationTables.ModeSelector.Elements = DriveModeSize;

    	return true;
	} else return false;
}

void SetupTorque( int request )
{
	ADCInterpolationTables.TorqueCurve.Input = TorqueInputs[request];
	ADCInterpolationTables.TorqueCurve.Output = TorqueOutputs[request];
	ADCInterpolationTables.TorqueCurve.Elements = TorqueCurveSize[request];
}

/**
 * function to perform a linear interpolation using given input/output value arrays and raw data.
 */
int16_t linearInterpolate(uint16_t Input[], int16_t Output[], uint16_t count, uint16_t RawADCInput)
{
    int i;

    if ( Input == NULL )
    {
    	return 0;
    }

    if ( count < 2 )
    {
    	return 0;
    }

    if(RawADCInput < Input[0])
    {  // if input less than first table value return first.
        return Output[0];
    }

    if(RawADCInput > Input[count-1])
    { // if input larger than last table value return last.
        return Output[count-1];
    }

    // loop through input values table till we find space where requested fits.
    for (i = 0; i < count-1; i++)
    {
        if (Input[i+1] > RawADCInput)
        {
            break;
        }
    }

    int dx,dy;

    /* interpolate */
    dx = Input[i+1] - Input[i];
    dy = Output[i+1] - Output[i];
    return Output[i] + ((RawADCInput - Input[i]) * dy / dx);
}


int getSteeringAnglePWM( void )
{
	volatile int angle = 0;

	if ( receivePWM() )
	{
		angle = getPWMDuty();
		ADCState.SteeringDuty = angle;
		ADCState.SteeringFreq = getPWMFreq();
		angle = ( angle*360.0 );
		angle = angle / 10000; // center around 180;
		ADCState.SteeringAngleAct = angle;
		angle += getEEPROMBlock(0)->steerCalib; // account for calibration
		if ( angle < 0 ) angle = angle + 360;
		if ( angle > 359 ) angle = angle - 360;
		angle = (angle - 180);
		ADCState.SteeringAngle = -1 * angle; // flip positive.
		xTaskNotify( ADCTaskHandle, ( 0x1 << SteeringAngleReceivedBit ), eSetBits);
		return true;
	} else
	{
		ADCState.SteeringAngle = 0xFFFF; // not read, return impossible angle for sanity check.
		ADCState.SteeringAngleAct = 0xFFFF;
		return false;
	}
}

/**
 * convert raw front brake reading into calibrated brake position
 */
int getBrakeF(uint16_t RawADCInput)
{
#ifdef HPF19
	if( usecanADC )  // check if we're operating on fake canbus ADC
	{
	  return CANADC.BrakeF;
	}
#endif
#ifdef NOBRAKES
	return 0;
#else
    struct ADCTable ADC = ADCInterpolationTables.BrakeF;
    return linearInterpolate(ADC.Input, ADC.Output, ADC.Elements, RawADCInput);
#endif
}

/**
 * convert raw rear brake reading into calibrated brake position
 */
int getBrakeR(uint16_t RawADCInput)
{
#ifdef HPF19
	if( usecanADC )  // check if we're operating on fake canbus ADC
	{
	  return CANADC.BrakeR;
	}
#endif
#ifdef NOBRAKES
	return 0;
#else
    struct ADCTable ADC = ADCInterpolationTables.BrakeR;
    return linearInterpolate(ADC.Input, ADC.Output, ADC.Elements, RawADCInput);
#endif
}

/**
 * convert front/rear brake inputs into brake balance percentage.
 */
int getBrakeBalance(uint16_t ADCInputF, uint16_t ADCInputR)
{
#ifdef HPF19
	if( usecanADC )  // check if we're operating on fake canbus ADC
	{ // lovely divide by zero possible here.
		if ( CANADC.BrakeF == 0) return 0;
		return (CANADC.BrakeF * 100) / ( CANADC.BrakeF + CANADC.BrakeR );
	}
#endif


	if (ADCInputF > 5 && ADCInputR > 5) // too small a value to get accurate reading
	{
		return (ADCInputF * 100) / ( ADCInputF + ADCInputR );
	} else
		return 0;

}

int getTorqueReqPercR( uint16_t RawADCInputR )
{
#ifdef HPF19
	if( usecanADC )  // check if we're operating on fake canbus ADC
	{
	  return CANADC.Torque_Req_R_Percent;
	}
#endif

    struct ADCTable ADC = ADCInterpolationTables.AccelR;
	return linearInterpolate(ADC.Input, ADC.Output, ADC.Elements, RawADCInputR);
}

int getTorqueReqPercL( uint16_t RawADCInputF )
{
#ifdef HPF19
	if( usecanADC )  // check if we're operating on fake canbus ADC
	{
	  return CANADC.Torque_Req_L_Percent;
	}
#endif
#ifdef NOAPPS
	return 0;
#else
    struct ADCTable ADC = ADCInterpolationTables.AccelL;
	return linearInterpolate(ADC.Input, ADC.Output, ADC.Elements, RawADCInputF);
#endif
}

int getBrakeTravelPerc( uint16_t RawADCInputF )
{
#ifdef NOAPPS
	return 0;
#else
    struct ADCTable ADC = ADCInterpolationTables.Regen;
	return linearInterpolate(ADC.Input, ADC.Output, ADC.Elements, RawADCInputF);
#endif
}


int getTorqueReqCurve( uint16_t ADCInput )
{
#ifdef HPF19
	if( usecanADC )  // check if we're operating on fake canbus ADC
	{
	  return CANADC.Torque_Req_L_Percent;
	}
#endif
#ifdef NOAPPS
	return 0;
#else
    struct ADCTable ADC = ADCInterpolationTables.TorqueCurve;
	int returnval= linearInterpolate(ADC.Input, ADC.Output, ADC.Elements, ADCInput);
	return returnval;
#endif
}


/**
 * process driving mode selector input, adjusts max possible torque request. Equivalent to gears.
 */
int getDrivingMode(uint16_t RawADCInput)
// this will become a configuration setting, changed at config portion of bootup only.
{ // torq_req_max, call once a second
#ifdef HPF19
	if( usecanADC )  // check if we're operating on fake canbus ADC
	{
	  return CANADC.DrivingMode;
	}
#endif
#ifndef STMADC
	return 5;
#endif

#ifdef NODRIVINGMODE
	return 5;
#else

    struct ADCTable ADC = ADCInterpolationTables.ModeSelector;

    int i;

    for(i=0;(ADC.Input[i] < RawADCInput) && (i < ADC.Elements-1);i++) { };

    //return this position in output table, no interpolation needed

    return ADC.Output[i];
#endif
}

int getCoolantTemp(uint16_t RawADCInput)
{
#ifdef HPF19
	if( usecanADC )  // check if we're operating on fake canbus ADC
	{
	  return CANADC.CoolantTempL;
	}
#endif
#ifndef STMADC
	return 20;
#endif

#ifdef NOTEMPERATURE
	return 20;
#else
    struct ADCTable ADC = ADCInterpolationTables.Coolant;

  //  if ( RawADCInput < CoolantInput[0] ) { return 0; }

    if ( RawADCInput > ADC.Input[ADC.Elements-1] ) { return 0; }

    return linearInterpolate(ADC.Input, ADC.Output, ADC.Elements, RawADCInput);
#endif
}


int getCoolantTemp2(uint16_t RawADCInput)
{
#ifdef HPF19
	if( usecanADC )  // check if we're operating on fake canbus ADC
	{
	  return CANADC.CoolantTempR;
	}
#endif
#ifndef STMADC
	return 20;
#endif

#ifdef NOTEMPERATURE
	return 20;
#else
    struct ADCTable ADC = ADCInterpolationTables.Coolant;

    if ( RawADCInput > ADC.Input[ADC.Elements-1] ) { return 0; }

    return linearInterpolate(ADC.Input, ADC.Output, ADC.Elements, RawADCInput);
#endif
}

#ifdef STMADC
void minmaxADCReset(void)
{
	for ( int i = 0; i<NumADCChan+NumADCChanADC3; i++)
	{
		ADC_DataMax[i]=0;
		ADC_DataMin[i]=0xFFFFFFFF;
	}
}
#endif


HAL_StatusTypeDef startADC(void)
{
#ifdef STMADC
	if ( (uint32_t) aADCxConvertedData < 0x24000000 ){
		while ( 1 ) {
			lcd_send_stringposDIR(0,0,"ADC DMA in wrong memory. ");
			lcd_send_stringposDIR(1,0,"Fix .LD and recompile! ");
		}
	}
#endif

	minmaxADC = 1;
	minmaxADCReset();
#ifdef STMADC
	ADC_MultiModeTypeDef multimode;
	multimode.Mode=ADC_MODE_INDEPENDENT;

#ifdef CALIBRATEADC
	if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK)
	{
		/* Calibration Error */
		Error_Handler();
	}

	if (HAL_ADCEx_Calibration_Start(&hadc3, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK)
	{
		/* Calibration Error */
		Error_Handler();
	}
#endif



	if ( HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
	{
		Error_Handler();
	}

#endif

	// in RTOS mode ADC poll is requested per cycle.

	DeviceState.ADC = OPERATIONAL;

	return 0;
}

#ifdef STMADC

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
	DeviceState.ADC = ERROR;
	if ( DeviceState.LCD == OPERATIONAL ){
		lcd_errormsg("ADC Error Check .LD");
	}
	toggleOutput(42);
}

void ReadADC1(bool half)
{
#ifdef CACHE
	// invalid the data cache before reading from it to ensure dma transfer readable
	SCB_InvalidateDCache_by_Addr (aADCxConvertedData, sizeof(aADCxConvertedData)+32 );
#endif

	ADCloops++;
	ADC1read = true;

	volatile int start = NumADCChan*SampleSize;
	if ( half ) start = 0;

	for (int i = 0; i<NumADCChan;i++)
	{
		uint16_t datasum = 0;
		for ( int j = 0; j<SampleSize;j++)
		{
			datasum += aADCxConvertedData[start+(i+j*NumADCChan)];//sum/SampleSize; // store the value in ADC_Data from buffer averaged over 10 samples for better accuracy.
		}

		ADC_Data[i] = datasum / (SampleSize);

		if ( minmaxADC )
		{
			if ( ADC_Data[i] < ADC_DataMin[i] ) ADC_DataMin[i] = ADC_Data[i];
			if ( ADC_Data[i] > ADC_DataMax[i] ) ADC_DataMax[i] = ADC_Data[i];
		}
	}

	if ( ADC3read ){ // if both ADC's are flagged as having been processed then mark all data available.
		ADCState.newdata = 1;
		ADC3read=false;
		ADC1read=false;
		ADCState.lastread=gettimer();
	}
}

void ReadADC3(bool half)
{
#ifdef CACHE
	// invalid the data cache before reading from it to ensure dma transfer readable
	SCB_InvalidateDCache_by_Addr (aADCxConvertedDataADC3, sizeof(aADCxConvertedDataADC3)+32 );
#endif
	ADC3loops++;
	ADC3read = true;
	volatile int start = NumADCChanADC3*SampleSize;
	if ( half ) start = 0;

	for (int i = 0; i<NumADCChanADC3;i++)
	{
		uint16_t datasum = 0;

		int pos = i + NumADCChan;

		for ( int j = 0; j<SampleSize;j++)
		{
			datasum += aADCxConvertedDataADC3[start+(i+j*NumADCChanADC3)];
		}

		ADC_Data[pos] = datasum / (SampleSize);
		if ( minmaxADC )
		{
			if ( ADC_Data[pos] < ADC_DataMin[pos] ) ADC_DataMin[pos] = ADC_Data[pos];
			if ( ADC_Data[pos] > ADC_DataMax[pos] ) ADC_DataMax[pos] = ADC_Data[pos];
		}
	}

	if ( ADC1read ){
		ADCState.newdata = 1;
		ADC3read=false;
		ADC1read=false;
		ADCState.lastread=gettimer();
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
//	ADC_msg msg;

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if(!usecanADC ) // don't process ADC values if ECU has been setup to use dummy values
	{
		if ( hadc->Instance == ADC1 )
		{
			blinkOutput(LED2, BlinkVeryFast, 1);
		    xSemaphoreGiveFromISR( xADC1, &xHigherPriorityTaskWoken );
		}
		else
		if ( hadc->Instance == ADC3 ) // TODO update for variable amount of ADC channels
		{
			blinkOutput(LED3, BlinkVeryFast, 1);
		    xSemaphoreGiveFromISR( xADC3, &xHigherPriorityTaskWoken );
		}

	    if( xHigherPriorityTaskWoken )
	    {
	        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	    }
	}
}
#endif


// checks all expected data is present and within acceptable range -- allow occasional error, remove inverter.
uint32_t CheckADCSanity( void )
{
	uint16_t returnvalue = 0;

	// check adc's are giving values in acceptable range.
	if ( abs(ADCState.SteeringAngle) >= 181 ) // if impossible angle.
	{
//            returnvalue &= ~(0x1 << SteeringAngleReceivedBit);
	 //   returnvalue |= 0x1 << SteeringAngleReceivedBit;
	}
	else returnvalue &= ~(0x1 << SteeringAngleReceivedBit);

	if ( DeviceState.CriticalSensors == OPERATIONAL ) // string will be empty if everything expected received.
	{

		if ( ADCState.BrakeF < 0 || ADCState.BrakeF >= 240 )
		   returnvalue |= 0x1 << BrakeFReceivedBit; // Received
		else returnvalue &= ~(0x1 << BrakeFReceivedBit); // ok

		if ( ADCState.BrakeR < 0 || ADCState.BrakeR >= 240 )
			returnvalue |= 0x1 << BrakeRReceivedBit;
		else returnvalue &= ~(0x1 << BrakeRReceivedBit);

		if ( ADCState.Torque_Req_R_Percent < 0 || ADCState.Torque_Req_R_Percent > 1000 ) // if value is not between 0 and 100 then out of range Received
			returnvalue |= 0x1 << AccelRReceivedBit;
		else returnvalue &= ~(0x1 << AccelRReceivedBit);

		if ( ADCState.Torque_Req_L_Percent < 0 || ADCState.Torque_Req_L_Percent > 1000 ) // if value is not between 0 and 100 then out of range Received
			returnvalue |= 0x1 << AccelLReceivedBit;
		else returnvalue &= ~(0x1 << AccelLReceivedBit);

	} else
	{ // not received data, set Received bits.
		returnvalue |= 0x1 << BrakeFReceivedBit; // Received
		returnvalue |= 0x1 << BrakeRReceivedBit;
		returnvalue |= 0x1 << AccelRReceivedBit;
		returnvalue |= 0x1 << AccelLReceivedBit;
		ADCState.BrakeF = 0; // if value is not between 0 and 255 then out of range Received
		ADCState.BrakeR = 0;
		ADCState.Torque_Req_R_Percent = 0;
		ADCState.Torque_Req_L_Percent = 0;
	}

	// calculate brake balance, if theres some pressure.
	if ( ADCState.BrakeF > 1 && ADCState.BrakeR > 1
		 && ADCState.BrakeF < 255 && ADCState.BrakeR < 255
	) {
		CarState.brake_balance = getBrakeBalance(ADCState.BrakeF, ADCState.BrakeR);
	} else CarState.brake_balance = -1;

	if ( returnvalue ){ // if error in adc data check if it's yet to be treated as fatal.
		Errors.ADCError++; // increase adc error counter
		Errors.ADCErrorState=returnvalue; // store current error value for checking.
		DeviceState.ADC = INERROR;

#ifdef STMADC
		for (int i=0;i<NumADCChan+2;i++)
		ADC_DataError[i] = ADC_Data[i];

		if ( Errors.ADCError < 5 )
		{
			returnvalue=0;
			Errors.ADCSent = false;
		} else
		{
			if ( !Errors.ADCSent ){
				Errors.ADCSent = true;
				CAN_SendADC(ADC_DataError, 1); // send error information to canbus - this should perhaps be latched to only happen once per error state.
			}
			returnvalue=0xFF; // 5 adc error reads happened in row, flag as error.

		}
#endif
	} else // no errors, clear flags.
	{
		if ( Errors.ADCError > 0 )	Errors.ADCError--;
		Errors.ADCErrorState=0;
	}

	return returnvalue;
}

#ifdef HPF19
bool receiveCANADCEnable(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle);  // debug ID to send arbitraty 'ADC' values for testing.
bool receiveCANADCInput(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle);

CANData ADCCANEnable = { 0, AdcSimInput_ID, 8, receiveCANInput, NULL, 0, 0 };
CANData ADCCANInput = { 0, AdcSimInput_ID, 8, receiveCANInput, NULL, 0, 0 };

bool receiveCANADCEnable(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle)
{
	if( CANRxData[0] == 1 && CANRxData[1]== 99 ) // if received value in ID is not 0 assume true and switch to fakeADC over CAN.
	{
//		stopADC(); //  disable ADC DMA interrupt to stop processing ADC input.
		// crashing if breakpoint ADC interrupt after this, just check variable in interrupt handler for now.
		usecanADC = 1; // set global state to use canbus ADC for feeding values.
		CANADC.SteeringAngle = 0; // set ADC_Data for steering
		CANADC.Torque_Req_L_Percent = 0; // set ADC_data for Left Throttle
		CANADC.Torque_Req_R_Percent = 0; // set ADC_data for Right Throttle
		CANADC.BrakeF = 0; // set ADC_data for Front Brake
		CANADC.BrakeR = 0; // set ADC_data for Rear Brake
		CANADC.DrivingMode = 5; // set ADC_Data for driving mode
		CANADC.CoolantTempL = 20; // set ADC_data for First Coolant Temp
		CANADC.CoolantTempR = 20; // set ADC_data for Second Coolant Temp
	} else // value of 0 received, switch back to real ADC.
	{
		usecanADC = 0;
	}
	return true;
}

bool receiveCANADCInput(uint8_t CANRxData[8], uint32_t DataLength, CANData * datahandle)
{
	CANADC.SteeringAngle = CANRxData[0]; // set ADC_Data for steering
	CANADC.Torque_Req_L_Percent = CANRxData[1]; // set ADC_data for Left Throttle
	CANADC.Torque_Req_R_Percent = CANRxData[2]; // set ADC_data for Right Throttle
	CANADC.BrakeF = CANRxData[3]; // set ADC_data for Front Brake
	CANADC.BrakeR = CANRxData[4]; // set ADC_data for Rear Brake
	CANADC.DrivingMode = CANRxData[5]; // set ADC_Data for driving mode
	CANADC.CoolantTempL = CANRxData[6]; // set ADC_data for First Coolant Temp
	CANADC.CoolantTempR = CANRxData[7]; // set ADC_data for Second Coolant Temp
	return true;
}

int initCANADC( void )
{
	RegisterCan1Message(ADCCANEnable);
	RegisterCan1Message(ADCCANInput);
}

#endif

void resetADC( void )
{
#ifdef HPF19
	usecanADC = 0;
#endif
}

int initADC( void )
{
#ifdef STMADC
	MX_ADC1_Init();
	MX_ADC3_Init();

	lcd_send_stringscroll("Start ADC");

	if ( startADC() == 0 )  //  starts the ADC dma processing.
	{
		DeviceState.ADC = OPERATIONAL;
	} else return 99;
#endif


	waitStr = xSemaphoreCreateMutex();

	ADCUpdate = xSemaphoreCreateMutex();

	ADCTaskHandle = xTaskCreateStatic(
	                      ADCTask,
	                      ADCTASKNAME,
	                      ADCSTACK_SIZE,
	                      ( void * ) 1,
	                      ADCTASKPRIORITY,
	                      xADCStack,
	                      &xADCTaskBuffer );


	configASSERT(ADCTaskHandle);

	return 0;
}
