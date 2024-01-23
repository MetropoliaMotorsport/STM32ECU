/*
 * Sponsored License - for use in support of a program or activity
 * sponsored by MathWorks.  Not for government, commercial or other
 * non-sponsored organizational use.
 *
 * File: TorqueVectoring.h
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

#ifndef RTW_HEADER_TorqueVectoring_h_
#define RTW_HEADER_TorqueVectoring_h_
#ifndef TorqueVectoring_COMMON_INCLUDES_
#define TorqueVectoring_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif                                 /* TorqueVectoring_COMMON_INCLUDES_ */

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

/* Forward declaration for rtModel */
typedef struct TorqueVectoring_tag_RTM TorqueVectoring_RT_MODEL;

/* Block signals and states (default storage) for system '<Root>' */
typedef struct {
	real_T Integrator_DSTATE; /* '<S45>/Integrator' */
} TorqueVectoring_DW;

/* External inputs (root inport signals with default storage) */
typedef struct {
	real_T StrAngleDeg; /* '<Root>/StrAngleDeg' */
	real_T VehicleSpeed; /* '<Root>/VehicleSpeed' */
	real_T TorqueVectoringEnabled; /* '<Root>/TorqueVectoringEnabled' */
	real_T VehicleYawRate; /* '<Root>/VehicleYawRate' */
	real_T TorquePedal; /* '<Root>/TorquePedal' */
	boolean_T FeedbackEnabled; /* '<Root>/FeedbackEnabled' */
	boolean_T FeedForwardEnabled; /* '<Root>/FeedForwardEnabled' */
} TorqueVectoring_ExtU;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
	real_T TVFL; /* '<Root>/TVFL' */
	real_T TVFR; /* '<Root>/TVFR' */
	real_T TVRL; /* '<Root>/TVRL' */
	real_T TVRR; /* '<Root>/TVRR' */
} TorqueVectoring_ExtY;

