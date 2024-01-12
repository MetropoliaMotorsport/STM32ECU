/*
 * Sponsored License - for use in support of a program or activity
 * sponsored by MathWorks.  Not for government, commercial or other
 * non-sponsored organizational use.
 *
 * File: TractionControl.h
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

#ifndef RTW_HEADER_TractionControl_h_
#define RTW_HEADER_TractionControl_h_
#ifndef TractionControl_COMMON_INCLUDES_
#define TractionControl_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif                                 /* TractionControl_COMMON_INCLUDES_ */

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

/* Forward declaration for rtModel */
typedef struct TractionControl_tag_RTM TractionControl_RT_MODEL;

/* Block signals and states (default storage) for system '<Root>' */
typedef struct {
  real_T Integrator_DSTATE[4];         /* '<S38>/Integrator' */
  real_T Filter_DSTATE[4];             /* '<S33>/Filter' */
  int8_T Integrator_PrevResetState[4]; /* '<S38>/Integrator' */
  int8_T Filter_PrevResetState[4];     /* '<S33>/Filter' */
} TractionControl_DW;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T VehicleSpeed;                 /* '<Root>/VehicleSpeed' */
  real_T WheelRotVelocityFL;           /* '<Root>/WheelRotVelocityFL' */
  real_T WheelRotVelocityFR;           /* '<Root>/WheelRotVelocityFR' */
  real_T WheelRotVelocityRL;           /* '<Root>/WheelRotVelocityRL' */
  real_T WheelRotVelocityRR;           /* '<Root>/WheelRotVelocityRR' */
  real_T Desiredwheelslip;             /* '<Root>/UpperSlipThreshold' */
  real_T LowerSlipThreshold;           /* '<Root>/LowerSlipThreshold' */
  real_T Proportionalgain;             /* '<Root>/Kp' */
  real_T Integralgain;                 /* '<Root>/Ki' */
  real_T Derivativegain;               /* '<Root>/Kd' */
  real_T BrakePressure;                /* '<Root>/BrakePressure' */
  real_T TC_enabled;                   /* '<Root>/TC_enabled' */
} TractionControl_ExtU;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T TC_FL;                        /* '<Root>/TC_FL' */
  real_T TC_FR;                        /* '<Root>/TC_FR' */
  real_T TC_RR;                        /* '<Root>/TC_RR' */
  real_T TC_RL;                        /* '<Root>/TC_RL' */
  real_T slipFL;                       /* '<Root>/slipFL' */
  real_T slipFR;                       /* '<Root>/slipFR' */
  real_T slipRL;                       /* '<Root>/slipRL' */
  real_T slipRR;                       /* '<Root>/slipRR' */
} TractionControl_ExtY;

