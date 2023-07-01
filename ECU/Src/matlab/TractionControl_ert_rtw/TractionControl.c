/*
 * Sponsored License - for use in support of a program or activity
 * sponsored by MathWorks.  Not for government, commercial or other
 * non-sponsored organizational use.
 *
 * File: TractionControl.c
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

#include "TractionControl.h"
#include <math.h>
#include "rtwtypes.h"
#include <stddef.h>
#define NumBitsPerChar                 8U

/* Block signals and states (default storage) */
TractionControl_DW TractionControl_DW_l;

/* External inputs (root inport signals with default storage) */
TractionControl_ExtU TractionControl_U;

/* External outputs (root outports fed by signals with default storage) */
TractionControl_ExtY TractionControl_Y;

/* Real-time model */
static TractionControl_RT_MODEL TractionControl_M_;
TractionControl_RT_MODEL *const TractionControl_M = &TractionControl_M_;
static real_T rtGetInf(void);
static real32_T rtGetInfF(void);
static real_T rtGetMinusInf(void);
static real32_T rtGetMinusInfF(void);

#define NOT_USING_NONFINITE_LITERALS   1

extern real_T rtInf;
extern real_T rtMinusInf;
extern real_T rtNaN;
extern real32_T rtInfF;
extern real32_T rtMinusInfF;
extern real32_T rtNaNF;
static void rt_InitInfAndNaN(size_t realSize);
static boolean_T rtIsInf(real_T value);
static boolean_T rtIsInfF(real32_T value);
static boolean_T rtIsNaN(real_T value);
static boolean_T rtIsNaNF(real32_T value);
typedef struct {
  struct {
    uint32_T wordH;
    uint32_T wordL;
  } words;
} BigEndianIEEEDouble;

typedef struct {
  struct {
    uint32_T wordL;
    uint32_T wordH;
  } words;
} LittleEndianIEEEDouble;

typedef struct {
  union {
    real32_T wordLreal;
    uint32_T wordLuint;
  } wordL;
} IEEESingle;

real_T rtInf;
real_T rtMinusInf;
real_T rtNaN;
real32_T rtInfF;
real32_T rtMinusInfF;
real32_T rtNaNF;
static real_T rtGetNaN(void);
static real32_T rtGetNaNF(void);

/*
 * Initialize rtInf needed by the generated code.
 * Inf is initialized as non-signaling. Assumes IEEE.
 */
static real_T rtGetInf(void)
{
  size_t bitsPerReal = sizeof(real_T) * (NumBitsPerChar);
  real_T inf = 0.0;
  if (bitsPerReal == 32U) {
    inf = rtGetInfF();
  } else {
    union {
      LittleEndianIEEEDouble bitVal;
      real_T fltVal;
    } tmpVal;

    tmpVal.bitVal.words.wordH = 0x7FF00000U;
    tmpVal.bitVal.words.wordL = 0x00000000U;
    inf = tmpVal.fltVal;
  }

  return inf;
}

/*
 * Initialize rtInfF needed by the generated code.
 * Inf is initialized as non-signaling. Assumes IEEE.
 */
static real32_T rtGetInfF(void)
{
  IEEESingle infF;
  infF.wordL.wordLuint = 0x7F800000U;
  return infF.wordL.wordLreal;
}

/*
 * Initialize rtMinusInf needed by the generated code.
 * Inf is initialized as non-signaling. Assumes IEEE.
 */
static real_T rtGetMinusInf(void)
{
  size_t bitsPerReal = sizeof(real_T) * (NumBitsPerChar);
  real_T minf = 0.0;
  if (bitsPerReal == 32U) {
    minf = rtGetMinusInfF();
  } else {
    union {
      LittleEndianIEEEDouble bitVal;
      real_T fltVal;
    } tmpVal;

    tmpVal.bitVal.words.wordH = 0xFFF00000U;
    tmpVal.bitVal.words.wordL = 0x00000000U;
    minf = tmpVal.fltVal;
  }

  return minf;
}

/*
 * Initialize rtMinusInfF needed by the generated code.
 * Inf is initialized as non-signaling. Assumes IEEE.
 */
static real32_T rtGetMinusInfF(void)
{
  IEEESingle minfF;
  minfF.wordL.wordLuint = 0xFF800000U;
  return minfF.wordL.wordLreal;
}

