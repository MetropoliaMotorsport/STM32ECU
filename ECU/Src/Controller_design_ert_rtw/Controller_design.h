/*
 * Sponsored License - for use in support of a program or activity
 * sponsored by MathWorks.  Not for government, commercial or other
 * non-sponsored organizational use.
 *
 * File: Controller_design.h
 *
 * Code generated for Simulink model 'Controller_design'.
 *
 * Model version                  : 1.48
 * Simulink Coder version         : 9.2 (R2019b) 18-Jul-2019
 * C/C++ source code generated on : Thu Sep  3 18:53:06 2020
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_Controller_design_h_
#define RTW_HEADER_Controller_design_h_
#include <stddef.h>
#include <math.h>
#ifndef Controller_design_COMMON_INCLUDES_
# define Controller_design_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif                                 /* Controller_design_COMMON_INCLUDES_ */

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
# define rtmGetErrorStatus(rtm)        ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
# define rtmSetErrorStatus(rtm, val)   ((rtm)->errorStatus = (val))
#endif

/* Forward declaration for rtModel */
typedef struct tag_RTM RT_MODEL;

/* Constant parameters (default storage) */
typedef struct {
  /* Expression: [120,130,152,157,177,242,249,258,258,272,263,248,227]
   * Referenced by: '<S3>/1-D Lookup Table'
   */
  real_T uDLookupTable_tableData[13];

  /* Expression: [1,2,4,6,8,10,12,14,16,18,22,24,26]
   * Referenced by: '<S3>/1-D Lookup Table'
   */
  real_T uDLookupTable_bp01Data[13];
} ConstP;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T velocity;                     /* '<Root>/velocity' */
  real_T Steering;                     /* '<Root>/Steering' */
  real_T Yawrate;                      /* '<Root>/Yaw rate' */
  real_T Modeselection;                /* '<Root>/Mode selection' */
} ExtU;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T tql;                          /* '<Root>/tql' */
  real_T tqr;                          /* '<Root>/tqr' */
  real_T val1;
  real_T val2;
  real_T val3;
} ExtY;

/* Real-time Model Data Structure */
struct tag_RTM {
  const char_T * volatile errorStatus;
};

/* External inputs (root inport signals with default storage) */
extern ExtU rtU;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY rtY;

/* Constant parameters (default storage) */
extern const ConstP rtConstP;

/* Model entry point functions */
extern void Controller_design_initialize(void);
extern void Controller_design_step(void);

/* Real-time Model object */
extern RT_MODEL *const rtM;

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
 * '<Root>' : 'Controller_design'
 * '<S1>'   : 'Controller_design/FeedForwardControllerforHPF019'
 * '<S2>'   : 'Controller_design/FeedForwardControllerforHPF019/Controller design '
 * '<S3>'   : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem'
 * '<S4>'   : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Desired value generator '
 * '<S5>'   : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Torque allocation for wheel '
 * '<S6>'   : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/Feed forward controller '
 * '<S7>'   : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller'
 * '<S8>'   : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Anti-windup'
 * '<S9>'   : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/D Gain'
 * '<S10>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Filter'
 * '<S11>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Filter ICs'
 * '<S12>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/I Gain'
 * '<S13>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Ideal P Gain'
 * '<S14>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Ideal P Gain Fdbk'
 * '<S15>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Integrator'
 * '<S16>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Integrator ICs'
 * '<S17>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/N Copy'
 * '<S18>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/N Gain'
 * '<S19>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/P Copy'
 * '<S20>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Parallel P Gain'
 * '<S21>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Reset Signal'
 * '<S22>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Saturation'
 * '<S23>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Saturation Fdbk'
 * '<S24>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Sum'
 * '<S25>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Sum Fdbk'
 * '<S26>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Tracking Mode'
 * '<S27>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Tracking Mode Sum'
 * '<S28>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/postSat Signal'
 * '<S29>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/preSat Signal'
 * '<S30>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Anti-windup/Disabled'
 * '<S31>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/D Gain/Disabled'
 * '<S32>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Filter/Disabled'
 * '<S33>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Filter ICs/Disabled'
 * '<S34>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/I Gain/Disabled'
 * '<S35>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Ideal P Gain/Passthrough'
 * '<S36>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Ideal P Gain Fdbk/Disabled'
 * '<S37>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Integrator/Disabled'
 * '<S38>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Integrator ICs/Disabled'
 * '<S39>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/N Copy/Disabled wSignal Specification'
 * '<S40>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/N Gain/Disabled'
 * '<S41>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/P Copy/Disabled'
 * '<S42>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Parallel P Gain/External Parameters'
 * '<S43>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Reset Signal/Disabled'
 * '<S44>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Saturation/Enabled'
 * '<S45>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Saturation Fdbk/Disabled'
 * '<S46>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Sum/Passthrough_P'
 * '<S47>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Sum Fdbk/Disabled'
 * '<S48>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Tracking Mode/Disabled'
 * '<S49>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/Tracking Mode Sum/Passthrough'
 * '<S50>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/postSat Signal/Forward_Path'
 * '<S51>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Controller subsystem/PID Controller/preSat Signal/Forward_Path'
 * '<S52>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Desired value generator /Yaw rate reference calculation '
 * '<S53>'  : 'Controller_design/FeedForwardControllerforHPF019/Controller design /Torque allocation for wheel /Torque allocation to wheels '
 */
#endif                                 /* RTW_HEADER_Controller_design_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
