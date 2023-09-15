/*
 * Sponsored License - for use in support of a program or activity
 * sponsored by MathWorks.  Not for government, commercial or other
 * non-sponsored organizational use.
 *
 * File: SubsystemModelReference.h
 *
 * Code generated for Simulink model 'SubsystemModelReference'.
 *
 * Model version                  : 1.26
 * Simulink Coder version         : 9.3 (R2020a) 18-Nov-2019
 * C/C++ source code generated on : Thu May 13 20:27:51 2021
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_SubsystemModelReference_h_
#define RTW_HEADER_SubsystemModelReference_h_
#include "rtwtypes.h"
#include <stddef.h>
#include <math.h>
#include <string.h>
#ifndef SubsystemModelReference_COMMON_INCLUDES_
# define SubsystemModelReference_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif                            /* SubsystemModelReference_COMMON_INCLUDES_ */

/* Model Code Variants */

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
# define rtmGetErrorStatus(rtm)        ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
# define rtmSetErrorStatus(rtm, val)   ((rtm)->errorStatus = (val))
#endif

/* Forward declaration for rtModel */
typedef struct tag_RTM RT_MODEL;

#ifndef DEFINED_TYPEDEF_FOR_struct_syoZde8SkScWoUcYRHehbB_
#define DEFINED_TYPEDEF_FOR_struct_syoZde8SkScWoUcYRHehbB_

typedef struct {
  uint8_T SimulinkDiagnostic;
  uint8_T Model[126];
  uint8_T Block[150];
  uint8_T OutOfRangeInputValue;
  uint8_T NoRuleFired;
  uint8_T EmptyOutputFuzzySet;
} struct_syoZde8SkScWoUcYRHehbB;

#endif

#ifndef DEFINED_TYPEDEF_FOR_struct_0sCPl2iL0Y7nsABAISPuqB_
#define DEFINED_TYPEDEF_FOR_struct_0sCPl2iL0Y7nsABAISPuqB_

typedef struct {
  uint8_T type[7];
  int32_T origTypeLength;
  real_T params[2];
  int32_T origParamLength;
} struct_0sCPl2iL0Y7nsABAISPuqB;

#endif

#ifndef DEFINED_TYPEDEF_FOR_struct_qSXrXoxNuDzWcXH2UjRkpE_
#define DEFINED_TYPEDEF_FOR_struct_qSXrXoxNuDzWcXH2UjRkpE_

typedef struct {
  struct_0sCPl2iL0Y7nsABAISPuqB mf[7];
  int32_T origNumMF;
} struct_qSXrXoxNuDzWcXH2UjRkpE;

#endif

#ifndef DEFINED_TYPEDEF_FOR_struct_2MXDb7W4JJZIDRf8j3cpxG_
#define DEFINED_TYPEDEF_FOR_struct_2MXDb7W4JJZIDRf8j3cpxG_

typedef struct {
  struct_0sCPl2iL0Y7nsABAISPuqB mf[4];
  int32_T origNumMF;
} struct_2MXDb7W4JJZIDRf8j3cpxG;

#endif

#ifndef DEFINED_TYPEDEF_FOR_struct_4x3q7BjkHFu27K6hVFEIuB_
#define DEFINED_TYPEDEF_FOR_struct_4x3q7BjkHFu27K6hVFEIuB_

