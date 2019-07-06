/*
 * ecu.c
 *
 *  Created on: 30 Dec 2018
 *      Author: Visa
 */

#include "ecumain.h"

volatile uint32_t ADCloops;

//variables that need to be accessible in ISR's

// ADC conversion buffer, should be aligned in memory for faster DMA?
ALIGN_32BYTES (static uint32_t aADCxConvertedData[ADC_CONVERTED_DATA_BUFFER_SIZE]);
ALIGN_32BYTES (static uint32_t aADCxConvertedDataADC3[ADC_CONVERTED_DATA_BUFFER_SIZE_ADC3]);

// new calibration

uint16_t SteeringInput[] = { 6539, 33500, 63019 };
int16_t SteeringOutput[] = { -195,   0,    210 };

// -1 needs to be at minimum
// should be 0 to 25bar at 1-5v   0.6666v to 3.3v at adc -> 13107 -> 65536
uint16_t BrakeRInput[] = {1024, 1025, BRAKEZERO, BRAKEMAX, 65535 }; // at 240bar, should be 240 output, at 0 bar should be 0 // 62914
int16_t BrakeROutput[] = {-1,     0,    0,     240,  255 }; // output range // 240

uint16_t BrakeFInput[] = { 1024, 1025, BRAKEZERO,   BRAKEMAX, 65535 }; // calibrated input range //62914
int16_t BrakeFOutput[] = { -1,     0,    0,     240,    255 }; // output range // 240

// zero should be approx real pedal zero, zero is read below this to allow for some variance without triggering errors.
// ditto max value.

// define zero as 5% actual travel and 100% as 95% of actual travel
uint16_t TorqueReqLInput[] = { 1999,  2000, (ACCELERATORLMAX-ACCELERATORLZERO)/100*5+ACCELERATORLZERO,   (ACCELERATORLMAX-ACCELERATORLZERO)/100*98+ACCELERATORLZERO,  64000,  64001 }; // calibration values for left input // 5800
int16_t TorqueReqLOutput[] = {  -1,  0,     0,     1000,      1000,  1001 }; // range defined 0-1000 to allow percentage accuracy even if not using full travel range.

// TorqueRMin(6798) / TorqueRMax(54369)
uint16_t TorqueReqRInput[] = { 1999, 2000, (ACCELERATORRMAX-ACCELERATORRZERO)/100*5+ACCELERATORRZERO,  (ACCELERATORRMAX-ACCELERATORRZERO)/100*98+ACCELERATORRZERO,   64000,   64001 }; // calibration values for right input // 6200
int16_t TorqueReqROutput[] = { -1,      0,      0,      1000,   1000,   1001 };


uint16_t TorqueLinearInput[] = {50,950}; // start registered travel at 8%
int16_t TorqueLinearOutput[] = {0,1000};

uint16_t TorqueLowTravelInput[] = {50,500}; // start registered travel at 10%
int16_t TorqueLowTravelOutput[] = {0, 1000};

uint16_t TorqueLargelowRangeInput[] = {50,600, 950}; // start registered travel at 10%
int16_t TorqueLargelowRangeOutput[] = {0, 400,1000};

uint16_t CoolantInput[] = { 1000,4235, 4851, 5661, 6889, 8952, 11246, 14262, 18894, 22968, 27081, 33576, 39050, 44819, 49192, 54011, 58954,  64113, 64112};
int16_t CoolantOutput[] = { -1,   120,   115,  109,  101,   90,    82,    72,    60,    52,    46,    38,    32,    26,    22,    16,    11,    6, -1};

uint16_t DrivingModeInput[] = { 0 , 1022, 1023, 1024,4500, 13500, 23500, 33000, 40000, 49000, 57500, 65534, 65535 };
int16_t DrivingModeOutput[] = { 1 , 1,    0,     1,   1,    2,    3,    4,   5,     6,    7,    8,   0 };

