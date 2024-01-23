/*
 * Sponsored License - for use in support of a program or activity
 * sponsored by MathWorks.  Not for government, commercial or other
 * non-sponsored organizational use.
 *
 * File: TractionControl_data.c
 *
 * Code generated for Simulink model 'TractionControl'.
 *
 * Model version                  : 3.6
 * Simulink Coder version         : 9.8 (R2022b) 13-May-2022
 * C/C++ source code generated on : Sat Jul  1 15:09:13 2023
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex-M
 * Code generation objective: Traceability
 * Validation result: Not run
 */

#include "TractionControl.h"

/* Block parameters (default storage) */
TractionControl_P_e TractionControl_P = {
/* Variable: TireRadius
 * Referenced by: '<S1>/WheelSlipCalculation'
 */
0.26,

/* Mask Parameter: TCcontroller_InitialConditionForFilter
 * Referenced by: '<S33>/Filter'
 */
0.0,

/* Mask Parameter: TCcontroller_InitialConditionForIntegra
 * Referenced by: '<S38>/Integrator'
 */
0.0,

/* Mask Parameter: TCcontroller_LowerSaturationLimit
 * Referenced by:
 *   '<S45>/Saturation'
 *   '<S31>/DeadZone'
 */
0.0,

/* Mask Parameter: TCcontroller_UpperSaturationLimit
 * Referenced by:
 *   '<S45>/Saturation'
 *   '<S31>/DeadZone'
 */
25.0,

/* Expression: 0
 * Referenced by: '<S29>/Constant1'
 */
0.0,

/* Expression: inf
 * Referenced by: '<S1>/Limit to positive'
 */
0.0,

/* Expression: 0
 * Referenced by: '<S1>/Limit to positive'
 */
0.0,

/* Computed Parameter: Integrator_gainval
 * Referenced by: '<S38>/Integrator'
 */
0.01,

/* Expression: 100
 * Referenced by: '<S1>/N'
 */
100.0,

/* Computed Parameter: Filter_gainval
 * Referenced by: '<S33>/Filter'
 */
0.01,

/* Expression: 0
 * Referenced by: '<S29>/Clamping_zero'
 */
0.0,

/* Computed Parameter: Constant_Value
 * Referenced by: '<S29>/Constant'
 */
1,

/* Computed Parameter: Constant2_Value
 * Referenced by: '<S29>/Constant2'
 */
-1,

/* Computed Parameter: Constant3_Value
 * Referenced by: '<S29>/Constant3'
 */
1,

/* Computed Parameter: Constant4_Value
 * Referenced by: '<S29>/Constant4'
 */
-1 };

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
