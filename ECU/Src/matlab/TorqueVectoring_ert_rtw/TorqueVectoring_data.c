/*
 * Sponsored License - for use in support of a program or activity
 * sponsored by MathWorks.  Not for government, commercial or other
 * non-sponsored organizational use.
 *
 * File: TorqueVectoring_data.c
 *
 * Code generated for Simulink model 'TorqueVectoring'.
 *
 * Model version                  : 3.2
 * Simulink Coder version         : 9.8 (R2022b) 13-May-2022
 * C/C++ source code generated on : Sat Jul  1 15:08:39 2023
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex-M
 * Code generation objective: Traceability
 * Validation result: Not run
 */

#include "TorqueVectoring.h"

/* Block parameters (default storage) */
TorqueVectoring_P_e TorqueVectoring_P = {
  /* Variable: DeadzoneYawrateError
   * Referenced by: '<S10>/Dead Zone'
   */
  2.0,

  /* Variable: Gr
   * Referenced by:
   *   '<S7>/Constant'
   *   '<S7>/Constant1'
   */
  12.0,

  /* Variable: Ku
   * Referenced by: '<S6>/Gain'
   */
  0.0,

  /* Variable: LowerYawMoment
   * Referenced by:
   *   '<S5>/Yaw moment saturation'
   *   '<S8>/Yaw moment saturation'
   *   '<S9>/Yaw moment saturation'
   *   '<S52>/Saturation'
   *   '<S38>/DeadZone'
   */
  -3600.0,

  /* Variable: Rd
   * Referenced by:
   *   '<S7>/Gain'
   *   '<S7>/Gain1'
   */
  0.205,

  /* Variable: Sgr
   * Referenced by: '<S6>/Gain1'
   */
  5.0,

  /* Variable: SteeringDeadzonelimit
   * Referenced by: '<S3>/Dead Zone'
   */
  10.0,

  /* Variable: TuneGains
   * Referenced by:
   *   '<S10>/Gain1'
   *   '<S10>/Gain2'
   */
  1.0,

  /* Variable: UpperYawMoment
   * Referenced by:
   *   '<S5>/Yaw moment saturation'
   *   '<S8>/Yaw moment saturation'
   *   '<S9>/Yaw moment saturation'
   *   '<S52>/Saturation'
   *   '<S38>/DeadZone'
   */
  3600.0,

  /* Variable: feedforwardgain
   * Referenced by: '<S9>/Constant'
   */
  4.0,

  /* Variable: g
   * Referenced by: '<S6>/Constant2'
   */
  9.81,

  /* Variable: l
   * Referenced by: '<S6>/Constant'
   */
  1.53,

  /* Variable: myy
   * Referenced by: '<S6>/Constant1'
   */
  1.3,

  /* Variable: wf
   * Referenced by: '<S7>/Constant1'
   */
  0.6,

  /* Variable: wr
   * Referenced by: '<S7>/Constant'
   */
  0.6,

  /* Mask Parameter: DiscretePIDController_InitialConditionF
   * Referenced by: '<S45>/Integrator'
   */
  0.0,

  /* Mask Parameter: CompareToConstant_const
   * Referenced by: '<S2>/Constant'
   */
  2.4,

  // TODO fix to matlab, currently manually entered value for for CompareToConstant_torquepedal so variable reference positions do not break.
  5,

  /* Expression: 0
   * Referenced by: '<S36>/Constant1'
   */
  0.0,

  /* Expression: 180/pi
   * Referenced by: '<S6>/Gain2'
   */
  57.295779513082323,

  /* Expression: 180/pi
   * Referenced by: '<S10>/Gain'
   */
  57.295779513082323,

  /* Expression: [0;55.7000000000000;59.9500000000000;120.600000000000;158.050000000000;169.950000000000;175.100000000000;165.900000000000;181.500000000000;112.650000000000;176.750000000000;175;158.900000000000]
   * Referenced by: '<S10>/P gains'
   */
  { 0.0, 55.7, 59.95, 120.6, 158.05, 169.95, 175.1, 165.9, 181.5, 112.65, 176.75,
    175.0, 158.9 },

  /* Expression: [0;3;5;7;9;11;13;15;17;19;21;23;25]
   * Referenced by: '<S10>/P gains'
   */
  { 0.0, 3.0, 5.0, 7.0, 9.0, 11.0, 13.0, 15.0, 17.0, 19.0, 21.0, 23.0, 25.0 },

  /* Computed Parameter: Integrator_gainval
   * Referenced by: '<S45>/Integrator'
   */
  0.01,

  /* Expression: 0.5
   * Referenced by: '<S7>/Gain3'
   */
  0.5,

  /* Expression: 0.5
   * Referenced by: '<S4>/Gain'
   */
  0.5,

  /* Expression: 0.5
   * Referenced by: '<S7>/Gain2'
   */
  0.5,

  /* Expression: 0.5
   * Referenced by: '<S4>/Gain1'
   */
  0.5,

  /* Expression: 0
   * Referenced by: '<S36>/Clamping_zero'
   */
  0.0,

  /* Expression: [0;4683;3141;3303.50000000000;3470.50000000000;2706;2433.50000000000;1907;2437;1644;3088;2655;1975]
   * Referenced by: '<S10>/I gains'
   */
  { 0.0, 4683.0, 3141.0, 3303.5, 3470.5, 2706.0, 2433.5, 1907.0, 2437.0, 1644.0,
    3088.0, 2655.0, 1975.0 },

  /* Expression: [0;3;5;7;9;11;13;15;17;19;21;23;25]
   * Referenced by: '<S10>/I gains'
   */
  { 0.0, 3.0, 5.0, 7.0, 9.0, 11.0, 13.0, 15.0, 17.0, 19.0, 21.0, 23.0, 25.0 },

  /* Computed Parameter: Constant_Value
   * Referenced by: '<S36>/Constant'
   */
  1,

  /* Computed Parameter: Constant2_Value
   * Referenced by: '<S36>/Constant2'
   */
  -1,

  /* Computed Parameter: Constant3_Value
   * Referenced by: '<S36>/Constant3'
   */
  1,

  /* Computed Parameter: Constant4_Value
   * Referenced by: '<S36>/Constant4'
   */
  -1
};

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
