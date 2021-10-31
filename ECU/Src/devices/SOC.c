/*
 * SOC.c
 *
 *  Created on: 20 Oct 2021
 *      Author: visa
 */


#include "ecumain.h"

#ifdef MATLAB

#include "GhettoSOC.h"   /* Model's header file */
#endif

int initSOC( void )
{
#ifdef MATLAB
	  GhettoSOC_initialize();
#endif
	return 0;
}

float doSOC(float U, float I )
{
	float ret = 0;
#ifdef MATLAB
	  static boolean_T OverrunFlag = false;

	  /* Disable interrupts here */

	  /* Check for overrun */
	  if (OverrunFlag) {
	    rtmSetErrorStatus(rtMSOC, "Overrun");
	    return 0;
	  }

	  OverrunFlag = true;

	  /* Save FPU context here (if necessary) */
	  /* Re-enable timer or interrupt here */
	  /* Set model inputs here */

	  /* Step the model */
	  GhettoSOC_step();

	  ret = rtYSOC.SOC;

	  /* Get model outputs here */

	  /* Indicate task complete */
	  OverrunFlag = false;

	  /* Disable interrupts here */
	  /* Restore FPU context here (if necessary) */
	  /* Enable interrupts here */
#endif

	  return ret;
}