/* Parameters (default storage) */
struct TorqueVectoring_P_e_ {
	real_T DeadzoneYawrateError; /* Variable: DeadzoneYawrateError
	 * Referenced by: '<S10>/Dead Zone'
	 */
	real_T Gr; /* Variable: Gr
	 * Referenced by:
	 *   '<S7>/Constant'
	 *   '<S7>/Constant1'
	 */
	real_T Ku; /* Variable: Ku
	 * Referenced by: '<S6>/Gain'
	 */
	real_T LowerYawMoment; /* Variable: LowerYawMoment
	 * Referenced by:
	 *   '<S5>/Yaw moment saturation'
	 *   '<S8>/Yaw moment saturation'
	 *   '<S9>/Yaw moment saturation'
	 *   '<S52>/Saturation'
	 *   '<S38>/DeadZone'
	 */
	real_T Rd; /* Variable: Rd
	 * Referenced by:
	 *   '<S7>/Gain'
	 *   '<S7>/Gain1'
	 */
	real_T Sgr; /* Variable: Sgr
	 * Referenced by: '<S6>/Gain1'
	 */
	real_T SteeringDeadzonelimit; /* Variable: SteeringDeadzonelimit
	 * Referenced by: '<S3>/Dead Zone'
	 */
	real_T TuneGains; /* Variable: TuneGains
	 * Referenced by:
	 *   '<S10>/Gain1'
	 *   '<S10>/Gain2'
	 */
	real_T UpperYawMoment; /* Variable: UpperYawMoment
	 * Referenced by:
	 *   '<S5>/Yaw moment saturation'
	 *   '<S8>/Yaw moment saturation'
	 *   '<S9>/Yaw moment saturation'
	 *   '<S52>/Saturation'
	 *   '<S38>/DeadZone'
	 */
	real_T feedforwardgain; /* Variable: feedforwardgain
	 * Referenced by: '<S9>/Constant'
	 */
	real_T g; /* Variable: g
	 * Referenced by: '<S6>/Constant2'
	 */
	real_T l; /* Variable: l
	 * Referenced by: '<S6>/Constant'
	 */
	real_T myy; /* Variable: myy
	 * Referenced by: '<S6>/Constant1'
	 */
	real_T wf; /* Variable: wf
	 * Referenced by: '<S7>/Constant1'
	 */
	real_T wr; /* Variable: wr
	 * Referenced by: '<S7>/Constant'
	 */
	real_T DiscretePIDController_InitialConditionF;
	/* Mask Parameter: DiscretePIDController_InitialConditionF
	 * Referenced by: '<S45>/Integrator'
	 */
	real_T CompareToConstant_const; /* Mask Parameter: CompareToConstant_const
	 * Referenced by: '<S2>/Constant'
	 */
	real_T CompareToConstant_torquepedal; /* Mask Parameter: CompareToConstant_torquepedal
	 * Referenced by: '<S2>/Constant'
	 */
	real_T Constant1_Value; /* Expression: 0
	 * Referenced by: '<S36>/Constant1'
	 */
	real_T Gain2_Gain; /* Expression: 180/pi
	 * Referenced by: '<S6>/Gain2'
	 */
	real_T Gain_Gain; /* Expression: 180/pi
	 * Referenced by: '<S10>/Gain'
	 */
	real_T Pgains_tableData[13];
	/* Expression: [0;55.7000000000000;59.9500000000000;120.600000000000;158.050000000000;169.950000000000;175.100000000000;165.900000000000;181.500000000000;112.650000000000;176.750000000000;175;158.900000000000]
	 * Referenced by: '<S10>/P gains'
	 */
	real_T Pgains_bp01Data[13]; /* Expression: [0;3;5;7;9;11;13;15;17;19;21;23;25]
	 * Referenced by: '<S10>/P gains'
	 */
	real_T Integrator_gainval; /* Computed Parameter: Integrator_gainval
	 * Referenced by: '<S45>/Integrator'
	 */
	real_T Gain3_Gain; /* Expression: 0.5
	 * Referenced by: '<S7>/Gain3'
	 */
	real_T Gain_Gain_d; /* Expression: 0.5
	 * Referenced by: '<S4>/Gain'
	 */
	real_T Gain2_Gain_o; /* Expression: 0.5
	 * Referenced by: '<S7>/Gain2'
	 */
	real_T Gain1_Gain; /* Expression: 0.5
	 * Referenced by: '<S4>/Gain1'
	 */
	real_T Clamping_zero_Value; /* Expression: 0
	 * Referenced by: '<S36>/Clamping_zero'
	 */
	real_T Igains_tableData[13];
	/* Expression: [0;4683;3141;3303.50000000000;3470.50000000000;2706;2433.50000000000;1907;2437;1644;3088;2655;1975]
	 * Referenced by: '<S10>/I gains'
	 */
	real_T Igains_bp01Data[13]; /* Expression: [0;3;5;7;9;11;13;15;17;19;21;23;25]
	 * Referenced by: '<S10>/I gains'
	 */
	int8_T Constant_Value; /* Computed Parameter: Constant_Value
	 * Referenced by: '<S36>/Constant'
	 */
	int8_T Constant2_Value; /* Computed Parameter: Constant2_Value
	 * Referenced by: '<S36>/Constant2'
	 */
	int8_T Constant3_Value; /* Computed Parameter: Constant3_Value
	 * Referenced by: '<S36>/Constant3'
	 */
	int8_T Constant4_Value; /* Computed Parameter: Constant4_Value
	 * Referenced by: '<S36>/Constant4'
	 */
};

/* Parameters (default storage) */
typedef struct TorqueVectoring_P_e_ TorqueVectoring_P_e;

/* Real-time Model Data Structure */
struct TorqueVectoring_tag_RTM {
	const char_T *volatile errorStatus;
};

/* Block parameters (default storage) */
extern TorqueVectoring_P_e TorqueVectoring_P;

/* Block signals and states (default storage) */
extern TorqueVectoring_DW TorqueVectoring_DW_l;

/* External inputs (root inport signals with default storage) */
extern TorqueVectoring_ExtU TorqueVectoring_U;

/* External outputs (root outports fed by signals with default storage) */
extern TorqueVectoring_ExtY TorqueVectoring_Y;

