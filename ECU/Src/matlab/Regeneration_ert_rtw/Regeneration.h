/*
 * Sponsored License - for use in support of a program or activity
 * sponsored by MathWorks.  Not for government, commercial or other
 * non-sponsored organizational use.
 *
 * File: Regeneration.h
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

#ifndef RTW_HEADER_Regeneration_h_
#define RTW_HEADER_Regeneration_h_
#ifndef Regeneration_COMMON_INCLUDES_
#define Regeneration_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif                                 /* Regeneration_COMMON_INCLUDES_ */

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

/* Forward declaration for rtModel */
typedef struct Regeneration_tag_RTM Regeneration_RT_MODEL;

/* Custom Type definition for MATLAB Function: '<S4>/check_regen' */
#ifndef struct_tag_sRNLN9nHXIPwPie46f9sYR
#define struct_tag_sRNLN9nHXIPwPie46f9sYR

struct tag_sRNLN9nHXIPwPie46f9sYR
{
  real_T regen_control;
  real_T energy;
};

#endif                                 /* struct_tag_sRNLN9nHXIPwPie46f9sYR */

#ifndef typedef_Regeneration_sRNLN9nHXIPwPie46f9sYR
#define typedef_Regeneration_sRNLN9nHXIPwPie46f9sYR

typedef struct tag_sRNLN9nHXIPwPie46f9sYR Regeneration_sRNLN9nHXIPwPie46f9sYR;

#endif                         /* typedef_Regeneration_sRNLN9nHXIPwPie46f9sYR */

/* Block signals and states (default storage) for system '<Root>' */
typedef struct {
  Regeneration_sRNLN9nHXIPwPie46f9sYR isActive;/* '<S4>/check_regen' */
  real_T u_start;                      /* '<S12>/Chart' */
  boolean_T isActive_not_empty;        /* '<S4>/check_regen' */
  boolean_T doneDoubleBufferReInit;    /* '<S12>/Chart' */
} Regeneration_DW;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T Torque_pedal;                 /* '<Root>/Torque_pedal' */
  real_T U_cell_max_mV;                /* '<Root>/U_cell_max_mV' */
  real_T U_cell_max_possible_mV;       /* '<Root>/U_cell_max_possible_mV' */
  real_T Slip_FL;                      /* '<Root>/Slip_FL' */
  real_T Slip_FR;                      /* '<Root>/Slip_FR' */
  real_T Slip_RL;                      /* '<Root>/Slip_RL' */
  real_T Slip_RR;                      /* '<Root>/Slip_RR' */
  real_T select_operating_mode;        /* '<Root>/select_operating_mode' */
  real_T regen_optimizer_on;           /* '<Root>/regen_optimizer_on' */
  real_T brake_pedal_position;         /* '<Root>/brake_pedal_position' */
  real_T max_regen_torque;             /* '<Root>/max_regen_torque' */
  real_T speed;                        /* '<Root>/speed' */
  real_T static_P_min_lim;             /* '<Root>/static_P_min_lim' */
  real_T pedal_rege_thresh_endurance_max;
                                  /* '<Root>/pedal_rege_thresh_endurance_max' */
  real_T IVT_WhCalculated;             /* '<Root>/IVT_WhCalculated' */
} Regeneration_ExtU;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T regenFL;                      /* '<Root>/regenFL' */
  real_T regenFR;                      /* '<Root>/regenFR' */
  real_T regenRL;                      /* '<Root>/regenRL' */
  real_T regenRR;                      /* '<Root>/regenRR' */
  real_T MotorRegenPowerLimNegkW;      /* '<Root>/MotorRegenPowerLimNegkW' */
  real_T SOC;                          /* '<Root>/SOC' */
} Regeneration_ExtY;

/* Parameters (default storage) */
struct Regeneration_P_e_ {
  real_T Deactivateregenifspeedbelow9kmh_const;
                        /* Mask Parameter: Deactivateregenifspeedbelow9kmh_const
                         * Referenced by: '<S11>/Constant'
                         */
  real_T Constant_Value;               /* Expression: 0
                                        * Referenced by: '<S10>/Constant'
                                        */
  real_T Mappedaltoregen_tableData[26];
  /* Expression: [0 -1 -2 -3 -4 -5 -6 -7 -8 -9 -10 -11 -12 -13 -14 -15 -16 -17 -18 -19 -20 -21 -22 -23 -24 -25]
   * Referenced by: '<S10>/Map pedal to regen '
   */
  real_T Mappedaltoregen_bp01Data[26]; /* Expression: linspace(0,80,26)
                                        * Referenced by: '<S10>/Map pedal to regen '
                                        */
  real_T Constant_Value_j;             /* Expression: 0
                                        * Referenced by: '<S13>/Constant'
                                        */
  real_T Proportionaltomaximumregenpower_Gain;/* Expression: -80000
                                               * Referenced by: '<S4>/Proportional to maximum regen power'
                                               */
  real_T Safetyoffset_Bias;            /* Expression: 7000
                                        * Referenced by: '<S4>/Safety offset'
                                        */
  real_T Saturatetomaxregenof100kW_UpperSat;/* Expression: 0
                                             * Referenced by: '<S4>/Saturate to max regen of -100 kW'
                                             */
  real_T Saturatetomaxregenof100kW_LowerSat;/* Expression: -100000
                                             * Referenced by: '<S4>/Saturate to max regen of -100 kW'
                                             */
  real_T Maxpermissiblenegativeslip_Value;/* Expression: 0.25
                                           * Referenced by: '<S5>/Max permissible negative slip'
                                           */
  real_T Constant_Value_f;             /* Expression: 0
                                        * Referenced by: '<S18>/Constant'
                                        */
  real_T Saturation_UpperSat;          /* Expression: 0
                                        * Referenced by: '<S5>/Saturation'
                                        */
  real_T Saturation_LowerSat;          /* Expression: -1
                                        * Referenced by: '<S5>/Saturation'
                                        */
  real_T Constant_Value_i;             /* Expression: 0
                                        * Referenced by: '<S1>/Constant'
                                        */
  real_T tokWh_Gain;                   /* Expression: 1/1000
                                        * Referenced by: '<S4>/to kWh'
                                        */
  real_T maxkWh_Value;                 /* Expression: 6.98
                                        * Referenced by: '<S3>/max kWh'
                                        */
  real_T Gain_Gain;                    /* Expression: 100
                                        * Referenced by: '<S3>/Gain'
                                        */
  real_T Criticalvoltagedifference_Value;/* Expression: 200
                                          * Referenced by: '<S4>/Critical voltage difference'
                                          */
  real_T Switch_Threshold;             /* Expression: 0
                                        * Referenced by: '<S4>/Switch'
                                        */
  real_T Gain_Gain_m;                  /* Expression: 1/4
                                        * Referenced by: '<S1>/Gain'
                                        */
};

