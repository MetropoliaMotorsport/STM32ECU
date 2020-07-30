/*
 * Sponsored License - for use in support of a program or activity
 * sponsored by MathWorks.  Not for government, commercial or other
 * non-sponsored organizational use.
 *
 * File: A_data.c
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

/* Invariant block signals (default storage) */
const ConstB_A_T A_ConstB = {
  19.6133                              /* '<S6>/Product2' */
};

/* Constant parameters (default storage) */
const ConstP_A_T A_ConstP = {
  /* Pooled Parameter (Expression: tanh([-5:5]))
   * Referenced by:
   *   '<S6>/I gain '
   *   '<S6>/P gain '
   */
  { -0.99990920426259511, -0.999329299739067, -0.99505475368673046,
    -0.9640275800758169, -0.76159415595576485, 0.0, 0.76159415595576485,
    0.9640275800758169, 0.99505475368673046, 0.999329299739067,
    0.99990920426259511 },

  /* Pooled Parameter (Expression: [-5:5])
   * Referenced by:
   *   '<S6>/I gain '
   *   '<S6>/P gain '
   */
  { -5.0, -4.0, -3.0, -2.0, -1.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0 }
};

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