/* Model step function */
void TractionControl_step(void)
{
  real_T TractionControl_rtb_CurrentSlip_idx_0;
  real_T TractionControl_rtb_CurrentSlip_idx_1;
  real_T TractionControl_rtb_CurrentSlip_idx_2;
  real_T TractionControl_rtb_CurrentSlip_idx_3;
  real_T TractionControl_u0;
  real_T TractionControl_y;
  real_T rtb_DeadZone_j;
  real_T rtb_NProdOut_l;
  int32_T rtb_TC_activated_c;
  int8_T TractionControl_tmp;
  int8_T TractionControl_tmp_0;

  /* Outputs for Atomic SubSystem: '<Root>/TC_controller' */
  /* MATLAB Function: '<S1>/WheelSlipCalculation' incorporates:
   *  Inport: '<Root>/VehicleSpeed'
   *  Inport: '<Root>/WheelRotVelocityFL'
   *  Inport: '<Root>/WheelRotVelocityFR'
   *  Inport: '<Root>/WheelRotVelocityRL'
   *  Inport: '<Root>/WheelRotVelocityRR'
   */
  TractionControl_rtb_CurrentSlip_idx_0 = TractionControl_P.TireRadius *
    TractionControl_U.WheelRotVelocityFL;
  TractionControl_rtb_CurrentSlip_idx_1 = TractionControl_P.TireRadius *
    TractionControl_U.WheelRotVelocityFR;
  TractionControl_rtb_CurrentSlip_idx_2 = TractionControl_P.TireRadius *
    TractionControl_U.WheelRotVelocityRL;
  TractionControl_rtb_CurrentSlip_idx_3 = TractionControl_P.TireRadius *
    TractionControl_U.WheelRotVelocityRR;
  if (TractionControl_U.VehicleSpeed == 0.0) {
    TractionControl_rtb_CurrentSlip_idx_0 =
      (TractionControl_rtb_CurrentSlip_idx_0 - TractionControl_U.VehicleSpeed) /
      (fmax(TractionControl_U.VehicleSpeed,
            TractionControl_rtb_CurrentSlip_idx_0) + 2.2204460492503131E-16);
    TractionControl_rtb_CurrentSlip_idx_1 =
      (TractionControl_rtb_CurrentSlip_idx_1 - TractionControl_U.VehicleSpeed) /
      (fmax(TractionControl_U.VehicleSpeed,
            TractionControl_rtb_CurrentSlip_idx_1) + 2.2204460492503131E-16);
    TractionControl_rtb_CurrentSlip_idx_2 =
      (TractionControl_rtb_CurrentSlip_idx_2 - TractionControl_U.VehicleSpeed) /
      (fmax(TractionControl_U.VehicleSpeed,
            TractionControl_rtb_CurrentSlip_idx_2) + 2.2204460492503131E-16);
    TractionControl_rtb_CurrentSlip_idx_3 =
      (TractionControl_rtb_CurrentSlip_idx_3 - TractionControl_U.VehicleSpeed) /
      (fmax(TractionControl_U.VehicleSpeed,
            TractionControl_rtb_CurrentSlip_idx_3) + 2.2204460492503131E-16);
  } else {
    TractionControl_rtb_CurrentSlip_idx_0 =
      (TractionControl_rtb_CurrentSlip_idx_0 - TractionControl_U.VehicleSpeed) /
      fmax(TractionControl_U.VehicleSpeed, TractionControl_rtb_CurrentSlip_idx_0);
    TractionControl_rtb_CurrentSlip_idx_1 =
      (TractionControl_rtb_CurrentSlip_idx_1 - TractionControl_U.VehicleSpeed) /
      fmax(TractionControl_U.VehicleSpeed, TractionControl_rtb_CurrentSlip_idx_1);
    TractionControl_rtb_CurrentSlip_idx_2 =
      (TractionControl_rtb_CurrentSlip_idx_2 - TractionControl_U.VehicleSpeed) /
      fmax(TractionControl_U.VehicleSpeed, TractionControl_rtb_CurrentSlip_idx_2);
    TractionControl_rtb_CurrentSlip_idx_3 =
      (TractionControl_rtb_CurrentSlip_idx_3 - TractionControl_U.VehicleSpeed) /
      fmax(TractionControl_U.VehicleSpeed, TractionControl_rtb_CurrentSlip_idx_3);
  }

  /* End of MATLAB Function: '<S1>/WheelSlipCalculation' */

  /* Sum: '<S1>/Sum' incorporates:
   *  Inport: '<Root>/UpperSlipThreshold'
   */
  TractionControl_u0 = TractionControl_rtb_CurrentSlip_idx_0 -
    TractionControl_U.Desiredwheelslip;

  /* Saturate: '<S1>/Limit to positive' */
  if (TractionControl_u0 > TractionControl_P.Limittopositive_UpperSat) {
    TractionControl_u0 = TractionControl_P.Limittopositive_UpperSat;
  } else if (TractionControl_u0 < TractionControl_P.Limittopositive_LowerSat) {
    TractionControl_u0 = TractionControl_P.Limittopositive_LowerSat;
  }

  /* MATLAB Function: '<S1>/SlipActivationLogic' incorporates:
   *  Inport: '<Root>/BrakePressure'
   *  Inport: '<Root>/TC_enabled'
   *  Inport: '<Root>/UpperSlipThreshold'
   *  Inport: '<Root>/VehicleSpeed'
   *  Sum: '<S1>/Sum'
   */
  if ((TractionControl_U.TC_enabled == 1.0) && (TractionControl_U.VehicleSpeed >
       3.0) && (TractionControl_U.BrakePressure < 0.1)) {
    rtb_TC_activated_c = (TractionControl_rtb_CurrentSlip_idx_0 >
                          TractionControl_U.Desiredwheelslip);
  } else {
    rtb_TC_activated_c = 0;
  }

  /* DiscreteIntegrator: '<S38>/Integrator' */
  if ((rtb_TC_activated_c <= 0) &&
      (TractionControl_DW_l.Integrator_PrevResetState[0] == 1)) {
    TractionControl_DW_l.Integrator_DSTATE[0] =
      TractionControl_P.TCcontroller_InitialConditionForIntegra;
  }

  /* DiscreteIntegrator: '<S33>/Filter' */
  if ((rtb_TC_activated_c <= 0) && (TractionControl_DW_l.Filter_PrevResetState[0]
       == 1)) {
    TractionControl_DW_l.Filter_DSTATE[0] =
      TractionControl_P.TCcontroller_InitialConditionForFilter;
  }

  /* Product: '<S41>/NProd Out' incorporates:
   *  Constant: '<S1>/N'
   *  DiscreteIntegrator: '<S33>/Filter'
   *  Inport: '<Root>/Kd'
   *  Product: '<S32>/DProd Out'
   *  Sum: '<S33>/SumD'
   */
  rtb_NProdOut_l = (TractionControl_u0 * TractionControl_U.Derivativegain -
                    TractionControl_DW_l.Filter_DSTATE[0]) *
    TractionControl_P.N_Value;

  /* Sum: '<S47>/Sum' incorporates:
   *  DiscreteIntegrator: '<S38>/Integrator'
   *  Inport: '<Root>/Kp'
   *  Product: '<S43>/PProd Out'
   */
  rtb_DeadZone_j = (TractionControl_u0 * TractionControl_U.Proportionalgain +
                    TractionControl_DW_l.Integrator_DSTATE[0]) + rtb_NProdOut_l;

  /* Saturate: '<S45>/Saturation' incorporates:
   *  DeadZone: '<S31>/DeadZone'
   */
  if (rtb_DeadZone_j > TractionControl_P.TCcontroller_UpperSaturationLimit) {
    TractionControl_y = TractionControl_P.TCcontroller_UpperSaturationLimit;
    rtb_DeadZone_j -= TractionControl_P.TCcontroller_UpperSaturationLimit;
  } else {
    if (rtb_DeadZone_j < TractionControl_P.TCcontroller_LowerSaturationLimit) {
      TractionControl_y = TractionControl_P.TCcontroller_LowerSaturationLimit;
    } else {
      TractionControl_y = rtb_DeadZone_j;
    }

    if (rtb_DeadZone_j >= TractionControl_P.TCcontroller_LowerSaturationLimit) {
      rtb_DeadZone_j = 0.0;
    } else {
      rtb_DeadZone_j -= TractionControl_P.TCcontroller_LowerSaturationLimit;
    }
  }

  /* Outport: '<Root>/TC_FL' incorporates:
   *  Product: '<S1>/Product'
   *  Saturate: '<S45>/Saturation'
   */
  TractionControl_Y.TC_FL = (real_T)rtb_TC_activated_c * TractionControl_y;

  /* Product: '<S35>/IProd Out' incorporates:
   *  Inport: '<Root>/Ki'
   */
  TractionControl_u0 *= TractionControl_U.Integralgain;

  /* Switch: '<S29>/Switch1' incorporates:
   *  Constant: '<S29>/Clamping_zero'
   *  Constant: '<S29>/Constant'
   *  Constant: '<S29>/Constant2'
   *  RelationalOperator: '<S29>/fix for DT propagation issue'
   */
  if (rtb_DeadZone_j > TractionControl_P.Clamping_zero_Value) {
    TractionControl_tmp = TractionControl_P.Constant_Value;
  } else {
    TractionControl_tmp = TractionControl_P.Constant2_Value;
  }

  /* Switch: '<S29>/Switch2' incorporates:
   *  Constant: '<S29>/Clamping_zero'
   *  Constant: '<S29>/Constant3'
   *  Constant: '<S29>/Constant4'
   *  RelationalOperator: '<S29>/fix for DT propagation issue1'
   */
  if (TractionControl_u0 > TractionControl_P.Clamping_zero_Value) {
    TractionControl_tmp_0 = TractionControl_P.Constant3_Value;
  } else {
    TractionControl_tmp_0 = TractionControl_P.Constant4_Value;
  }

  /* Switch: '<S29>/Switch' incorporates:
   *  Constant: '<S29>/Clamping_zero'
   *  Constant: '<S29>/Constant1'
   *  Logic: '<S29>/AND3'
   *  RelationalOperator: '<S29>/Equal1'
   *  RelationalOperator: '<S29>/Relational Operator'
   *  Switch: '<S29>/Switch1'
   *  Switch: '<S29>/Switch2'
   */
  if ((TractionControl_P.Clamping_zero_Value != rtb_DeadZone_j) &&
      (TractionControl_tmp == TractionControl_tmp_0)) {
    TractionControl_u0 = TractionControl_P.Constant1_Value;
  }

  /* Update for DiscreteIntegrator: '<S38>/Integrator' incorporates:
   *  DiscreteIntegrator: '<S33>/Filter'
   *  Switch: '<S29>/Switch'
   */
  TractionControl_DW_l.Integrator_DSTATE[0] +=
    TractionControl_P.Integrator_gainval * TractionControl_u0;
  if (rtb_TC_activated_c > 0) {
    TractionControl_DW_l.Integrator_PrevResetState[0] = 1;
    TractionControl_DW_l.Filter_PrevResetState[0] = 1;
  } else {
    TractionControl_DW_l.Integrator_PrevResetState[0] = 0;
    TractionControl_DW_l.Filter_PrevResetState[0] = 0;
  }

  /* Update for DiscreteIntegrator: '<S33>/Filter' */
  TractionControl_DW_l.Filter_DSTATE[0] += TractionControl_P.Filter_gainval *
    rtb_NProdOut_l;

  /* Sum: '<S1>/Sum' incorporates:
   *  Inport: '<Root>/UpperSlipThreshold'
   */
  TractionControl_u0 = TractionControl_rtb_CurrentSlip_idx_1 -
    TractionControl_U.Desiredwheelslip;

  /* Saturate: '<S1>/Limit to positive' */
  if (TractionControl_u0 > TractionControl_P.Limittopositive_UpperSat) {
    TractionControl_u0 = TractionControl_P.Limittopositive_UpperSat;
  } else if (TractionControl_u0 < TractionControl_P.Limittopositive_LowerSat) {
    TractionControl_u0 = TractionControl_P.Limittopositive_LowerSat;
  }

  /* MATLAB Function: '<S1>/SlipActivationLogic' incorporates:
   *  Inport: '<Root>/BrakePressure'
   *  Inport: '<Root>/TC_enabled'
   *  Inport: '<Root>/UpperSlipThreshold'
   *  Inport: '<Root>/VehicleSpeed'
   *  Sum: '<S1>/Sum'
   */
  if ((TractionControl_U.TC_enabled == 1.0) && (TractionControl_U.VehicleSpeed >
       3.0) && (TractionControl_U.BrakePressure < 0.1)) {
    rtb_TC_activated_c = (TractionControl_rtb_CurrentSlip_idx_1 >
                          TractionControl_U.Desiredwheelslip);
  } else {
    rtb_TC_activated_c = 0;
  }

  /* DiscreteIntegrator: '<S38>/Integrator' */
  if ((rtb_TC_activated_c <= 0) &&
      (TractionControl_DW_l.Integrator_PrevResetState[1] == 1)) {
    TractionControl_DW_l.Integrator_DSTATE[1] =
      TractionControl_P.TCcontroller_InitialConditionForIntegra;
  }

  /* DiscreteIntegrator: '<S33>/Filter' */
  if ((rtb_TC_activated_c <= 0) && (TractionControl_DW_l.Filter_PrevResetState[1]
       == 1)) {
    TractionControl_DW_l.Filter_DSTATE[1] =
      TractionControl_P.TCcontroller_InitialConditionForFilter;
  }

  /* Product: '<S41>/NProd Out' incorporates:
   *  Constant: '<S1>/N'
   *  DiscreteIntegrator: '<S33>/Filter'
   *  Inport: '<Root>/Kd'
   *  Product: '<S32>/DProd Out'
   *  Sum: '<S33>/SumD'
   */
  rtb_NProdOut_l = (TractionControl_u0 * TractionControl_U.Derivativegain -
                    TractionControl_DW_l.Filter_DSTATE[1]) *
    TractionControl_P.N_Value;

  /* Sum: '<S47>/Sum' incorporates:
   *  DiscreteIntegrator: '<S38>/Integrator'
   *  Inport: '<Root>/Kp'
   *  Product: '<S43>/PProd Out'
   */
  rtb_DeadZone_j = (TractionControl_u0 * TractionControl_U.Proportionalgain +
                    TractionControl_DW_l.Integrator_DSTATE[1]) + rtb_NProdOut_l;

  /* Saturate: '<S45>/Saturation' incorporates:
   *  DeadZone: '<S31>/DeadZone'
   */
  if (rtb_DeadZone_j > TractionControl_P.TCcontroller_UpperSaturationLimit) {
    TractionControl_y = TractionControl_P.TCcontroller_UpperSaturationLimit;
    rtb_DeadZone_j -= TractionControl_P.TCcontroller_UpperSaturationLimit;
  } else {
    if (rtb_DeadZone_j < TractionControl_P.TCcontroller_LowerSaturationLimit) {
      TractionControl_y = TractionControl_P.TCcontroller_LowerSaturationLimit;
    } else {
      TractionControl_y = rtb_DeadZone_j;
    }

    if (rtb_DeadZone_j >= TractionControl_P.TCcontroller_LowerSaturationLimit) {
      rtb_DeadZone_j = 0.0;
    } else {
      rtb_DeadZone_j -= TractionControl_P.TCcontroller_LowerSaturationLimit;
    }
  }

  /* Outport: '<Root>/TC_FR' incorporates:
   *  Product: '<S1>/Product'
   *  Saturate: '<S45>/Saturation'
   */
  TractionControl_Y.TC_FR = (real_T)rtb_TC_activated_c * TractionControl_y;

  /* Product: '<S35>/IProd Out' incorporates:
   *  Inport: '<Root>/Ki'
   */
  TractionControl_u0 *= TractionControl_U.Integralgain;

  /* Switch: '<S29>/Switch1' incorporates:
   *  Constant: '<S29>/Clamping_zero'
   *  Constant: '<S29>/Constant'
   *  Constant: '<S29>/Constant2'
   *  RelationalOperator: '<S29>/fix for DT propagation issue'
   */
  if (rtb_DeadZone_j > TractionControl_P.Clamping_zero_Value) {
    TractionControl_tmp = TractionControl_P.Constant_Value;
  } else {
    TractionControl_tmp = TractionControl_P.Constant2_Value;
  }

  /* Switch: '<S29>/Switch2' incorporates:
   *  Constant: '<S29>/Clamping_zero'
   *  Constant: '<S29>/Constant3'
   *  Constant: '<S29>/Constant4'
   *  RelationalOperator: '<S29>/fix for DT propagation issue1'
   */
  if (TractionControl_u0 > TractionControl_P.Clamping_zero_Value) {
    TractionControl_tmp_0 = TractionControl_P.Constant3_Value;
  } else {
    TractionControl_tmp_0 = TractionControl_P.Constant4_Value;
  }

  /* Switch: '<S29>/Switch' incorporates:
   *  Constant: '<S29>/Clamping_zero'
   *  Constant: '<S29>/Constant1'
   *  Logic: '<S29>/AND3'
   *  RelationalOperator: '<S29>/Equal1'
   *  RelationalOperator: '<S29>/Relational Operator'
   *  Switch: '<S29>/Switch1'
   *  Switch: '<S29>/Switch2'
   */
  if ((TractionControl_P.Clamping_zero_Value != rtb_DeadZone_j) &&
      (TractionControl_tmp == TractionControl_tmp_0)) {
    TractionControl_u0 = TractionControl_P.Constant1_Value;
  }

  /* Update for DiscreteIntegrator: '<S38>/Integrator' incorporates:
   *  DiscreteIntegrator: '<S33>/Filter'
   *  Switch: '<S29>/Switch'
   */
  TractionControl_DW_l.Integrator_DSTATE[1] +=
    TractionControl_P.Integrator_gainval * TractionControl_u0;
  if (rtb_TC_activated_c > 0) {
    TractionControl_DW_l.Integrator_PrevResetState[1] = 1;
    TractionControl_DW_l.Filter_PrevResetState[1] = 1;
  } else {
    TractionControl_DW_l.Integrator_PrevResetState[1] = 0;
    TractionControl_DW_l.Filter_PrevResetState[1] = 0;
  }

  /* Update for DiscreteIntegrator: '<S33>/Filter' */
  TractionControl_DW_l.Filter_DSTATE[1] += TractionControl_P.Filter_gainval *
    rtb_NProdOut_l;

  /* Sum: '<S1>/Sum' incorporates:
   *  Inport: '<Root>/UpperSlipThreshold'
   */
  TractionControl_u0 = TractionControl_rtb_CurrentSlip_idx_2 -
    TractionControl_U.Desiredwheelslip;

  /* Saturate: '<S1>/Limit to positive' */
  if (TractionControl_u0 > TractionControl_P.Limittopositive_UpperSat) {
    TractionControl_u0 = TractionControl_P.Limittopositive_UpperSat;
  } else if (TractionControl_u0 < TractionControl_P.Limittopositive_LowerSat) {
    TractionControl_u0 = TractionControl_P.Limittopositive_LowerSat;
  }

  /* MATLAB Function: '<S1>/SlipActivationLogic' incorporates:
   *  Inport: '<Root>/BrakePressure'
   *  Inport: '<Root>/TC_enabled'
   *  Inport: '<Root>/UpperSlipThreshold'
   *  Inport: '<Root>/VehicleSpeed'
   *  Sum: '<S1>/Sum'
   */
  if ((TractionControl_U.TC_enabled == 1.0) && (TractionControl_U.VehicleSpeed >
       3.0) && (TractionControl_U.BrakePressure < 0.1)) {
    rtb_TC_activated_c = (TractionControl_rtb_CurrentSlip_idx_2 >
                          TractionControl_U.Desiredwheelslip);
  } else {
    rtb_TC_activated_c = 0;
  }

  /* DiscreteIntegrator: '<S38>/Integrator' */
  if ((rtb_TC_activated_c <= 0) &&
      (TractionControl_DW_l.Integrator_PrevResetState[2] == 1)) {
    TractionControl_DW_l.Integrator_DSTATE[2] =
      TractionControl_P.TCcontroller_InitialConditionForIntegra;
  }

  /* DiscreteIntegrator: '<S33>/Filter' */
  if ((rtb_TC_activated_c <= 0) && (TractionControl_DW_l.Filter_PrevResetState[2]
       == 1)) {
    TractionControl_DW_l.Filter_DSTATE[2] =
      TractionControl_P.TCcontroller_InitialConditionForFilter;
  }

  /* Product: '<S41>/NProd Out' incorporates:
   *  Constant: '<S1>/N'
   *  DiscreteIntegrator: '<S33>/Filter'
   *  Inport: '<Root>/Kd'
   *  Product: '<S32>/DProd Out'
   *  Sum: '<S33>/SumD'
   */
  rtb_NProdOut_l = (TractionControl_u0 * TractionControl_U.Derivativegain -
                    TractionControl_DW_l.Filter_DSTATE[2]) *
    TractionControl_P.N_Value;

  /* Sum: '<S47>/Sum' incorporates:
   *  DiscreteIntegrator: '<S38>/Integrator'
   *  Inport: '<Root>/Kp'
   *  Product: '<S43>/PProd Out'
   */
  rtb_DeadZone_j = (TractionControl_u0 * TractionControl_U.Proportionalgain +
                    TractionControl_DW_l.Integrator_DSTATE[2]) + rtb_NProdOut_l;

  /* Saturate: '<S45>/Saturation' incorporates:
   *  DeadZone: '<S31>/DeadZone'
   */
  if (rtb_DeadZone_j > TractionControl_P.TCcontroller_UpperSaturationLimit) {
    TractionControl_y = TractionControl_P.TCcontroller_UpperSaturationLimit;
    rtb_DeadZone_j -= TractionControl_P.TCcontroller_UpperSaturationLimit;
  } else {
    if (rtb_DeadZone_j < TractionControl_P.TCcontroller_LowerSaturationLimit) {
      TractionControl_y = TractionControl_P.TCcontroller_LowerSaturationLimit;
    } else {
      TractionControl_y = rtb_DeadZone_j;
    }

    if (rtb_DeadZone_j >= TractionControl_P.TCcontroller_LowerSaturationLimit) {
      rtb_DeadZone_j = 0.0;
    } else {
      rtb_DeadZone_j -= TractionControl_P.TCcontroller_LowerSaturationLimit;
    }
  }

  /* Outport: '<Root>/TC_RR' incorporates:
   *  Product: '<S1>/Product'
   *  Saturate: '<S45>/Saturation'
   */
  TractionControl_Y.TC_RR = (real_T)rtb_TC_activated_c * TractionControl_y;

  /* Product: '<S35>/IProd Out' incorporates:
   *  Inport: '<Root>/Ki'
   */
  TractionControl_u0 *= TractionControl_U.Integralgain;

  /* Switch: '<S29>/Switch1' incorporates:
   *  Constant: '<S29>/Clamping_zero'
   *  Constant: '<S29>/Constant'
   *  Constant: '<S29>/Constant2'
   *  RelationalOperator: '<S29>/fix for DT propagation issue'
   */
  if (rtb_DeadZone_j > TractionControl_P.Clamping_zero_Value) {
    TractionControl_tmp = TractionControl_P.Constant_Value;
  } else {
    TractionControl_tmp = TractionControl_P.Constant2_Value;
  }

  /* Switch: '<S29>/Switch2' incorporates:
   *  Constant: '<S29>/Clamping_zero'
   *  Constant: '<S29>/Constant3'
   *  Constant: '<S29>/Constant4'
   *  RelationalOperator: '<S29>/fix for DT propagation issue1'
   */
  if (TractionControl_u0 > TractionControl_P.Clamping_zero_Value) {
    TractionControl_tmp_0 = TractionControl_P.Constant3_Value;
  } else {
    TractionControl_tmp_0 = TractionControl_P.Constant4_Value;
  }

  /* Switch: '<S29>/Switch' incorporates:
   *  Constant: '<S29>/Clamping_zero'
   *  Constant: '<S29>/Constant1'
   *  Logic: '<S29>/AND3'
   *  RelationalOperator: '<S29>/Equal1'
   *  RelationalOperator: '<S29>/Relational Operator'
   *  Switch: '<S29>/Switch1'
   *  Switch: '<S29>/Switch2'
   */
  if ((TractionControl_P.Clamping_zero_Value != rtb_DeadZone_j) &&
      (TractionControl_tmp == TractionControl_tmp_0)) {
    TractionControl_u0 = TractionControl_P.Constant1_Value;
  }

  /* Update for DiscreteIntegrator: '<S38>/Integrator' incorporates:
   *  DiscreteIntegrator: '<S33>/Filter'
   *  Switch: '<S29>/Switch'
   */
  TractionControl_DW_l.Integrator_DSTATE[2] +=
    TractionControl_P.Integrator_gainval * TractionControl_u0;
  if (rtb_TC_activated_c > 0) {
    TractionControl_DW_l.Integrator_PrevResetState[2] = 1;
    TractionControl_DW_l.Filter_PrevResetState[2] = 1;
  } else {
    TractionControl_DW_l.Integrator_PrevResetState[2] = 0;
    TractionControl_DW_l.Filter_PrevResetState[2] = 0;
  }

  /* Update for DiscreteIntegrator: '<S33>/Filter' */
  TractionControl_DW_l.Filter_DSTATE[2] += TractionControl_P.Filter_gainval *
    rtb_NProdOut_l;

  /* Sum: '<S1>/Sum' incorporates:
   *  Inport: '<Root>/UpperSlipThreshold'
   */
  TractionControl_u0 = TractionControl_rtb_CurrentSlip_idx_3 -
    TractionControl_U.Desiredwheelslip;

  /* Saturate: '<S1>/Limit to positive' */
  if (TractionControl_u0 > TractionControl_P.Limittopositive_UpperSat) {
    TractionControl_u0 = TractionControl_P.Limittopositive_UpperSat;
  } else if (TractionControl_u0 < TractionControl_P.Limittopositive_LowerSat) {
    TractionControl_u0 = TractionControl_P.Limittopositive_LowerSat;
  }

  /* MATLAB Function: '<S1>/SlipActivationLogic' incorporates:
   *  Inport: '<Root>/BrakePressure'
   *  Inport: '<Root>/TC_enabled'
   *  Inport: '<Root>/UpperSlipThreshold'
   *  Inport: '<Root>/VehicleSpeed'
   *  Sum: '<S1>/Sum'
   */
  if ((TractionControl_U.TC_enabled == 1.0) && (TractionControl_U.VehicleSpeed >
       3.0) && (TractionControl_U.BrakePressure < 0.1)) {
    rtb_TC_activated_c = (TractionControl_rtb_CurrentSlip_idx_3 >
                          TractionControl_U.Desiredwheelslip);
  } else {
    rtb_TC_activated_c = 0;
  }

  /* DiscreteIntegrator: '<S38>/Integrator' */
  if ((rtb_TC_activated_c <= 0) &&
      (TractionControl_DW_l.Integrator_PrevResetState[3] == 1)) {
    TractionControl_DW_l.Integrator_DSTATE[3] =
      TractionControl_P.TCcontroller_InitialConditionForIntegra;
  }

  /* DiscreteIntegrator: '<S33>/Filter' */
  if ((rtb_TC_activated_c <= 0) && (TractionControl_DW_l.Filter_PrevResetState[3]
       == 1)) {
    TractionControl_DW_l.Filter_DSTATE[3] =
      TractionControl_P.TCcontroller_InitialConditionForFilter;
  }

  /* Product: '<S41>/NProd Out' incorporates:
   *  Constant: '<S1>/N'
   *  DiscreteIntegrator: '<S33>/Filter'
   *  Inport: '<Root>/Kd'
   *  Product: '<S32>/DProd Out'
   *  Sum: '<S33>/SumD'
   */
  rtb_NProdOut_l = (TractionControl_u0 * TractionControl_U.Derivativegain -
                    TractionControl_DW_l.Filter_DSTATE[3]) *
    TractionControl_P.N_Value;

  /* Sum: '<S47>/Sum' incorporates:
   *  DiscreteIntegrator: '<S38>/Integrator'
   *  Inport: '<Root>/Kp'
   *  Product: '<S43>/PProd Out'
   */
  rtb_DeadZone_j = (TractionControl_u0 * TractionControl_U.Proportionalgain +
                    TractionControl_DW_l.Integrator_DSTATE[3]) + rtb_NProdOut_l;

  /* Saturate: '<S45>/Saturation' incorporates:
   *  DeadZone: '<S31>/DeadZone'
   */
  if (rtb_DeadZone_j > TractionControl_P.TCcontroller_UpperSaturationLimit) {
    TractionControl_y = TractionControl_P.TCcontroller_UpperSaturationLimit;
    rtb_DeadZone_j -= TractionControl_P.TCcontroller_UpperSaturationLimit;
  } else {
    if (rtb_DeadZone_j < TractionControl_P.TCcontroller_LowerSaturationLimit) {
      TractionControl_y = TractionControl_P.TCcontroller_LowerSaturationLimit;
    } else {
      TractionControl_y = rtb_DeadZone_j;
    }

    if (rtb_DeadZone_j >= TractionControl_P.TCcontroller_LowerSaturationLimit) {
      rtb_DeadZone_j = 0.0;
    } else {
      rtb_DeadZone_j -= TractionControl_P.TCcontroller_LowerSaturationLimit;
    }
  }

  /* Product: '<S35>/IProd Out' incorporates:
   *  Inport: '<Root>/Ki'
   */
  TractionControl_u0 *= TractionControl_U.Integralgain;

  /* Switch: '<S29>/Switch1' incorporates:
   *  Constant: '<S29>/Clamping_zero'
   *  Constant: '<S29>/Constant'
   *  Constant: '<S29>/Constant2'
   *  RelationalOperator: '<S29>/fix for DT propagation issue'
   */
  if (rtb_DeadZone_j > TractionControl_P.Clamping_zero_Value) {
    TractionControl_tmp = TractionControl_P.Constant_Value;
  } else {
    TractionControl_tmp = TractionControl_P.Constant2_Value;
  }

  /* Switch: '<S29>/Switch2' incorporates:
   *  Constant: '<S29>/Clamping_zero'
   *  Constant: '<S29>/Constant3'
   *  Constant: '<S29>/Constant4'
   *  RelationalOperator: '<S29>/fix for DT propagation issue1'
   */
  if (TractionControl_u0 > TractionControl_P.Clamping_zero_Value) {
    TractionControl_tmp_0 = TractionControl_P.Constant3_Value;
  } else {
    TractionControl_tmp_0 = TractionControl_P.Constant4_Value;
  }

  /* Switch: '<S29>/Switch' incorporates:
   *  Constant: '<S29>/Clamping_zero'
   *  Constant: '<S29>/Constant1'
   *  Logic: '<S29>/AND3'
   *  RelationalOperator: '<S29>/Equal1'
   *  RelationalOperator: '<S29>/Relational Operator'
   *  Switch: '<S29>/Switch1'
   *  Switch: '<S29>/Switch2'
   */
  if ((TractionControl_P.Clamping_zero_Value != rtb_DeadZone_j) &&
      (TractionControl_tmp == TractionControl_tmp_0)) {
    TractionControl_u0 = TractionControl_P.Constant1_Value;
  }

  /* Update for DiscreteIntegrator: '<S38>/Integrator' incorporates:
   *  DiscreteIntegrator: '<S33>/Filter'
   *  Switch: '<S29>/Switch'
   */
  TractionControl_DW_l.Integrator_DSTATE[3] +=
    TractionControl_P.Integrator_gainval * TractionControl_u0;
  if (rtb_TC_activated_c > 0) {
    TractionControl_DW_l.Integrator_PrevResetState[3] = 1;
    TractionControl_DW_l.Filter_PrevResetState[3] = 1;
  } else {
    TractionControl_DW_l.Integrator_PrevResetState[3] = 0;
    TractionControl_DW_l.Filter_PrevResetState[3] = 0;
  }

  /* Update for DiscreteIntegrator: '<S33>/Filter' */
  TractionControl_DW_l.Filter_DSTATE[3] += TractionControl_P.Filter_gainval *
    rtb_NProdOut_l;

  /* Outport: '<Root>/TC_RL' incorporates:
   *  Product: '<S1>/Product'
   */
  TractionControl_Y.TC_RL = (real_T)rtb_TC_activated_c * TractionControl_y;

  /* End of Outputs for SubSystem: '<Root>/TC_controller' */

  /* Outport: '<Root>/slipFL' */
  TractionControl_Y.slipFL = TractionControl_rtb_CurrentSlip_idx_0;

  /* Outport: '<Root>/slipFR' */
  TractionControl_Y.slipFR = TractionControl_rtb_CurrentSlip_idx_1;

  /* Outport: '<Root>/slipRL' */
  TractionControl_Y.slipRL = TractionControl_rtb_CurrentSlip_idx_2;

  /* Outport: '<Root>/slipRR' */
  TractionControl_Y.slipRR = TractionControl_rtb_CurrentSlip_idx_3;
}

