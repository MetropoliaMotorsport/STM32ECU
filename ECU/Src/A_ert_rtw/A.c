/*
 * Sponsored License - for use in support of a program or activity
 * sponsored by MathWorks.  Not for government, commercial or other
 * non-sponsored organizational use.
 *
 * File: A.c
 *
 * Code generated for Simulink model 'A'.
 *
 * Model version                  : 1.30
 * Simulink Coder version         : 9.2 (R2019b) 18-Jul-2019
 * C/C++ source code generated on : Tue Jul 28 09:26:15 2020
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#include "A.h"
#include "A_private.h"

/* Block states (default storage) */
DW_A_T A_DW;

/* External inputs (root inport signals with default storage) */
ExtU_A_T A_U;

/* Real-time model */
RT_MODEL_A_T A_M_;
RT_MODEL_A_T *const A_M = &A_M_;
real_T look1_binlxpw(real_T u0, const real_T bp0[], const real_T table[],
                     uint32_T maxIndex)
{
  real_T frac;
  uint32_T iRght;
  uint32_T iLeft;
  uint32_T bpIdx;

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
     Overflow mode: 'portable wrapping'
   */
  return (table[iLeft + 1U] - table[iLeft]) * frac + table[iLeft];
}

/* Model step function */
void A_step(void)
{
  real_T tmp;

  /* Signum: '<S6>/Sign' incorporates:
   *  Inport: '<Root>/Vehicle velocity '
   */
  if (A_U.Velocity < 0.0) {
    tmp = -1.0;
  } else if (A_U.Velocity > 0.0) {
    tmp = 1.0;
  } else if (A_U.Velocity == 0.0) {
    tmp = 0.0;
  } else {
    tmp = (rtNaN);
  }

  /* End of Signum: '<S6>/Sign' */

  /* Update for DiscreteIntegrator: '<S40>/Integrator' incorporates:
   *  Constant: '<S6>/Behaviour gradient adjustable '
   *  Constant: '<S6>/Wheelbase'
   *  Gain: '<S6>/Gain'
   *  Gain: '<S6>/at wheel level'
   *  Inport: '<Root>/Steering'
   *  Inport: '<Root>/Vehicle velocity '
   *  Inport: '<Root>/Yaw rate'
   *  Lookup_n-D: '<S6>/I gain '
   *  Math: '<S6>/Square'
   *  MinMax: '<S6>/choose smallest'
   *  Product: '<S37>/IProd Out'
   *  Product: '<S6>/Divide'
   *  Product: '<S6>/Divide1'
   *  Product: '<S6>/Product'
   *  Product: '<S6>/Product1'
   *  Product: '<S6>/Product3'
   *  Sum: '<S6>/Subtract'
   *  Sum: '<S6>/Sum'
   */
  A_DW.Integrator_DSTATE += (fmin(0.2 * A_U.Velocity * A_U.Steering_angle /
    (A_U.Steering_angle * A_U.Steering_angle * 0.0 + 1.53), A_ConstB.Product2 /
    A_U.Steering_angle * 57.295779513082323 * tmp) - A_U.Yaw_rate) *
    look1_binlxpw(A_U.Steering_angle, A_ConstP.pooled2, A_ConstP.pooled1, 10U) *
    0.01;
}

/* Model initialize function */
void A_initialize(void)
{
  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));
}

/* Model terminate function */
void A_terminate(void)
{
  /* (no terminate code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