typedef struct {
  uint8_T type[7];
  uint8_T andMethod[3];
  uint8_T orMethod[3];
  uint8_T defuzzMethod[8];
  uint8_T impMethod[3];
  uint8_T aggMethod[3];
  real_T inputRange[4];
  real_T outputRange[2];
  struct_qSXrXoxNuDzWcXH2UjRkpE inputMF[2];
  struct_2MXDb7W4JJZIDRf8j3cpxG outputMF;
  real_T antecedent[56];
  real_T consequent[28];
  real_T connection[28];
  real_T weight[28];
  int32_T numSamples;
  int32_T numInputs;
  int32_T numOutputs;
  int32_T numRules;
  int32_T numInputMFs[2];
  int32_T numCumInputMFs[2];
  int32_T numOutputMFs;
  int32_T numCumOutputMFs;
  real_T outputSamplePoints[101];
  int32_T orrSize[2];
  int32_T aggSize[2];
  int32_T irrSize[2];
  int32_T rfsSize[2];
  int32_T sumSize[2];
  int32_T inputFuzzySetType;
} struct_4x3q7BjkHFu27K6hVFEIuB;

#endif

/* Block signals and states (default storage) for system '<Root>' */
typedef struct {
  real_T DiscreteTimeIntegrator_DSTATE;/* '<S13>/Discrete-Time Integrator' */
  real_T UD_DSTATE;                    /* '<S29>/UD' */
  real_T UD_DSTATE_a;                  /* '<S41>/UD' */
  real_T UD_DSTATE_j;                  /* '<S53>/UD' */
  real_T UD_DSTATE_g;                  /* '<S65>/UD' */
} DW;

/* Constant parameters (default storage) */
typedef struct {
  /* Pooled Parameter (Expression: fis.outputSamplePoints)
   * Referenced by:
   *   '<S30>/Output Sample Points'
   *   '<S42>/Output Sample Points'
   *   '<S54>/Output Sample Points'
   *   '<S66>/Output Sample Points'
   */
  real_T pooled2[101];

  /* Expression: [0;55.7000000000000;59.9500000000000;120.600000000000;158.050000000000;169.950000000000;175.100000000000;165.900000000000;181.500000000000;112.650000000000;176.750000000000;175;158.900000000000]
   * Referenced by: '<S13>/P gains'
   */
  real_T Pgains_tableData[13];

  /* Pooled Parameter (Expression: [0;3;5;7;9;11;13;15;17;19;21;23;25])
   * Referenced by:
   *   '<S13>/I gains'
   *   '<S13>/P gains'
   */
  real_T pooled10[13];

  /* Expression: [0;4683;3141;3303.50000000000;3470.50000000000;2706;2433.50000000000;1907;2437;1644;3088;2655;1975]
   * Referenced by: '<S13>/I gains'
   */
  real_T Igains_tableData[13];
} ConstP;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T bus_Vehicle_velocity;         /* '<Root>/B_-1_-1' */
  real_T bus_Vehicle_acceleration;     /* '<Root>/B_-1_-1' */
  real_T bus_rotation_speed_FL;        /* '<Root>/B_-1_-1' */
  real_T bus_rotation_speed_FR;        /* '<Root>/B_-1_-1' */
  real_T bus_rotation_speed_RL;        /* '<Root>/B_-1_-1' */
  real_T bus_rotation_speed_RR;        /* '<Root>/B_-1_-1' */
  real_T bus_Vehicle_yaw_rate;         /* '<Root>/B_-1_-1' */
  real_T bus_Vehicle_str_ang;          /* '<Root>/B_-1_-1' */
  real_T bus_Torque_FL;                /* '<Root>/B_-1_-1' */
  real_T bus_Torque_FR;                /* '<Root>/B_-1_-1' */
  real_T bus_Torque_RL;                /* '<Root>/B_-1_-1' */
  real_T bus_Torque_RR;                /* '<Root>/B_-1_-1' */
  real_T bus_Pedal_torque_position;    /* '<Root>/B_-1_-1' */
  boolean_T bus_Traction_control_active;/* '<Root>/B_-1_-1' */
  boolean_T bus_Velocity_control_active;/* '<Root>/B_-1_-1' */
  boolean_T bus_feedback_active;       /* '<Root>/B_-1_-1' */
  boolean_T bus_feedforward_active;    /* '<Root>/B_-1_-1' */
  real_T bus_Torque_vectoring_active;  /* '<Root>/B_-1_-1' */
} ExtU;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T TV_TV_torqueFL;               /* '<Root>/TV_TV_torqueFL' */
  real_T TV_TV_torqueFR;               /* '<Root>/TV_TV_torqueFR' */
  real_T TV_TV_torqueRL;               /* '<Root>/TV_TV_torqueRL' */
  real_T TV_TV_torqueRR;               /* '<Root>/TV_TV_torqueRR' */
  real_T TCS_TCS_FL;                   /* '<Root>/TCS_TCS_FL' */
  real_T TCS_TCS_FR;                   /* '<Root>/TCS_TCS_FR' */
  real_T TCS_TCS_RL;                   /* '<Root>/TCS_TCS_RL' */
  real_T TCS_TCS_RR;                   /* '<Root>/TCS_TCS_RR' */
  real_T TCS_RPMmaxFL;                 /* '<Root>/TCS_RPMmaxFL' */
  real_T TCS_RPMmaxFR;                 /* '<Root>/TCS_RPMmaxFR' */
  real_T TCS_RPMmaxRL;                 /* '<Root>/TCS_RPMmaxRL' */
  real_T TCS_RPMmaxRR;                 /* '<Root>/TCS_RPMmaxRR' */
  boolean_T Errors_VelocityError;      /* '<Root>/Errors_VelocityError' */
  boolean_T Errors_AccelerationError;  /* '<Root>/Errors_AccelerationError' */
  boolean_T Errors_rotationErrorFL;    /* '<Root>/Errors_rotationErrorFL' */
  boolean_T Errors_rotationErrorFR;    /* '<Root>/Errors_rotationErrorFR' */
  boolean_T Errors_rotationErrorRL;    /* '<Root>/Errors_rotationErrorRL' */
  boolean_T Errors_rotationErrorRR;    /* '<Root>/Errors_rotationErrorRR' */
  boolean_T Errors_YawrateError;       /* '<Root>/Errors_YawrateError' */
  boolean_T Errors_StrAngError;        /* '<Root>/Errors_StrAngError' */
} ExtY;