/* Model initialize function */
void TractionControl_initialize(void)
{
  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));

  /* non-finite (run-time) assignments */
  TractionControl_P.Limittopositive_UpperSat = rtInf;

  /* SystemInitialize for Atomic SubSystem: '<Root>/TC_controller' */
  /* InitializeConditions for DiscreteIntegrator: '<S38>/Integrator' */
  TractionControl_DW_l.Integrator_DSTATE[0] =
    TractionControl_P.TCcontroller_InitialConditionForIntegra;
  TractionControl_DW_l.Integrator_PrevResetState[0] = 2;

  /* InitializeConditions for DiscreteIntegrator: '<S33>/Filter' */
  TractionControl_DW_l.Filter_DSTATE[0] =
    TractionControl_P.TCcontroller_InitialConditionForFilter;
  TractionControl_DW_l.Filter_PrevResetState[0] = 2;

  /* InitializeConditions for DiscreteIntegrator: '<S38>/Integrator' */
  TractionControl_DW_l.Integrator_DSTATE[1] =
    TractionControl_P.TCcontroller_InitialConditionForIntegra;
  TractionControl_DW_l.Integrator_PrevResetState[1] = 2;

  /* InitializeConditions for DiscreteIntegrator: '<S33>/Filter' */
  TractionControl_DW_l.Filter_DSTATE[1] =
    TractionControl_P.TCcontroller_InitialConditionForFilter;
  TractionControl_DW_l.Filter_PrevResetState[1] = 2;

  /* InitializeConditions for DiscreteIntegrator: '<S38>/Integrator' */
  TractionControl_DW_l.Integrator_DSTATE[2] =
    TractionControl_P.TCcontroller_InitialConditionForIntegra;
  TractionControl_DW_l.Integrator_PrevResetState[2] = 2;

  /* InitializeConditions for DiscreteIntegrator: '<S33>/Filter' */
  TractionControl_DW_l.Filter_DSTATE[2] =
    TractionControl_P.TCcontroller_InitialConditionForFilter;
  TractionControl_DW_l.Filter_PrevResetState[2] = 2;

  /* InitializeConditions for DiscreteIntegrator: '<S38>/Integrator' */
  TractionControl_DW_l.Integrator_DSTATE[3] =
    TractionControl_P.TCcontroller_InitialConditionForIntegra;
  TractionControl_DW_l.Integrator_PrevResetState[3] = 2;

  /* InitializeConditions for DiscreteIntegrator: '<S33>/Filter' */
  TractionControl_DW_l.Filter_DSTATE[3] =
    TractionControl_P.TCcontroller_InitialConditionForFilter;
  TractionControl_DW_l.Filter_PrevResetState[3] = 2;

  /* End of SystemInitialize for SubSystem: '<Root>/TC_controller' */
}