void SetupADCInterpolationTables( void )
{

    // calibrated input range for steering, from left lock to center to right lock.
    // check if this can be simplified?

    ADCInterpolationTables.Steering.Input = SteeringInput;
    ADCInterpolationTables.Steering.Output = SteeringOutput;

    ADCInterpolationTables.Steering.Elements = sizeof(SteeringInput)/sizeof(SteeringInput[0]); // calculate elements from memory size of whole array divided by size of on element.

    ADCInterpolationTables.BrakeR.Input = BrakeRInput;
    ADCInterpolationTables.BrakeR.Output = BrakeROutput;

    ADCInterpolationTables.BrakeR.Elements = sizeof(BrakeRInput)/sizeof(BrakeRInput[0]);

    ADCInterpolationTables.BrakeF.Input = BrakeFInput;
    ADCInterpolationTables.BrakeF.Output = BrakeFOutput;

    ADCInterpolationTables.BrakeF.Elements = sizeof(BrakeFInput)/sizeof(BrakeFInput[0]);

    ADCInterpolationTables.AccelL.Input = TorqueReqLInput;
    ADCInterpolationTables.AccelL.Output = TorqueReqLOutput;

    ADCInterpolationTables.AccelL.Elements = sizeof(TorqueReqLInput)/sizeof(TorqueReqLInput[0]);

    ADCInterpolationTables.AccelR.Input = TorqueReqRInput;
    ADCInterpolationTables.AccelR.Output = TorqueReqROutput;

    ADCInterpolationTables.AccelR.Elements = sizeof(TorqueReqRInput)/sizeof(TorqueReqRInput[0]);


    ADCInterpolationTables.TorqueCurve.Input = TorqueLinearInput;
    ADCInterpolationTables.TorqueCurve.Output = TorqueLinearOutput;

    ADCInterpolationTables.TorqueCurve.Elements = sizeof(TorqueLinearInput)/sizeof(TorqueLinearInput[0]);

    ADCInterpolationTables.CoolantL.Input = CoolantInput;
    ADCInterpolationTables.CoolantL.Output = CoolantOutput;

    ADCInterpolationTables.CoolantL.Elements = sizeof(CoolantInput)/sizeof(CoolantInput[0]);

    // currently set calibration of coolant 2 to same as coolant 1

    ADCInterpolationTables.CoolantR.Input = CoolantInput;
    ADCInterpolationTables.CoolantR.Output = CoolantOutput;

    ADCInterpolationTables.CoolantR.Elements = sizeof(CoolantInput)/sizeof(CoolantInput[0]);

    ADCInterpolationTables.ModeSelector.Input = DrivingModeInput;
    ADCInterpolationTables.ModeSelector.Output = DrivingModeOutput;

    ADCInterpolationTables.ModeSelector.Elements = sizeof(DrivingModeInput)/sizeof(DrivingModeInput[0]);
}

void SetupNormalTorque( void )
{
	ADCInterpolationTables.TorqueCurve.Input = TorqueLinearInput;
	ADCInterpolationTables.TorqueCurve.Output = TorqueLinearOutput;
	ADCInterpolationTables.TorqueCurve.Elements = sizeof(TorqueLinearInput)/sizeof(TorqueLinearInput[0]);
}

void SetupLargeLowRangeTorque( void )
{
	ADCInterpolationTables.TorqueCurve.Input = TorqueLargelowRangeInput;
	ADCInterpolationTables.TorqueCurve.Output = TorqueLargelowRangeOutput;
	ADCInterpolationTables.TorqueCurve.Elements = sizeof(TorqueLargelowRangeInput)/sizeof(TorqueLargelowRangeInput[0]);
}

void SetupLowTravelTorque( void )
{
    ADCInterpolationTables.TorqueCurve.Input = TorqueLowTravelInput;
    ADCInterpolationTables.TorqueCurve.Output = TorqueLowTravelOutput;
    ADCInterpolationTables.TorqueCurve.Elements = sizeof(TorqueLowTravelInput)/sizeof(TorqueLowTravelInput[0]);
}

