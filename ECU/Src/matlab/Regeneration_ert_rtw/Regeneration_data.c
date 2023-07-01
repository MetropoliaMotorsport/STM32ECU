/*
 * Sponsored License - for use in support of a program or activity
 * sponsored by MathWorks.  Not for government, commercial or other
 * non-sponsored organizational use.
 *
 * File: Regeneration_data.c
 *
 * Code generated for Simulink model 'Regeneration'.
 *
 * Model version                  : 3.2
 * Simulink Coder version         : 9.8 (R2022b) 13-May-2022
 * C/C++ source code generated on : Sat Jul  1 15:04:50 2023
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex-M
 * Code generation objective: Traceability
 * Validation result: Not run
 */

#include "Regeneration.h"

/* Block parameters (default storage) */
Regeneration_P_e Regeneration_P = {
  /* Mask Parameter: Deactivateregenifspeedbelow9kmh_const
   * Referenced by: '<S11>/Constant'
   */
  1.4,

  /* Expression: 0
   * Referenced by: '<S10>/Constant'
   */
  0.0,

  /* Expression: [0 -1 -2 -3 -4 -5 -6 -7 -8 -9 -10 -11 -12 -13 -14 -15 -16 -17 -18 -19 -20 -21 -22 -23 -24 -25]
   * Referenced by: '<S10>/Map pedal to regen '
   */
  { 0.0, -1.0, -2.0, -3.0, -4.0, -5.0, -6.0, -7.0, -8.0, -9.0, -10.0, -11.0,
    -12.0, -13.0, -14.0, -15.0, -16.0, -17.0, -18.0, -19.0, -20.0, -21.0, -22.0,
    -23.0, -24.0, -25.0 },

  /* Expression: linspace(0,80,26)
   * Referenced by: '<S10>/Map pedal to regen '
   */
  { 0.0, 3.2, 6.4, 9.6, 12.8, 16.0, 19.2, 22.4, 25.6, 28.8, 32.0, 35.2, 38.4,
    41.6, 44.8, 48.0, 51.2, 54.4, 57.6, 60.8, 64.0, 67.2, 70.4, 73.6, 76.8, 80.0
  },

  /* Expression: 0
   * Referenced by: '<S13>/Constant'
   */
  0.0,

  /* Expression: -80000
   * Referenced by: '<S4>/Proportional to maximum regen power'
   */
  -80000.0,

  /* Expression: 7000
   * Referenced by: '<S4>/Safety offset'
   */
  7000.0,

  /* Expression: 0
   * Referenced by: '<S4>/Saturate to max regen of -100 kW'
   */
  0.0,

  /* Expression: -100000
   * Referenced by: '<S4>/Saturate to max regen of -100 kW'
   */
  -100000.0,

  /* Expression: 0.25
   * Referenced by: '<S5>/Max permissible negative slip'
   */
  0.25,

  /* Expression: 0
   * Referenced by: '<S18>/Constant'
   */
  0.0,

  /* Expression: 0
   * Referenced by: '<S5>/Saturation'
   */
  0.0,

  /* Expression: -1
   * Referenced by: '<S5>/Saturation'
   */
  -1.0,

  /* Expression: 0
   * Referenced by: '<S1>/Constant'
   */
  0.0,

  /* Expression: 1/1000
   * Referenced by: '<S4>/to kWh'
   */
  0.001,

  /* Expression: 6.98
   * Referenced by: '<S3>/max kWh'
   */
  6.98,

  /* Expression: 100
   * Referenced by: '<S3>/Gain'
   */
  100.0,

  /* Expression: 200
   * Referenced by: '<S4>/Critical voltage difference'
   */
  200.0,

  /* Expression: 0
   * Referenced by: '<S4>/Switch'
   */
  0.0,

  /* Expression: 1/4
   * Referenced by: '<S1>/Gain'
   */
  0.25
};

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
