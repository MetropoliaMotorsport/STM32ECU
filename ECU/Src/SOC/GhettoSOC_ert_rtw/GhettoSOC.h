/*
 * Sponsored License - for use in support of a program or activity
 * sponsored by MathWorks.  Not for government, commercial or other
 * non-sponsored organizational use.
 *
 * File: GhettoSOC.h
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

#ifndef RTW_HEADER_GhettoSOC_h_
#define RTW_HEADER_GhettoSOC_h_
#ifndef GhettoSOC_COMMON_INCLUDES_
# define GhettoSOC_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif                                 /* GhettoSOC_COMMON_INCLUDES_ */

/* Model Code Variants */

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
# define rtmGetErrorStatus(rtmSOC)        ((rtmSOC)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
# define rtmSetErrorStatus(rtmSOC, val)   ((rtmSOC)->errorStatus = (val))
#endif

/* Forward declaration for rtModel */
typedef struct tag_RTMSOC RT_MODELSOC;

/* Block signals and states (default storage) for system '<Root>' */
typedef struct {
  real_T DiscreteTimeIntegrator_DSTATE;/* '<S1>/Discrete-Time Integrator' */
  uint8_T DiscreteTimeIntegrator_IC_LOADI;/* '<S1>/Discrete-Time Integrator' */
} DWSOC;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T U_battery;                    /* '<Root>/U_battery' */
  real_T I_battery;                    /* '<Root>/I_battery' */
} ExtUSOC;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T SOC;                          /* '<Root>/SOC' */
} ExtYSOC;

/* Real-time Model Data Structure */
struct tag_RTMSOC {
  const char_T * volatile errorStatus;
};

/* Block signals and states (default storage) */
extern DWSOC rtDWSOC;

/* External inputs (root inport signals with default storage) */
extern ExtUSOC rtUSOC;

/* External outputs (root outports fed by signals with default storage) */
extern ExtYSOC rtYSOC;

/* Model entry point functions */
extern void GhettoSOC_initialize(void);
extern void GhettoSOC_step(void);

/* Real-time Model object */
extern RT_MODELSOC *const rtMSOC;

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
 * '<Root>' : 'GhettoSOC'
 * '<S1>'   : 'GhettoSOC/Piecewise function with coulomb counting method '
 * '<S2>'   : 'GhettoSOC/Piecewise function with coulomb counting method /Calculate initlal SoC from initial voltage '
 * '<S3>'   : 'GhettoSOC/Piecewise function with coulomb counting method /Calculate initlal SoC from initial voltage /Piecewise SoC-OCV estimation '
 */
#endif                                 /* RTW_HEADER_GhettoSOC_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
