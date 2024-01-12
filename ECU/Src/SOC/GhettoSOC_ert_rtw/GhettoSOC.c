/*
 * Sponsored License - for use in support of a program or activity
 * sponsored by MathWorks.  Not for government, commercial or other
 * non-sponsored organizational use.
 *
 * File: GhettoSOC.c
 *
 * Code generated for Simulink model 'GhettoSOC'.
 *
 * Model version                  : 1.55
 * Simulink Coder version         : 9.3 (R2020a) 18-Nov-2019
 * C/C++ source code generated on : Mon Oct 11 18:41:10 2021
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#include "GhettoSOC.h"

/* Block signals and states (default storage) */
DWSOC rtDWSOC;

/* External inputs (root inport signals with default storage) */
ExtUSOC rtUSOC;

/* External outputs (root outports fed by signals with default storage) */
ExtYSOC rtYSOC;

/* Real-time model */
RT_MODELSOC rtMSOC_;
RT_MODELSOC *const rtMSOC = &rtMSOC_;

/* Model step function */
void GhettoSOC_step(void)
{
  real_T g;
  real_T d;
  real_T rtb_Gain;

  /* Gain: '<S2>/Gain' incorporates:
   *  Inport: '<Root>/U_battery'
   */
  rtb_Gain = 0.0069444444444444441 * rtUSOC.U_battery;

  /* MATLAB Function: '<S2>/Piecewise SoC-OCV estimation ' */
  if ((4.038 < rtb_Gain) && (rtb_Gain < 4.2)) {
    g = 88.55;
    d = 268.93;
  } else if ((3.998 < rtb_Gain) && (rtb_Gain < 4.038)) {
    g = 50.0;
    d = 116.13;
  } else if ((3.837 < rtb_Gain) && (rtb_Gain < 3.998)) {
    g = 149.38;
    d = 513.45;
  } else if ((3.776 < rtb_Gain) && (rtb_Gain < 3.837)) {
    g = 239.84;
    d = 860.53;
  } else if ((3.655 < rtb_Gain) && (rtb_Gain < 3.776)) {
    g = 298.1;
    d = 1080.5;
  } else if ((3.581 < rtb_Gain) && (rtb_Gain < 3.655)) {
    g = 51.486;
    d = 179.16;
  } else if ((3.002 < rtb_Gain) && (rtb_Gain < 3.581)) {
    g = 8.9983;
    d = 27.013;
  } else {
    g = 0.0;
    d = 0.0;
  }

  /* DiscreteIntegrator: '<S1>/Discrete-Time Integrator' incorporates:
   *  Gain: '<S1>/Gain1'
   *  MATLAB Function: '<S2>/Piecewise SoC-OCV estimation '
   *  Product: '<S2>/Product'
   *  Sum: '<S2>/Minus'
   */
  if (rtDWSOC.DiscreteTimeIntegrator_IC_LOADI != 0) {
    rtDWSOC.DiscreteTimeIntegrator_DSTATE = (rtb_Gain * g - d) * 0.01;
  }

  /* Gain: '<S1>/Gain' incorporates:
   *  DiscreteIntegrator: '<S1>/Discrete-Time Integrator'
   */
  rtb_Gain = 100.0 * rtDWSOC.DiscreteTimeIntegrator_DSTATE;

  /* Saturate: '<S1>/Saturation' */
  if (rtb_Gain > 100.0) {
    /* Outport: '<Root>/SOC' */
    rtYSOC.SOC = 100.0;
  } else if (rtb_Gain < 0.0) {
    /* Outport: '<Root>/SOC' */
    rtYSOC.SOC = 0.0;
  } else {
    /* Outport: '<Root>/SOC' */
		rtYSOC.SOC = rtb_Gain;
  }

  /* End of Saturate: '<S1>/Saturation' */

  /* Update for DiscreteIntegrator: '<S1>/Discrete-Time Integrator' incorporates:
   *  Constant: '<S1>/Constant'
   *  Gain: '<S1>/Gain2'
   *  Inport: '<Root>/I_battery'
   *  Product: '<S1>/Divide'
   */
  rtDWSOC.DiscreteTimeIntegrator_IC_LOADI = 0U;
  rtDWSOC.DiscreteTimeIntegrator_DSTATE += 0.00027777777777777778 * rtUSOC.I_battery /
    13.2 * 0.02;
}

/* Model initialize function */
void GhettoSOC_initialize(void)
{
  /* InitializeConditions for DiscreteIntegrator: '<S1>/Discrete-Time Integrator' */
  rtDWSOC.DiscreteTimeIntegrator_IC_LOADI = 1U;
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