/* Real-time Model Data Structure */
struct tag_RTM {
  const char_T * volatile errorStatus;
};

/* Block signals and states (default storage) */
extern DW rtDW;

/* External inputs (root inport signals with default storage) */
extern ExtU rtU;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY rtY;

/* Constant parameters (default storage) */
extern const ConstP rtConstP;

/* Model entry point functions */
extern void SubsystemModelReference_initialize(void);
extern void SubsystemModelReference_step(void);

/* Real-time Model object */
extern RT_MODEL *const rtM;

/*-
 * These blocks were eliminated from the model due to optimizations:
 *
 * Block '<S11>/Scope1' : Unused code path elimination
 * Block '<S29>/Data Type Duplicate' : Unused code path elimination
 * Block '<S28>/Scope' : Unused code path elimination
 * Block '<S41>/Data Type Duplicate' : Unused code path elimination
 * Block '<S53>/Data Type Duplicate' : Unused code path elimination
 * Block '<S65>/Data Type Duplicate' : Unused code path elimination
 * Block '<S71>/Data Type Duplicate' : Unused code path elimination
 * Block '<S72>/Data Type Duplicate' : Unused code path elimination
 * Block '<S73>/Data Type Duplicate' : Unused code path elimination
 * Block '<S74>/Data Type Duplicate' : Unused code path elimination
 * Block '<S75>/Data Type Duplicate' : Unused code path elimination
 * Block '<S76>/Data Type Duplicate' : Unused code path elimination
 * Block '<S77>/Data Type Duplicate' : Unused code path elimination
 * Block '<S78>/Data Type Duplicate' : Unused code path elimination
 * Block '<S13>/Gain1' : Eliminated nontunable gain of 1
 * Block '<S13>/Gain2' : Eliminated nontunable gain of 1
 * Block '<S30>/InputConversion' : Eliminate redundant data type conversion
 * Block '<S42>/InputConversion' : Eliminate redundant data type conversion
 * Block '<S54>/InputConversion' : Eliminate redundant data type conversion
 * Block '<S66>/InputConversion' : Eliminate redundant data type conversion
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
 * '<Root>' : 'SubsystemModelReference'
 * '<S1>'   : 'SubsystemModelReference/TV_TC_Controller'
 * '<S2>'   : 'SubsystemModelReference/TV_TC_Controller/Torque vectoring system'
 * '<S3>'   : 'SubsystemModelReference/TV_TC_Controller/Traction control system'
 * '<S4>'   : 'SubsystemModelReference/TV_TC_Controller/Traction control system1'
 * '<S5>'   : 'SubsystemModelReference/TV_TC_Controller/Torque vectoring system/Compare To Constant'
 * '<S6>'   : 'SubsystemModelReference/TV_TC_Controller/Torque vectoring system/If steering angle is between -10 and 10 degrees system is off '
 * '<S7>'   : 'SubsystemModelReference/TV_TC_Controller/Torque vectoring system/Yaw moment distribution'
 * '<S8>'   : 'SubsystemModelReference/TV_TC_Controller/Torque vectoring system/Yaw rate controller'
 * '<S9>'   : 'SubsystemModelReference/TV_TC_Controller/Torque vectoring system/Yaw rate reference calculation'
 * '<S10>'  : 'SubsystemModelReference/TV_TC_Controller/Torque vectoring system/Yaw moment distribution/Lateral torque distribution'
 * '<S11>'  : 'SubsystemModelReference/TV_TC_Controller/Torque vectoring system/Yaw rate controller/Feedback controller type'
 * '<S12>'  : 'SubsystemModelReference/TV_TC_Controller/Torque vectoring system/Yaw rate controller/Feedforward controller type'
 * '<S13>'  : 'SubsystemModelReference/TV_TC_Controller/Torque vectoring system/Yaw rate controller/Feedback controller type/PI controller type'
 * '<S14>'  : 'SubsystemModelReference/TV_TC_Controller/Torque vectoring system/Yaw rate controller/Feedback controller type/PI controller type/Clamping circuit'
 * '<S15>'  : 'SubsystemModelReference/TV_TC_Controller/Torque vectoring system/Yaw rate reference calculation/MATLAB Function'
 * '<S16>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Bus output for TC system'
 * '<S17>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Compare To Constant'
 * '<S18>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Compare To Constant1'
 * '<S19>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control FL'
 * '<S20>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control FR'
 * '<S21>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control RL'
 * '<S22>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control RR'
 * '<S23>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control FL/Fuzzy logic control'
 * '<S24>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control FL/Velocity control RPM'
 * '<S25>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control FL/Fuzzy logic control/Controller structure '
 * '<S26>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control FL/Fuzzy logic control/MATLAB Function'
 * '<S27>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control FL/Fuzzy logic control/Slip thresholds'
 * '<S28>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control FL/Fuzzy logic control/Wheel slip calculation FL'
 * '<S29>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control FL/Fuzzy logic control/Controller structure /Discrete Derivative'
 * '<S30>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control FL/Fuzzy logic control/Controller structure /Fuzzy Logic  Controller'
 * '<S31>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control FL/Fuzzy logic control/Controller structure /Fuzzy Logic  Controller/Defuzzify Outputs'
 * '<S32>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control FL/Fuzzy logic control/Controller structure /Fuzzy Logic  Controller/Evaluate Rule Antecedents'
 * '<S33>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control FL/Fuzzy logic control/Controller structure /Fuzzy Logic  Controller/Evaluate Rule Consequents'
 * '<S34>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control FL/Velocity control RPM/Reference omega speed '
 * '<S35>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control FR/Fuzzy logic control'
 * '<S36>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control FR/Velocity control RPM'
 * '<S37>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control FR/Fuzzy logic control/Controller structure '
 * '<S38>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control FR/Fuzzy logic control/MATLAB Function'
 * '<S39>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control FR/Fuzzy logic control/Slip thresholds'
 * '<S40>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control FR/Fuzzy logic control/Wheel slip calculation FR'
 * '<S41>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control FR/Fuzzy logic control/Controller structure /Discrete Derivative'
 * '<S42>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control FR/Fuzzy logic control/Controller structure /Fuzzy Logic  Controller'
 * '<S43>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control FR/Fuzzy logic control/Controller structure /Fuzzy Logic  Controller/Defuzzify Outputs'
 * '<S44>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control FR/Fuzzy logic control/Controller structure /Fuzzy Logic  Controller/Evaluate Rule Antecedents'
 * '<S45>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control FR/Fuzzy logic control/Controller structure /Fuzzy Logic  Controller/Evaluate Rule Consequents'
 * '<S46>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control FR/Velocity control RPM/Reference omega speed '
 * '<S47>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control RL/Fuzzy logic control'
 * '<S48>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control RL/Velocity control RPM'
 * '<S49>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control RL/Fuzzy logic control/Controller structure '
 * '<S50>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control RL/Fuzzy logic control/MATLAB Function'
 * '<S51>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control RL/Fuzzy logic control/Slip thresholds'
 * '<S52>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control RL/Fuzzy logic control/Wheel slip calculation RL'
 * '<S53>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control RL/Fuzzy logic control/Controller structure /Discrete Derivative'
 * '<S54>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control RL/Fuzzy logic control/Controller structure /Fuzzy Logic  Controller'
 * '<S55>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control RL/Fuzzy logic control/Controller structure /Fuzzy Logic  Controller/Defuzzify Outputs'
 * '<S56>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control RL/Fuzzy logic control/Controller structure /Fuzzy Logic  Controller/Evaluate Rule Antecedents'
 * '<S57>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control RL/Fuzzy logic control/Controller structure /Fuzzy Logic  Controller/Evaluate Rule Consequents'
 * '<S58>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control RL/Velocity control RPM/Reference omega speed '
 * '<S59>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control RR/Fuzzy logic control'
 * '<S60>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control RR/Velocity control RPM'
 * '<S61>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control RR/Fuzzy logic control/Controller structure '
 * '<S62>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control RR/Fuzzy logic control/MATLAB Function'
 * '<S63>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control RR/Fuzzy logic control/Slip thresholds'
 * '<S64>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control RR/Fuzzy logic control/Wheel slip calculation RR'
 * '<S65>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control RR/Fuzzy logic control/Controller structure /Discrete Derivative'
 * '<S66>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control RR/Fuzzy logic control/Controller structure /Fuzzy Logic  Controller'
 * '<S67>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control RR/Fuzzy logic control/Controller structure /Fuzzy Logic  Controller/Defuzzify Outputs'
 * '<S68>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control RR/Fuzzy logic control/Controller structure /Fuzzy Logic  Controller/Evaluate Rule Antecedents'
 * '<S69>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control RR/Fuzzy logic control/Controller structure /Fuzzy Logic  Controller/Evaluate Rule Consequents'
 * '<S70>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system/Traction control RR/Velocity control RPM/Reference omega speed '
 * '<S71>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system1/Vehicle acceleration '
 * '<S72>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system1/Vehicle velocity'
 * '<S73>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system1/rotation speed front left '
 * '<S74>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system1/rotation speed front right'
 * '<S75>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system1/rotation speed rear left'
 * '<S76>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system1/rotation speed rear right'
 * '<S77>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system1/steering wheel angle'
 * '<S78>'  : 'SubsystemModelReference/TV_TC_Controller/Traction control system1/vehicle yaw rate'
 */
#endif                               /* RTW_HEADER_SubsystemModelReference_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
