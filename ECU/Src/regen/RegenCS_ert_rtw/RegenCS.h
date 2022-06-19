/*
 * Sponsored License - for use in support of a program or activity
 * sponsored by MathWorks.  Not for government, commercial or other
 * non-sponsored organizational use.
 *
 * File: RegenCS.h
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

#ifndef RTW_HEADER_RegenCS_h_
#define RTW_HEADER_RegenCS_h_
#include <math.h>
#ifndef RegenCS_COMMON_INCLUDES_
#define RegenCS_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif                                 /* RegenCS_COMMON_INCLUDES_ */

/* Model Code Variants */

/* Macros for accessing real-time model data structure */

/* Constant parameters (default storage) */
typedef struct {
  /* Expression: [1.0000 0.9444 0.8889 0.8333 0.7778 0.7222 0.6667 0.6111 0.5556 0.5000]
   * Referenced by: '<S1>/SteerReducing'
   */
  real_T SteerReducing_tableData[10];

  /* Expression: [0 15.5556 31.1111 46.6667 62.2222 77.7778 93.3333 108.8889 124.4444 140.0000]
   * Referenced by: '<S1>/SteerReducing'
   */
  real_T SteerReducing_bp01Data[10];

  /* Expression: [0 0 0.1111 0.2222 0.3333 0.4444 0.5556 0.6667 0.7778 0.8889  1.0000]*4
   * Referenced by: '<S2>/Regen total torque'
   */
  real_T Regentotaltorque_tableData[11];

  /* Expression: [0 20.0000 34.4444 48.8889 63.3333 77.7778 92.2222 106.6667 121.1111 135.5556 150.0000]
   * Referenced by: '<S2>/Regen total torque'
   */
  real_T Regentotaltorque_bp01Data[11];

  /* Expression:  [50 51.11 52.22 53.33 54.44 55.55 56.66 57.77 58.88 60]/100
   * Referenced by: '<S2>/Torque balance'
   */
  real_T Torquebalance_tableData[10];

  /* Expression: [75.0000 83.3333 91.6667 100.0000 108.3333 116.6667 125.0000 133.3333 141.6667 150.0000]
   * Referenced by: '<S2>/Torque balance'
   */
  real_T Torquebalance_bp01Data[10];
} ConstP;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T RegenPos;                     /* '<Root>/RegenPos' */
  real_T SteeringAngleDeg;             /* '<Root>/SteeringAngleDeg' */
  real_T SteerReducingOn;              /* '<Root>/SteerReducingOn' */
  real_T RegenBalanceOn;               /* '<Root>/RegenBalanceOn' */
  real_T TorqueBalance;                /* '<Root>/TorqueBalance' */
  real_T MaxRegen;                     /* '<Root>/MaxRegen' */
} ExtU;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T RegenFL;                      /* '<Root>/RegenFL' */
  real_T RegenFR;                      /* '<Root>/RegenFR' */
  real_T RegenRL;                      /* '<Root>/RegenRL' */
  real_T RegenRR;                      /* '<Root>/RegenRR' */
} ExtY;

/* External inputs (root inport signals with default storage) */
extern ExtU regU;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY regY;

/* Constant parameters (default storage) */
extern const ConstP regConstP;

/* Model entry point functions */
extern void RegenCS_initialize(void);
extern void RegenCS_step(void);

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Use the MATLAB hilite_system command to trace the generated code back
 * to the model.  For example,
 *
 * hilite_system('<S3>')    - opens system 3
 * hilite_system('<S3>/Kp') - opens and selects block Kp which resides in S3
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'RegenCS'
 * '<S1>'   : 'RegenCS/Regen_Balance'
 * '<S2>'   : 'RegenCS/Regen_Balance/RegenPos_Torque '
 */
#endif                                 /* RTW_HEADER_RegenCS_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