/* Parameters (default storage) */
typedef struct Regeneration_P_e_ Regeneration_P_e;

/* Real-time Model Data Structure */
struct Regeneration_tag_RTM {
  const char_T * volatile errorStatus;
};

/* Block parameters (default storage) */
extern Regeneration_P_e Regeneration_P;

/* Block signals and states (default storage) */
extern Regeneration_DW Regeneration_DW_l;

/* External inputs (root inport signals with default storage) */
extern Regeneration_ExtU Regeneration_U;

/* External outputs (root outports fed by signals with default storage) */
extern Regeneration_ExtY Regeneration_Y;

/* Model entry point functions */
extern void Regeneration_initialize(void);
extern void Regeneration_step(void);

/* Real-time Model object */
extern Regeneration_RT_MODEL *const Regeneration_M;

/*-
 * These blocks were eliminated from the model due to optimizations:
 *
 * Block '<S14>/Data Type Duplicate' : Unused code path elimination
 * Block '<S14>/Data Type Propagation' : Unused code path elimination
 * Block '<S12>/Scope' : Unused code path elimination
 * Block '<S12>/Scope1' : Unused code path elimination
 * Block '<S12>/Scope2' : Unused code path elimination
 * Block '<S5>/Scope' : Unused code path elimination
 * Block '<S5>/Scope1' : Unused code path elimination
 * Block '<S5>/Scope2' : Unused code path elimination
 * Block '<S6>/Data Type Duplicate' : Unused code path elimination
 * Block '<S6>/Data Type Propagation' : Unused code path elimination
 * Block '<S7>/Data Type Duplicate' : Unused code path elimination
 * Block '<S7>/Data Type Propagation' : Unused code path elimination
 * Block '<S8>/Data Type Duplicate' : Unused code path elimination
 * Block '<S8>/Data Type Propagation' : Unused code path elimination
 * Block '<S9>/Data Type Duplicate' : Unused code path elimination
 * Block '<S9>/Data Type Propagation' : Unused code path elimination
 * Block '<S1>/Scope' : Unused code path elimination
 */

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
 * '<Root>' : 'Regeneration'
 * '<S1>'   : 'Regeneration/Regeneration'
 * '<S2>'   : 'Regeneration/Regeneration/Activation logic'
 * '<S3>'   : 'Regeneration/Regeneration/BatteryPercentage'
 * '<S4>'   : 'Regeneration/Regeneration/Regen_limit'
 * '<S5>'   : 'Regeneration/Regeneration/Regen_optimizer'
 * '<S6>'   : 'Regeneration/Regeneration/Saturation Dynamic'
 * '<S7>'   : 'Regeneration/Regeneration/Saturation Dynamic1'
 * '<S8>'   : 'Regeneration/Regeneration/Saturation Dynamic2'
 * '<S9>'   : 'Regeneration/Regeneration/Saturation Dynamic3'
 * '<S10>'  : 'Regeneration/Regeneration/Activation logic/Autocross mode'
 * '<S11>'  : 'Regeneration/Regeneration/Activation logic/Deactivate regen if speed below 9 km//h'
 * '<S12>'  : 'Regeneration/Regeneration/Activation logic/Endurance mode'
 * '<S13>'  : 'Regeneration/Regeneration/Activation logic/No regeneration'
 * '<S14>'  : 'Regeneration/Regeneration/Activation logic/Autocross mode/Saturation Dynamic'
 * '<S15>'  : 'Regeneration/Regeneration/Activation logic/Endurance mode/Chart'
 * '<S16>'  : 'Regeneration/Regeneration/Activation logic/Endurance mode/MATLAB Function'
 * '<S17>'  : 'Regeneration/Regeneration/Regen_limit/check_regen'
 * '<S18>'  : 'Regeneration/Regeneration/Regen_optimizer/Bang-bang controller'
 */
#endif                                 /* RTW_HEADER_Regeneration_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