/* Parameters (default storage) */
struct TractionControl_P_e_ {
  real_T TireRadius;                   /* Variable: TireRadius
                                        * Referenced by: '<S1>/WheelSlipCalculation'
                                        */
  real_T TCcontroller_InitialConditionForFilter;
                       /* Mask Parameter: TCcontroller_InitialConditionForFilter
                        * Referenced by: '<S33>/Filter'
                        */
  real_T TCcontroller_InitialConditionForIntegra;
                      /* Mask Parameter: TCcontroller_InitialConditionForIntegra
                       * Referenced by: '<S38>/Integrator'
                       */
  real_T TCcontroller_LowerSaturationLimit;
                            /* Mask Parameter: TCcontroller_LowerSaturationLimit
                             * Referenced by:
                             *   '<S45>/Saturation'
                             *   '<S31>/DeadZone'
                             */
  real_T TCcontroller_UpperSaturationLimit;
                            /* Mask Parameter: TCcontroller_UpperSaturationLimit
                             * Referenced by:
                             *   '<S45>/Saturation'
                             *   '<S31>/DeadZone'
                             */
  real_T Constant1_Value;              /* Expression: 0
                                        * Referenced by: '<S29>/Constant1'
                                        */
  real_T Limittopositive_UpperSat;     /* Expression: inf
                                        * Referenced by: '<S1>/Limit to positive'
                                        */
  real_T Limittopositive_LowerSat;     /* Expression: 0
                                        * Referenced by: '<S1>/Limit to positive'
                                        */
  real_T Integrator_gainval;           /* Computed Parameter: Integrator_gainval
                                        * Referenced by: '<S38>/Integrator'
                                        */
  real_T N_Value;                      /* Expression: 100
                                        * Referenced by: '<S1>/N'
                                        */
  real_T Filter_gainval;               /* Computed Parameter: Filter_gainval
                                        * Referenced by: '<S33>/Filter'
                                        */
  real_T Clamping_zero_Value;          /* Expression: 0
                                        * Referenced by: '<S29>/Clamping_zero'
                                        */
  int8_T Constant_Value;               /* Computed Parameter: Constant_Value
                                        * Referenced by: '<S29>/Constant'
                                        */
  int8_T Constant2_Value;              /* Computed Parameter: Constant2_Value
                                        * Referenced by: '<S29>/Constant2'
                                        */
  int8_T Constant3_Value;              /* Computed Parameter: Constant3_Value
                                        * Referenced by: '<S29>/Constant3'
                                        */
  int8_T Constant4_Value;              /* Computed Parameter: Constant4_Value
                                        * Referenced by: '<S29>/Constant4'
                                        */
};

/* Parameters (default storage) */
typedef struct TractionControl_P_e_ TractionControl_P_e;

/* Real-time Model Data Structure */
struct TractionControl_tag_RTM {
  const char_T * volatile errorStatus;
};

/* Block parameters (default storage) */
extern TractionControl_P_e TractionControl_P;

/* Block signals and states (default storage) */
extern TractionControl_DW TractionControl_DW_l;

/* External inputs (root inport signals with default storage) */
extern TractionControl_ExtU TractionControl_U;

/* External outputs (root outports fed by signals with default storage) */
extern TractionControl_ExtY TractionControl_Y;

/* Model entry point functions */
extern void TractionControl_initialize(void);
extern void TractionControl_step(void);