/*
 * Initialize the rtInf, rtMinusInf, and rtNaN needed by the
 * generated code. NaN is initialized as non-signaling. Assumes IEEE.
 */
static void rt_InitInfAndNaN(size_t realSize)
{
  (void) (realSize);
  rtNaN = rtGetNaN();
  rtNaNF = rtGetNaNF();
  rtInf = rtGetInf();
  rtInfF = rtGetInfF();
  rtMinusInf = rtGetMinusInf();
  rtMinusInfF = rtGetMinusInfF();
}

/* Test if value is infinite */
static boolean_T rtIsInf(real_T value)
{
  return (boolean_T)((value==rtInf || value==rtMinusInf) ? 1U : 0U);
}

/* Test if single-precision value is infinite */
static boolean_T rtIsInfF(real32_T value)
{
  return (boolean_T)(((value)==rtInfF || (value)==rtMinusInfF) ? 1U : 0U);
}

/* Test if value is not a number */
static boolean_T rtIsNaN(real_T value)
{
  boolean_T result = (boolean_T) 0;
  size_t bitsPerReal = sizeof(real_T) * (NumBitsPerChar);
  if (bitsPerReal == 32U) {
    result = rtIsNaNF((real32_T)value);
  } else {
    union {
      LittleEndianIEEEDouble bitVal;
      real_T fltVal;
    } tmpVal;

    tmpVal.fltVal = value;
    result = (boolean_T)((tmpVal.bitVal.words.wordH & 0x7FF00000) == 0x7FF00000 &&
                         ( (tmpVal.bitVal.words.wordH & 0x000FFFFF) != 0 ||
                          (tmpVal.bitVal.words.wordL != 0) ));
  }

  return result;
}

