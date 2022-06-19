/*
 * Sponsored License - for use in support of a program or activity
 * sponsored by MathWorks.  Not for government, commercial or other
 * non-sponsored organizational use.
 *
 * File: RegenCS_data.c
 *
 * Code generated for Simulink model 'RegenCS'.
 *
 * Model version                  : 1.57
 * Simulink Coder version         : 9.6 (R2021b) 14-May-2021
 * C/C++ source code generated on : Sun Jun 19 15:29:07 2022
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex-M
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#include "RegenCS.h"

/* Constant parameters (default storage) */
const regConstP regConstP_d = {
  /* Expression: [1.0000 0.9444 0.8889 0.8333 0.7778 0.7222 0.6667 0.6111 0.5556 0.5000]
   * Referenced by: '<S1>/SteerReducing'
   */
  { 1.0, 0.9444, 0.8889, 0.8333, 0.7778, 0.7222, 0.6667, 0.6111, 0.5556, 0.5 },

  /* Expression: [0 15.5556 31.1111 46.6667 62.2222 77.7778 93.3333 108.8889 124.4444 140.0000]
   * Referenced by: '<S1>/SteerReducing'
   */
  { 0.0, 15.5556, 31.1111, 46.6667, 62.2222, 77.7778, 93.3333, 108.8889,
    124.4444, 140.0 },

  /* Expression: [0 0 0.1111 0.2222 0.3333 0.4444 0.5556 0.6667 0.7778 0.8889  1.0000]*4
   * Referenced by: '<S2>/Regen total torque'
   */
  { 0.0, 0.0, 0.4444, 0.8888, 1.3332, 1.7776, 2.2224, 2.6668, 3.1112, 3.5556,
    4.0 },

  /* Expression: [0 20.0000 34.4444 48.8889 63.3333 77.7778 92.2222 106.6667 121.1111 135.5556 150.0000]
   * Referenced by: '<S2>/Regen total torque'
   */
  { 0.0, 20.0, 34.4444, 48.8889, 63.3333, 77.7778, 92.2222, 106.6667, 121.1111,
    135.5556, 150.0 },

  /* Expression:  [50 51.11 52.22 53.33 54.44 55.55 56.66 57.77 58.88 60]/100
   * Referenced by: '<S2>/Torque balance'
   */
  { 0.5, 0.5111, 0.5222, 0.5333, 0.5444, 0.5555, 0.5666, 0.5777, 0.5888, 0.6 },

  /* Expression: [75.0000 83.3333 91.6667 100.0000 108.3333 116.6667 125.0000 133.3333 141.6667 150.0000]
   * Referenced by: '<S2>/Torque balance'
   */
  { 75.0, 83.3333, 91.6667, 100.0, 108.3333, 116.6667, 125.0, 133.3333, 141.6667,
    150.0 }
};

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