/* Model entry point functions */
extern void TorqueVectoring_initialize(void);
extern void TorqueVectoring_step(void);

/* Real-time Model object */
extern TorqueVectoring_RT_MODEL *const TorqueVectoring_M;

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
 * '<Root>' : 'TorqueVectoring'
 * '<S1>'   : 'TorqueVectoring/Subsystem'
 * '<S2>'   : 'TorqueVectoring/Subsystem/Compare To Constant'
 * '<S3>'   : 'TorqueVectoring/Subsystem/If steering angle is between -10 and 10 degrees system is off '
 * '<S4>'   : 'TorqueVectoring/Subsystem/Yaw moment distribution'
 * '<S5>'   : 'TorqueVectoring/Subsystem/Yaw rate controller'
 * '<S6>'   : 'TorqueVectoring/Subsystem/Yaw rate reference calculation'
 * '<S7>'   : 'TorqueVectoring/Subsystem/Yaw moment distribution/Lateral torque distribution'
 * '<S8>'   : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type'
 * '<S9>'   : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedforward controller type'
 * '<S10>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type'
 * '<S11>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller'
 * '<S12>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Anti-windup'
 * '<S13>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/D Gain'
 * '<S14>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Filter'
 * '<S15>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Filter ICs'
 * '<S16>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/I Gain'
 * '<S17>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Ideal P Gain'
 * '<S18>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Ideal P Gain Fdbk'
 * '<S19>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Integrator'
 * '<S20>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Integrator ICs'
 * '<S21>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/N Copy'
 * '<S22>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/N Gain'
 * '<S23>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/P Copy'
 * '<S24>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Parallel P Gain'
 * '<S25>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Reset Signal'
 * '<S26>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Saturation'
 * '<S27>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Saturation Fdbk'
 * '<S28>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Sum'
 * '<S29>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Sum Fdbk'
 * '<S30>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Tracking Mode'
 * '<S31>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Tracking Mode Sum'
 * '<S32>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Tsamp - Integral'
 * '<S33>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Tsamp - Ngain'
 * '<S34>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/postSat Signal'
 * '<S35>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/preSat Signal'
 * '<S36>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Anti-windup/Disc. Clamping Parallel'
 * '<S37>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Anti-windup/Disc. Clamping Parallel/Dead Zone'
 * '<S38>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Anti-windup/Disc. Clamping Parallel/Dead Zone/Enabled'
 * '<S39>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/D Gain/Disabled'
 * '<S40>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Filter/Disabled'
 * '<S41>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Filter ICs/Disabled'
 * '<S42>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/I Gain/External Parameters'
 * '<S43>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Ideal P Gain/Passthrough'
 * '<S44>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Ideal P Gain Fdbk/Disabled'
 * '<S45>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Integrator/Discrete'
 * '<S46>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Integrator ICs/Internal IC'
 * '<S47>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/N Copy/Disabled wSignal Specification'
 * '<S48>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/N Gain/Disabled'
 * '<S49>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/P Copy/Disabled'
 * '<S50>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Parallel P Gain/External Parameters'
 * '<S51>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Reset Signal/Disabled'
 * '<S52>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Saturation/Enabled'
 * '<S53>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Saturation Fdbk/Disabled'
 * '<S54>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Sum/Sum_PI'
 * '<S55>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Sum Fdbk/Disabled'
 * '<S56>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Tracking Mode/Disabled'
 * '<S57>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Tracking Mode Sum/Passthrough'
 * '<S58>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Tsamp - Integral/Passthrough'
 * '<S59>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/Tsamp - Ngain/Passthrough'
 * '<S60>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/postSat Signal/Forward_Path'
 * '<S61>'  : 'TorqueVectoring/Subsystem/Yaw rate controller/Feedback controller type/PI controller type/Discrete PID Controller/preSat Signal/Forward_Path'
 * '<S62>'  : 'TorqueVectoring/Subsystem/Yaw rate reference calculation/MATLAB Function'
 */
#endif                                 /* RTW_HEADER_TorqueVectoring_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