/* Test if single-precision value is not a number */
static boolean_T rtIsNaNF(real32_T value)
{
  IEEESingle tmp;
  tmp.wordL.wordLreal = value;
  return (boolean_T)( (tmp.wordL.wordLuint & 0x7F800000) == 0x7F800000 &&
                     (tmp.wordL.wordLuint & 0x007FFFFF) != 0 );
}

/*
 * Initialize rtNaN needed by the generated code.
 * NaN is initialized as non-signaling. Assumes IEEE.
 */
static real_T rtGetNaN(void)
{
  size_t bitsPerReal = sizeof(real_T) * (NumBitsPerChar);
  real_T nan = 0.0;
  if (bitsPerReal == 32U) {
    nan = rtGetNaNF();
  } else {
    union {
      LittleEndianIEEEDouble bitVal;
      real_T fltVal;
    } tmpVal;

    tmpVal.bitVal.words.wordH = 0xFFF80000U;
    tmpVal.bitVal.words.wordL = 0x00000000U;
    nan = tmpVal.fltVal;
  }

  return nan;
}

/*
 * Initialize rtNaNF needed by the generated code.
 * NaN is initialized as non-signaling. Assumes IEEE.
 */
static real32_T rtGetNaNF(void)
{
  IEEESingle nanF = { { 0.0F } };

  nanF.wordL.wordLuint = 0xFFC00000U;
  return nanF.wordL.wordLreal;
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
