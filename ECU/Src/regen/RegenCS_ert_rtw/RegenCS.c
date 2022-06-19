/*
 * Sponsored License - for use in support of a program or activity
 * sponsored by MathWorks.  Not for government, commercial or other
 * non-sponsored organizational use.
 *
 * File: RegenCS.c
 *
 * Code generated for Simulink model 'RegenCS'.
 *
 * Model version                  : 1.56
 * Simulink Coder version         : 9.6 (R2021b) 14-May-2021
 * C/C++ source code generated on : Fri Jun 17 20:13:01 2022
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex-M
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#include "RegenCS.h"

/* External inputs (root inport signals with default storage) */
ExtU regU;

/* External outputs (root outports fed by signals with default storage) */
ExtY regY;
static real_T look1_binlx(real_T u0, const real_T bp0[], const real_T table[],
  uint32_T maxIndex);
static real_T look1_binlx(real_T u0, const real_T bp0[], const real_T table[],
  uint32_T maxIndex)
{
  real_T frac;
  real_T yL_0d0;
  uint32_T iLeft;

  /* Column-major Lookup 1-D
     Search method: 'binary'
     Use previous index: 'off'
     Interpolation method: 'Linear point-slope'
     Extrapolation method: 'Linear'
     Use last breakpoint for index at or above upper limit: 'off'
     Remove protection against out-of-range input in generated code: 'off'
   */
  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Linear'
     Use previous index: 'off'
     Use last breakpoint for index at or above upper limit: 'off'
     Remove protection against out-of-range input in generated code: 'off'
   */
  if (u0 <= bp0[0U]) {
    iLeft = 0U;
    frac = (u0 - bp0[0U]) / (bp0[1U] - bp0[0U]);
  } else if (u0 < bp0[maxIndex]) {
    uint32_T bpIdx;
    uint32_T iRght;

    /* Binary Search */
    bpIdx = maxIndex >> 1U;
    iLeft = 0U;
    iRght = maxIndex;
    while (iRght - iLeft > 1U) {
      if (u0 < bp0[bpIdx]) {
        iRght = bpIdx;
      } else {
        iLeft = bpIdx;
      }

      bpIdx = (iRght + iLeft) >> 1U;
    }

    frac = (u0 - bp0[iLeft]) / (bp0[iLeft + 1U] - bp0[iLeft]);
  } else {
    iLeft = maxIndex - 1U;
    frac = (u0 - bp0[maxIndex - 1U]) / (bp0[maxIndex] - bp0[maxIndex - 1U]);
  }

  /* Column-major Interpolation 1-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'off'
     Overflow mode: 'wrapping'
   */
  yL_0d0 = table[iLeft];
  return (table[iLeft + 1U] - yL_0d0) * frac + yL_0d0;
}

/* Model step function */
void RegenCS_step(void)
{
  real_T rtb_Gain1;
  real_T rtb_Product4;
  real_T rtb_SteerReducing;
  real_T rtb_Switch1;
  real_T rtb_Torquebalance;

  /* Lookup_n-D: '<S1>/SteerReducing' incorporates:
   *  Abs: '<S1>/Abs'
   *  Inport: '<Root>/SteeringAngleDeg'
   */
  rtb_SteerReducing = look1_binlx(fabs(regU.SteeringAngleDeg),
    regConstP.SteerReducing_bp01Data, regConstP.SteerReducing_tableData, 9U);

  /* Gain: '<S2>/Gain1' incorporates:
   *  Inport: '<Root>/TorqueBalance'
   */
  rtb_Gain1 = 0.01 * regU.TorqueBalance;

  /* Product: '<S2>/Product4' incorporates:
   *  Gain: '<S2>/Gain3'
   *  Inport: '<Root>/MaxRegen'
   *  Inport: '<Root>/RegenPos'
   *  Lookup_n-D: '<S2>/Regen total torque'
   */
  rtb_Product4 = look1_binlx(regU.RegenPos, regConstP.Regentotaltorque_bp01Data,
    regConstP.Regentotaltorque_tableData, 10U) * -regU.MaxRegen;

  /* Lookup_n-D: '<S2>/Torque balance' incorporates:
   *  Inport: '<Root>/RegenPos'
   */
  rtb_Torquebalance = look1_binlx(regU.RegenPos,
    regConstP.Torquebalance_bp01Data, regConstP.Torquebalance_tableData, 9U);

  /* Switch: '<S2>/Switch' incorporates:
   *  Gain: '<S2>/Gain'
   *  Inport: '<Root>/RegenBalanceOn'
   *  Product: '<S2>/Product'
   *  Product: '<S2>/Product2'
   */
  if (regU.RegenBalanceOn >= 1.0) {
    rtb_Switch1 = rtb_Gain1 * rtb_Product4 * 2.0 * rtb_Torquebalance;
  } else {
    rtb_Switch1 = rtb_Gain1 * rtb_Product4;
  }

  /* End of Switch: '<S2>/Switch' */

  /* Switch: '<S1>/Switch' incorporates:
   *  Inport: '<Root>/SteerReducingOn'
   *  Product: '<S1>/Product'
   */
  if (regU.SteerReducingOn >= 1.0) {
    rtb_Switch1 *= rtb_SteerReducing;
  }

  /* End of Switch: '<S1>/Switch' */

  /* Gain: '<S1>/Transfertowheel' */
  rtb_Switch1 *= 0.5;

  /* Outport: '<Root>/RegenFL' */
  regY.RegenFL = rtb_Switch1;

  /* Outport: '<Root>/RegenFR' */
  regY.RegenFR = rtb_Switch1;

  /* Switch: '<S2>/Switch1' incorporates:
   *  Constant: '<S2>/Constant'
   *  Constant: '<S2>/Constant1'
   *  Gain: '<S2>/Gain2'
   *  Inport: '<Root>/RegenBalanceOn'
   *  Product: '<S2>/Product1'
   *  Product: '<S2>/Product3'
   *  Sum: '<S2>/Minus'
   *  Sum: '<S2>/Minus1'
   */
  if (regU.RegenBalanceOn >= 1.0) {
    rtb_Switch1 = (1.0 - rtb_Gain1) * rtb_Product4 * 2.0 * (1.0 -
      rtb_Torquebalance);
  } else {
    rtb_Switch1 = (1.0 - rtb_Gain1) * rtb_Product4;
  }

  /* End of Switch: '<S2>/Switch1' */

  /* Switch: '<S1>/Switch1' incorporates:
   *  Inport: '<Root>/SteerReducingOn'
   *  Product: '<S1>/Product1'
   */
  if (regU.SteerReducingOn >= 1.0) {
    rtb_Switch1 *= rtb_SteerReducing;
  }

  /* End of Switch: '<S1>/Switch1' */

  /* Gain: '<S1>/Transfertowheel1' */
  rtb_Switch1 *= 0.5;

  /* Outport: '<Root>/RegenRL' */
  regY.RegenRL = rtb_Switch1;

  /* Outport: '<Root>/RegenRR' */
  regY.RegenRR = rtb_Switch1;
}

/* Model initialize function */
void RegenCS_initialize(void)
{
  /* (no initialization code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
