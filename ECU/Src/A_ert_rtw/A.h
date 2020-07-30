/*
 * Sponsored License - for use in support of a program or activity
 * sponsored by MathWorks.  Not for government, commercial or other
 * non-sponsored organizational use.
 *
 * File: A.h
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

#ifndef RTW_HEADER_A_h_
#define RTW_HEADER_A_h_
#include <math.h>
#ifndef A_COMMON_INCLUDES_
# define A_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif                                 /* A_COMMON_INCLUDES_ */

#include "A_types.h"
#include "rtGetNaN.h"
#include "rt_nonfinite.h"
#include "rtGetInf.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
# define rtmGetErrorStatus(rtm)        ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
# define rtmSetErrorStatus(rtm, val)   ((rtm)->errorStatus = (val))
#endif

/* Block states (default storage) for system '<Root>' */
typedef struct {
  real_T Integrator_DSTATE;            /* '<S40>/Integrator' */
} DW_A_T;

/* Invariant block signals (default storage) */
typedef struct {
  const real_T Product2;               /* '<S6>/Product2' */
} ConstB_A_T;

/* Constant parameters (default storage) */
typedef struct {
  /* Pooled Parameter (Expression: tanh([-5:5]))
   * Referenced by:
   *   '<S6>/I gain '
   *   '<S6>/P gain '
   */
  real_T pooled1[11];

  /* Pooled Parameter (Expression: [-5:5])
   * Referenced by:
   *   '<S6>/I gain '
   *   '<S6>/P gain '
   */
  real_T pooled2[11];
} ConstP_A_T;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T Yaw_rate;                     /* '<Root>/Yaw rate' */
  real_T Velocity;                     /* '<Root>/Vehicle velocity ' */
  real_T Wheel_speed;                  /* '<Root>/Wheel speeds ' */
  real_T Side_slip;                    /* '<Root>/Vehicle body slip ' */
  real_T Longitudinalacceleration;     /* '<Root>/Longitudinal acceleration ' */
  real_T Steering_angle;               /* '<Root>/Steering' */
  real_T Throttle_request;             /* '<Root>/Throttle request' */
  real_T Regenerationrequest;          /* '<Root>/Regeneration request' */
  real_T Torquevectoringmode;          /* '<Root>/Torque vectoring mode' */
  real_T Lateralacceleration;          /* '<Root>/Lateral acceleration' */
} ExtU_A_T;

/* Real-time Model Data Structure */
struct tag_RTM_A_T {
  const char_T * volatile errorStatus;
};

/* Block states (default storage) */
extern DW_A_T A_DW;

/* External inputs (root inport signals with default storage) */
extern ExtU_A_T A_U;
extern const ConstB_A_T A_ConstB;      /* constant block i/o */

/* Constant parameters (default storage) */
extern const ConstP_A_T A_ConstP;

/* Model entry point functions */
extern void A_initialize(void);
extern void A_step(void);
extern void A_terminate(void);

/* Real-time Model object */
extern RT_MODEL_A_T *const A_M;

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
 * '<Root>' : 'A'
 * '<S1>'   : 'A/Controller sub system '
 * '<S2>'   : 'A/Controller sub system /Torque vectoring '
 * '<S3>'   : 'A/Controller sub system /Torque vectoring /Feed_forward_Mode_3'
 * '<S4>'   : 'A/Controller sub system /Torque vectoring /Lateral_acceleration_Mode_2'
 * '<S5>'   : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1'
 * '<S6>'   : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Desired values//pre-processor '
 * '<S7>'   : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque allocation//post-processor  '
 * '<S8>'   : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller '
 * '<S9>'   : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque allocation//post-processor  /Torque allocation'
 * '<S10>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller'
 * '<S11>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Anti-windup'
 * '<S12>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/D Gain'
 * '<S13>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Filter'
 * '<S14>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Filter ICs'
 * '<S15>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/I Gain'
 * '<S16>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Ideal P Gain'
 * '<S17>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Ideal P Gain Fdbk'
 * '<S18>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Integrator'
 * '<S19>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Integrator ICs'
 * '<S20>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/N Copy'
 * '<S21>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/N Gain'
 * '<S22>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/P Copy'
 * '<S23>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Parallel P Gain'
 * '<S24>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Reset Signal'
 * '<S25>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Saturation'
 * '<S26>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Saturation Fdbk'
 * '<S27>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Sum'
 * '<S28>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Sum Fdbk'
 * '<S29>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Tracking Mode'
 * '<S30>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Tracking Mode Sum'
 * '<S31>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/postSat Signal'
 * '<S32>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/preSat Signal'
 * '<S33>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Anti-windup/Passthrough'
 * '<S34>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/D Gain/Disabled'
 * '<S35>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Filter/Disabled'
 * '<S36>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Filter ICs/Disabled'
 * '<S37>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/I Gain/External Parameters'
 * '<S38>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Ideal P Gain/Passthrough'
 * '<S39>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Ideal P Gain Fdbk/Disabled'
 * '<S40>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Integrator/Discrete'
 * '<S41>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Integrator ICs/Internal IC'
 * '<S42>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/N Copy/Disabled wSignal Specification'
 * '<S43>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/N Gain/Disabled'
 * '<S44>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/P Copy/Disabled'
 * '<S45>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Parallel P Gain/External Parameters'
 * '<S46>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Reset Signal/Disabled'
 * '<S47>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Saturation/Enabled'
 * '<S48>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Saturation Fdbk/Disabled'
 * '<S49>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Sum/Sum_PI'
 * '<S50>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Sum Fdbk/Disabled'
 * '<S51>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Tracking Mode/Disabled'
 * '<S52>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/Tracking Mode Sum/Passthrough'
 * '<S53>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/postSat Signal/Forward_Path'
 * '<S54>'  : 'A/Controller sub system /Torque vectoring /PI_controller_Mode_1/Torque vectoring controller /PID Controller/preSat Signal/Forward_Path'
 */
#endif                                 /* RTW_HEADER_A_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