/* Real-time Model object */
extern TractionControl_RT_MODEL *const TractionControl_M;

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
 * '<Root>' : 'TractionControl'
 * '<S1>'   : 'TractionControl/TC_controller'
 * '<S2>'   : 'TractionControl/TC_controller/SlipActivationLogic'
 * '<S3>'   : 'TractionControl/TC_controller/TC controller'
 * '<S4>'   : 'TractionControl/TC_controller/WheelSlipCalculation'
 * '<S5>'   : 'TractionControl/TC_controller/TC controller/Anti-windup'
 * '<S6>'   : 'TractionControl/TC_controller/TC controller/D Gain'
 * '<S7>'   : 'TractionControl/TC_controller/TC controller/Filter'
 * '<S8>'   : 'TractionControl/TC_controller/TC controller/Filter ICs'
 * '<S9>'   : 'TractionControl/TC_controller/TC controller/I Gain'
 * '<S10>'  : 'TractionControl/TC_controller/TC controller/Ideal P Gain'
 * '<S11>'  : 'TractionControl/TC_controller/TC controller/Ideal P Gain Fdbk'
 * '<S12>'  : 'TractionControl/TC_controller/TC controller/Integrator'
 * '<S13>'  : 'TractionControl/TC_controller/TC controller/Integrator ICs'
 * '<S14>'  : 'TractionControl/TC_controller/TC controller/N Copy'
 * '<S15>'  : 'TractionControl/TC_controller/TC controller/N Gain'
 * '<S16>'  : 'TractionControl/TC_controller/TC controller/P Copy'
 * '<S17>'  : 'TractionControl/TC_controller/TC controller/Parallel P Gain'
 * '<S18>'  : 'TractionControl/TC_controller/TC controller/Reset Signal'
 * '<S19>'  : 'TractionControl/TC_controller/TC controller/Saturation'
 * '<S20>'  : 'TractionControl/TC_controller/TC controller/Saturation Fdbk'
 * '<S21>'  : 'TractionControl/TC_controller/TC controller/Sum'
 * '<S22>'  : 'TractionControl/TC_controller/TC controller/Sum Fdbk'
 * '<S23>'  : 'TractionControl/TC_controller/TC controller/Tracking Mode'
 * '<S24>'  : 'TractionControl/TC_controller/TC controller/Tracking Mode Sum'
 * '<S25>'  : 'TractionControl/TC_controller/TC controller/Tsamp - Integral'
 * '<S26>'  : 'TractionControl/TC_controller/TC controller/Tsamp - Ngain'
 * '<S27>'  : 'TractionControl/TC_controller/TC controller/postSat Signal'
 * '<S28>'  : 'TractionControl/TC_controller/TC controller/preSat Signal'
 * '<S29>'  : 'TractionControl/TC_controller/TC controller/Anti-windup/Disc. Clamping Parallel'
 * '<S30>'  : 'TractionControl/TC_controller/TC controller/Anti-windup/Disc. Clamping Parallel/Dead Zone'
 * '<S31>'  : 'TractionControl/TC_controller/TC controller/Anti-windup/Disc. Clamping Parallel/Dead Zone/Enabled'
 * '<S32>'  : 'TractionControl/TC_controller/TC controller/D Gain/External Parameters'
 * '<S33>'  : 'TractionControl/TC_controller/TC controller/Filter/Disc. Forward Euler Filter'
 * '<S34>'  : 'TractionControl/TC_controller/TC controller/Filter ICs/Internal IC - Filter'
 * '<S35>'  : 'TractionControl/TC_controller/TC controller/I Gain/External Parameters'
 * '<S36>'  : 'TractionControl/TC_controller/TC controller/Ideal P Gain/Passthrough'
 * '<S37>'  : 'TractionControl/TC_controller/TC controller/Ideal P Gain Fdbk/Disabled'
 * '<S38>'  : 'TractionControl/TC_controller/TC controller/Integrator/Discrete'
 * '<S39>'  : 'TractionControl/TC_controller/TC controller/Integrator ICs/Internal IC'
 * '<S40>'  : 'TractionControl/TC_controller/TC controller/N Copy/Disabled'
 * '<S41>'  : 'TractionControl/TC_controller/TC controller/N Gain/External Parameters'
 * '<S42>'  : 'TractionControl/TC_controller/TC controller/P Copy/Disabled'
 * '<S43>'  : 'TractionControl/TC_controller/TC controller/Parallel P Gain/External Parameters'
 * '<S44>'  : 'TractionControl/TC_controller/TC controller/Reset Signal/External Reset'
 * '<S45>'  : 'TractionControl/TC_controller/TC controller/Saturation/Enabled'
 * '<S46>'  : 'TractionControl/TC_controller/TC controller/Saturation Fdbk/Disabled'
 * '<S47>'  : 'TractionControl/TC_controller/TC controller/Sum/Sum_PID'
 * '<S48>'  : 'TractionControl/TC_controller/TC controller/Sum Fdbk/Disabled'
 * '<S49>'  : 'TractionControl/TC_controller/TC controller/Tracking Mode/Disabled'
 * '<S50>'  : 'TractionControl/TC_controller/TC controller/Tracking Mode Sum/Passthrough'
 * '<S51>'  : 'TractionControl/TC_controller/TC controller/Tsamp - Integral/Passthrough'
 * '<S52>'  : 'TractionControl/TC_controller/TC controller/Tsamp - Ngain/Passthrough'
 * '<S53>'  : 'TractionControl/TC_controller/TC controller/postSat Signal/Forward_Path'
 * '<S54>'  : 'TractionControl/TC_controller/TC controller/preSat Signal/Forward_Path'
 */
#endif                                 /* RTW_HEADER_TractionControl_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