/**
 * function to perform a linear interpolation using given input/output value arrays and raw data.
 */
int16_t linearInterpolate(uint16_t Input[], int16_t Output[], uint16_t count, uint16_t RawADCInput)
{
    int i;

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

/**
 * convert raw steering ADC input into a steering percentage left/right.
 * -- is there a deadzone?
 */
int getSteeringAngle(uint16_t RawADCInput) // input is only 12bit
{
	if( usecanADC )  // check if we're operating on fake canbus ADC
	{
	  return CANADC.SteeringAngle;
	}
#ifndef STMADC
	return 0;
#endif

#ifdef NOSTEERING
	return 0;
#else
    struct ADCTable ADC = ADCInterpolationTables.Steering;
    int angle=linearInterpolate(ADC.Input, ADC.Output, ADC.Elements, RawADCInput); // make input 12bit.
    return angle;
#endif
}

/**
 * convert raw front brake reading into calibrated brake position
 */
int getBrakeF(uint16_t RawADCInput)
{
	if( usecanADC )  // check if we're operating on fake canbus ADC
	{
	  return CANADC.BrakeF;
	}
#ifndef STMADC
	return 0;
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
	if( usecanADC )  // check if we're operating on fake canbus ADC
	{
	  return CANADC.BrakeR;
	}
#ifndef STMADC
	return 0;
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
	if( usecanADC )  // check if we're operating on fake canbus ADC
	{
	  return (CANADC.BrakeF * 100) / ( CANADC.BrakeF + CANADC.BrakeR );
	}
#ifndef STMADC
	return 50;
#endif

	if (ADCInputF > 5 && ADCInputR > 5) // too small a value to get accurate reading
	{
		return (ADCInputF * 100) / ( ADCInputF + ADCInputR );
	} else
		return 0;

}

int getTorqueReqPercR( uint16_t RawADCInputR )
{
	if( usecanADC )  // check if we're operating on fake canbus ADC
	{
	  return CANADC.Torque_Req_R_Percent;
	}
#ifndef STMADC
	return 0;
#endif

#ifdef NOAPPS
	return 0;
#else
    struct ADCTable ADC = ADCInterpolationTables.AccelR;
	return linearInterpolate(ADC.Input, ADC.Output, ADC.Elements, RawADCInputR);
#endif
}

int getTorqueReqPercL( uint16_t RawADCInputF )
{
	if( usecanADC )  // check if we're operating on fake canbus ADC
	{
	  return CANADC.Torque_Req_L_Percent;
	}
#ifndef STMADC
	return 0;
#endif
#ifdef NOAPPS
	return 0;
#else
    struct ADCTable ADC = ADCInterpolationTables.AccelL;
	return linearInterpolate(ADC.Input, ADC.Output, ADC.Elements, RawADCInputF);
#endif
}



int getTorqueReqCurve( uint16_t ADCInput )
{
	if( usecanADC )  // check if we're operating on fake canbus ADC
	{
	  return CANADC.Torque_Req_L_Percent;
	}
#ifndef STMADC
	return 0;
#endif
#ifdef NOAPPS
	return 0;
#else
    struct ADCTable ADC = ADCInterpolationTables.TorqueCurve;
	return linearInterpolate(ADC.Input, ADC.Output, ADC.Elements, ADCInput);
#endif
}


/**
 * process driving mode selector input, adjusts max possible torque request. Equivalent to gears.
 */
int getDrivingMode(uint16_t RawADCInput)
// this will become a configuration setting, changed at config portion of bootup only.
{ // torq_req_max, call once a second
	if( usecanADC )  // check if we're operating on fake canbus ADC
	{
	  return CANADC.DrivingMode;
	}
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

int getCoolantTemp1(uint16_t RawADCInput)
{
	if( usecanADC )  // check if we're operating on fake canbus ADC
	{
	  return CANADC.CoolantTempL;
	}

#ifndef STMADC
	return 20;
#endif

#ifdef NOTEMPERATURE
	return 20;
#else
    struct ADCTable ADC = ADCInterpolationTables.CoolantL;

  //  if ( RawADCInput < CoolantInput[0] ) { return 0; }

    if ( RawADCInput > ADC.Input[ADC.Elements-1] ) { return 0; }

    return linearInterpolate(ADC.Input, ADC.Output, ADC.Elements, RawADCInput);
#endif
}


int getCoolantTemp2(uint16_t RawADCInput)
{
	if( usecanADC )  // check if we're operating on fake canbus ADC
	{
	  return CANADC.CoolantTempR;
	}
#ifndef STMADC
	return 20;
#endif

#ifdef NOTEMPERATURE
	return 20;
#else
    struct ADCTable ADC = ADCInterpolationTables.CoolantR;

  //  if ( RawADCInput < CoolantInput[0] ) { return 0; }

    if ( RawADCInput > ADC.Input[ADC.Elements-1] ) { return 0; }

    return linearInterpolate(ADC.Input, ADC.Output, ADC.Elements, RawADCInput);
#endif
}

void minmaxADCReset(void)
{
	for ( int i = 0; i<NumADCChan+NumADCChanADC3; i++)
	{
		ADC_DataMax[i]=0;
		ADC_DataMin[i]=0xFFFFFFFF;
	}
}


HAL_StatusTypeDef startADC(void)
{
#ifdef STMADC
	minmaxADC = 1;
	minmaxADCReset();
	ADC_MultiModeTypeDef multimode;
	multimode.Mode=ADC_MODE_INDEPENDENT;
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

	if ( HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
	{
		Error_Handler();
	}

	if ( HAL_ADCEx_MultiModeStart_DMA(&hadc1, (uint32_t *)aADCxConvertedData, ADC_CONVERTED_DATA_BUFFER_SIZE)  != HAL_OK)
	{
		Error_Handler();
	}
	DeviceState.ADC = OPERATIONAL;
	// start ADC conversion
	//  return HAL_ADC_Start_DMA(&hadc1,(uint32_t *)aADCxConvertedData,ADC_CONVERTED_DATA_BUFFER_SIZE);
	if ( HAL_ADC_Start_DMA(&hadc3,(uint32_t *)aADCxConvertedDataADC3,ADC_CONVERTED_DATA_BUFFER_SIZE_ADC3) != HAL_OK)
	{
		Error_Handler();
	}

#endif

	return 0;
}


HAL_StatusTypeDef stopADC( void )
{


#ifdef STMADC
	  return (HAL_ADC_Stop_DMA(&hadc1) != HAL_OK); // this was causing a hang shortly after calling before.
#endif
		return 0;
}

#ifdef STMADC

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
// nothing yet
	if(!usecanADC) // don't process ADC values if ECU has been setup to use dummy values
	{
		if ( hadc->Instance == ADC1 )
		{
			ADCloops++;
			for (int i = 0; i<NumADCChan;i++)
			{
				int value = 0;
				{
					value = (aADCxConvertedData[i]);
				}
				ADC_Data[i] = value;
				if ( minmaxADC )
				{
					if ( ADC_Data[i] < ADC_DataMin[i] ) ADC_DataMin[i] = ADC_Data[i];
					if ( ADC_Data[i] > ADC_DataMax[i] ) ADC_DataMax[i] = ADC_Data[i];
				}
			}
			ADCState.newdata = 1;
		}
		else
		if ( hadc->Instance == ADC3 )
		{
			ADC_Data[8] =  aADCxConvertedDataADC3[0];
			ADC_Data[9] =  aADCxConvertedDataADC3[1];

			if ( minmaxADC )
			{
				for( int i = 8; i < 10; i++)
				{
					if ( ADC_Data[i] < ADC_DataMin[i] ) ADC_DataMin[i] = ADC_Data[i];
					if ( ADC_Data[i] > ADC_DataMax[i] ) ADC_DataMax[i] = ADC_Data[i];
				}
			}
		}

	}


}


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if(!usecanADC) // don't process ADC values if ECU has been setup to use dummy values
	{
		if ( hadc->Instance == ADC1 )
		{
			ADCloops++;
			for (int i = 0; i<NumADCChan;i++)
			{

				ADC_Data[i] = (aADCxConvertedData[i+NumADCChan]);//sum/SampleSize; // store the value in ADC_Data from buffer averaged over 10 samples for better accuracy.
				if ( minmaxADC )
				{
					if ( ADC_Data[i] < ADC_DataMin[i] ) ADC_DataMin[i] = ADC_Data[i];
					if ( ADC_Data[i] > ADC_DataMax[i] ) ADC_DataMax[i] = ADC_Data[i];
				}
			}
			ADCState.newdata = 1;
		}
		else
		if ( hadc->Instance == ADC3 )
		{
			ADC_Data[8] =  aADCxConvertedDataADC3[2];
			ADC_Data[9] =  aADCxConvertedDataADC3[3];

			if ( minmaxADC )
			{
				for( int i = 8; i < 10; i++)
				{
					if ( ADC_Data[i] < ADC_DataMin[i] ) ADC_DataMin[i] = ADC_Data[i];
					if ( ADC_Data[i] > ADC_DataMax[i] ) ADC_DataMax[i] = ADC_Data[i];
				}
			}
		}

	}
}
#endif


// checks all expected data is present and within acceptable range -- allow occasional error, remove inverter.
uint16_t CheckADCSanity( void )
{
	uint16_t returnvalue=(0x1 << BrakeFErrorBit)+
						(0x1 << BrakeRErrorBit)+
						(0x1 << AccelRErrorBit)+
						(0x1 << AccelLErrorBit)+
						(0x1 << CoolantLErrorBit)+
						(0x1 << CoolantRErrorBit)+
						(0x1 << SteeringAngleErrorBit)+
						(0x1 << DrivingModeErrorBit );
					//	(0x1 << BMSVoltageErrorBit); // set all bits to error state at start, should be 0 by end if OK

	// check bit bit = (number >> n) & 0x1;

	if ( DeviceState.ADC != OFFLINE )
	{
		// request ADC data.
        // check adc's are giving values in acceptable range.

        ADCState.BrakeF = getBrakeF(ADC_Data[BrakeFADC]); // if value is not between 0 and 255 then out of range error
		if ( ADCState.BrakeF < 0 || ADCState.BrakeF >= 240 )
		   returnvalue |= 0x1 << BrakeFErrorBit; // error
		else returnvalue &= ~(0x1 << BrakeFErrorBit); // ok

        ADCState.BrakeR = getBrakeR(ADC_Data[BrakeRADC]);
        if ( ADCState.BrakeR < 0 || ADCState.BrakeR >= 240 )
            returnvalue |= 0x1 << BrakeRErrorBit;
        else returnvalue &= ~(0x1 << BrakeRErrorBit);

        ADCState.Torque_Req_R_Percent = getTorqueReqPercR(ADC_Data[ThrottleRADC]);
        if ( ADCState.Torque_Req_R_Percent < 0 || ADCState.Torque_Req_R_Percent > 1000 ) // if value is not between 0 and 100 then out of range error
            returnvalue |= 0x1 << AccelRErrorBit;
        else returnvalue &= ~(0x1 << AccelRErrorBit);

        ADCState.Torque_Req_L_Percent = getTorqueReqPercL(ADC_Data[ThrottleLADC]);
        if ( ADCState.Torque_Req_L_Percent < 0 || ADCState.Torque_Req_L_Percent > 1000 ) // if value is not between 0 and 100 then out of range error
            returnvalue |= 0x1 << AccelLErrorBit;
        else returnvalue &= ~(0x1 << AccelLErrorBit);

        ADCState.CoolantTempL = getCoolantTemp1(ADC_Data[CoolantTempLADC]);
        if ( ADCState.CoolantTempL <= 0 || ADCState.CoolantTempL >= 255 )
        {
            ADCState.CoolantTempR = 255;
            returnvalue &= ~(0x1 << CoolantLErrorBit); // don't go to error state for invalid valuexs
           // returnvalue |= 0x1 << CoolantLErrorBit;
        }
        else returnvalue &= ~(0x1 << CoolantLErrorBit);

        static int16_t TempRHistory[30] = {0,0,0,0,0,0,0,0,0,0};
        static uint8_t TempRLast = 0;

        TempRHistory[TempRLast] = ADC_Data[CoolantTempRADC];
        TempRLast++;

    	if ( TempRLast > 29 ) TempRLast = 0;
    	long sum = 0;
    	for ( int i=0;i<30;i++)
    	{
    		sum += TempRHistory[i];
    	}

        ADCState.CoolantTempRRaw = sum/30;

        ADCState.CoolantTempR = getCoolantTemp2(ADC_Data[CoolantTempRADC]);
        if ( ADCState.CoolantTempR <= 0 || ADCState.CoolantTempR >= 255 )
        {
            ADCState.CoolantTempR = 255;
            returnvalue &= ~(0x1 << CoolantRErrorBit); // don't go to error state for invalid valuexs
         //   returnvalue |= 0x1 << CoolantRErrorBit;
        }
        else returnvalue &= ~(0x1 << CoolantRErrorBit);

        // rolling average to clean up steering angle a bit.

        static int16_t SteeringAngleHistory[10] = {0,0,0,0,0,0,0,0,0,0};
        static uint8_t SteeringAngleLast = 0;

        SteeringAngleHistory[SteeringAngleLast] = getSteeringAngle(ADC_Data[SteeringADC]);
        SteeringAngleLast++;

    	if ( SteeringAngleLast > 9 ) SteeringAngleLast = 0;

    	sum = 0;
    	for ( int i=0;i<10;i++)
    	{
    		sum += SteeringAngleHistory[i];
    	}

        ADCState.SteeringAngle = sum/10;// simple filter steering angle;

        if ( abs(ADCState.SteeringAngle) >= 130 ) // if impossible angle.
        {
            ADCState.SteeringAngle = 180;
            returnvalue &= ~(0x1 << SteeringAngleErrorBit);
         //   returnvalue |= 0x1 << SteeringAngleErrorBit;
        }
        else returnvalue &= ~(0x1 << SteeringAngleErrorBit);

        ADCState.DrivingMode = getDrivingMode(ADC_Data[DrivingModeADC]);
        if ( abs(ADCState.DrivingMode) == 0 )
        {
            ADCState.DrivingMode = 0;
            returnvalue &= ~(0x1 << DrivingModeErrorBit);
         //   returnvalue |= 0x1 << DrivingModeErrorBit;
        }
        else returnvalue &= ~(0x1 << DrivingModeErrorBit);

	} else // set all ADC to error bit if not online.
	{
		for ( int i=0;i<7;i++)
		{
			returnvalue |= 0x1 << i; // set error.
	//		returnvalue &= ~(0x1 << i); // currently setting all to zero to allow quick testing
		}
	}

//	if ( returnvalue == 0 ) {
		CarState.brake_balance = getBrakeBalance(ADCState.BrakeF, ADCState.BrakeR);
//	} else CarState.brake_balance = -1;

	if ( returnvalue ){
		Errors.ADCError++;
		Errors.ADCErrorState=returnvalue;
		for (int i=0;i<NumADCChan+2;i++)
		ADC_DataError[i] = ADC_Data[i];

		if ( Errors.ADCError < 5 )
		{
			returnvalue=0;
		} else
		{
			returnvalue=0xFF;
			CAN_SendADC(ADC_DataError, 1);
		}
	} else
	{
		if ( Errors.ADCError > 0 )	Errors.ADCError--;
		Errors.ADCErrorState=0;
	}

	return returnvalue;
}
