/*
 * Sponsored License - for use in support of a program or activity
 * sponsored by MathWorks.  Not for government, commercial or other
 * non-sponsored organizational use.
 *
 * File: SubsystemModelReference.c
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

#include "SubsystemModelReference.h"
#define NumBitsPerChar                 8U

/* Block signals and states (default storage) */
DW rtDW;

/* External inputs (root inport signals with default storage) */
ExtU rtU;

/* External outputs (root outports fed by signals with default storage) */
ExtY rtY;

/* Real-time model */
RT_MODEL rtM_;
RT_MODEL *const rtM = &rtM_;
static real_T look1_binlx(real_T u0, const real_T bp0[], const real_T table[],
  uint32_T maxIndex);
static void EvaluateRuleConsequents(const real_T rtu_antecedentOutputs[28],
  const real_T rtu_samplePoints[101], real_T rty_aggregatedOutputs[101]);
static void MATLABFunction(real_T rtu_upper_slip_treshold, real_T
  rtu_lower_slip_treshold, real_T rtu_Current_slip, real_T *rty_SlipActiveFR);

/* Forward declaration for local functions */
static void createMamdaniOutputMFCache(const real_T outputSamplePoints[101],
  real_T outputMFCache[404]);
static real_T evaluateAndMethod(const real_T x[2]);
static real_T evaluateOrMethod(const real_T x[2]);
static real_T rtGetNaN(void);
static real32_T rtGetNaNF(void);
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
static real_T rtGetInf(void);
static real32_T rtGetInfF(void);
static real_T rtGetMinusInf(void);
static real32_T rtGetMinusInfF(void);

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
  IEEESingle nanF = { { 0 } };

  nanF.wordL.wordLuint = 0xFFC00000U;
  return nanF.wordL.wordLreal;
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

static real_T look1_binlx(real_T u0, const real_T bp0[], const real_T table[],
  uint32_T maxIndex)
{
  real_T frac;
  uint32_T iRght;
  uint32_T iLeft;
  uint32_T bpIdx;

  /* Column-major Lookup 1-D
     Search method: 'binary'
     Use previous index: 'off'
     Interpolation method: 'Linear point-slope'
     Extrapolation method: 'Linear'
     Use last breakpoint for index at or above upper limit: 'off'
     Remove protection against out-of-range input in generated code: 'off'
   */
  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Linear'
     Use previous index: 'off'
     Use last breakpoint for index at or above upper limit: 'off'
     Remove protection against out-of-range input in generated code: 'off'
   */
  if (u0 <= bp0[0U]) {
    iLeft = 0U;
    frac = (u0 - bp0[0U]) / (bp0[1U] - bp0[0U]);
  } else if (u0 < bp0[maxIndex]) {
    /* Binary Search */
    bpIdx = maxIndex >> 1U;
    iLeft = 0U;
    iRght = maxIndex;
    while (iRght - iLeft > 1U) {
      if (u0 < bp0[bpIdx]) {
        iRght = bpIdx;
      } else {
        iLeft = bpIdx;
      }

      bpIdx = (iRght + iLeft) >> 1U;
    }

    frac = (u0 - bp0[iLeft]) / (bp0[iLeft + 1U] - bp0[iLeft]);
  } else {
    iLeft = maxIndex - 1U;
    frac = (u0 - bp0[maxIndex - 1U]) / (bp0[maxIndex] - bp0[maxIndex - 1U]);
  }

  /* Column-major Interpolation 1-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'off'
     Overflow mode: 'wrapping'
   */
  return (table[iLeft + 1U] - table[iLeft]) * frac + table[iLeft];
}

/* Function for MATLAB Function: '<S30>/Evaluate Rule Consequents' */
static void createMamdaniOutputMFCache(const real_T outputSamplePoints[101],
  real_T outputMFCache[404])
{
  int32_T e_k;
  real_T z1;
  real_T outputSamplePoints_0;
  for (e_k = 0; e_k < 101; e_k++) {
    outputSamplePoints_0 = outputSamplePoints[e_k];
    z1 = outputSamplePoints_0 * outputSamplePoints_0;
    z1 = -z1 / 0.0242;
    z1 = exp(z1);
    outputMFCache[e_k << 2] = z1;
    z1 = outputSamplePoints_0 - 0.3;
    z1 *= z1;
    z1 = -z1 / 0.0242;
    z1 = exp(z1);
    outputMFCache[(e_k << 2) + 1] = z1;
    z1 = outputSamplePoints_0 - 0.6;
    z1 *= z1;
    z1 = -z1 / 0.0242;
    z1 = exp(z1);
    outputMFCache[(e_k << 2) + 2] = z1;
    z1 = outputSamplePoints_0 - 1.0;
    z1 *= z1;
    z1 = -z1 / 0.0242;
    z1 = exp(z1);
    outputMFCache[(e_k << 2) + 3] = z1;
  }
}

/* Function for MATLAB Function: '<S30>/Evaluate Rule Consequents' */
static real_T evaluateAndMethod(const real_T x[2])
{
  real_T y;
  if (x[0] > x[1]) {
    y = x[1];
  } else if (rtIsNaN(x[0])) {
    if (!rtIsNaN(x[1])) {
      y = x[1];
    } else {
      y = x[0];
    }
  } else {
    y = x[0];
  }

  return y;
}

/* Function for MATLAB Function: '<S30>/Evaluate Rule Consequents' */
static real_T evaluateOrMethod(const real_T x[2])
{
  real_T y;
  if (x[0] < x[1]) {
    y = x[1];
  } else if (rtIsNaN(x[0])) {
    if (!rtIsNaN(x[1])) {
      y = x[1];
    } else {
      y = x[0];
    }
  } else {
    y = x[0];
  }

  return y;
}

/*
 * Output and update for atomic system:
 *    '<S30>/Evaluate Rule Consequents'
 *    '<S42>/Evaluate Rule Consequents'
 *    '<S54>/Evaluate Rule Consequents'
 *    '<S66>/Evaluate Rule Consequents'
 */
static void EvaluateRuleConsequents(const real_T rtu_antecedentOutputs[28],
  const real_T rtu_samplePoints[101], real_T rty_aggregatedOutputs[101])
{
  real_T outputMFCache[404];
  int32_T ruleID;
  int32_T sampleID;
  real_T outputMFCache_0[2];
  real_T tmp;
  real_T rty_aggregatedOutputs_0[2];
  static const int8_T b[28] = { 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 3, 4,
    4, 4, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4 };

  memset(&rty_aggregatedOutputs[0], 0, 101U * sizeof(real_T));
  createMamdaniOutputMFCache(rtu_samplePoints, outputMFCache);
  for (ruleID = 0; ruleID < 28; ruleID++) {
    outputMFCache_0[1] = rtu_antecedentOutputs[ruleID];
    for (sampleID = 0; sampleID < 101; sampleID++) {
      outputMFCache_0[0] = outputMFCache[((sampleID << 2) + b[ruleID]) - 1];
      tmp = evaluateAndMethod(outputMFCache_0);
      rty_aggregatedOutputs_0[0] = rty_aggregatedOutputs[sampleID];
      rty_aggregatedOutputs_0[1] = tmp;
      rty_aggregatedOutputs[sampleID] = evaluateOrMethod(rty_aggregatedOutputs_0);
    }
  }
}

/*
 * Output and update for atomic system:
 *    '<S23>/MATLAB Function'
 *    '<S35>/MATLAB Function'
 */
static void MATLABFunction(real_T rtu_upper_slip_treshold, real_T
  rtu_lower_slip_treshold, real_T rtu_Current_slip, real_T *rty_SlipActiveFR)
{
  *rty_SlipActiveFR = 0.0;
  if (rtu_Current_slip > rtu_upper_slip_treshold) {
    *rty_SlipActiveFR = 1.0;
  } else {
    if (rtu_Current_slip < rtu_lower_slip_treshold) {
      *rty_SlipActiveFR = 0.0;
    }
  }
}

/* Model step function */
void SubsystemModelReference_step(void)
{
  real_T inputMFCache[11];
  int32_T ruleID;
  real_T area;
  real_T rtb_Product_dt;
  real_T rtb_Product1_a;
  real_T rtb_Product2_h;
  real_T rtb_Product3;
  real_T rtb_LimittomaxRPM;
  real_T rtb_LimittomaxRPM_d;
  real_T rtb_LimittomaxRPM_a;
  real_T rtb_LimittomaxRPM_n;
  boolean_T rtb_AND_oh;
  boolean_T rtb_AND_hu;
  boolean_T rtb_AND_f;
  boolean_T rtb_AND_i;
  boolean_T rtb_AND_am;
  boolean_T rtb_AND_c;
  boolean_T rtb_AND_g;
  boolean_T rtb_AND_k3;
  real_T rtb_Gain_k;
  real_T rtb_Gain1_a;
  real_T rtb_Steeringangleatwheelsdegs;
  real_T rtb_Product;
  real_T rtb_Plus;
  real_T rtb_Desiredyawratereferencedegs;
  real_T rtb_Upperlimitrads;
  real_T rtb_Pgains;
  real_T rtb_DeadZone_j;
  real_T rtb_Product1;
  boolean_T rtb_AND;
  real_T rtb_Yawmomentsaturation;
  real_T rtb_Product2;
  real_T rtb_Product_i;
  boolean_T rtb_AND_d;
  real_T rtb_Yawmomentsaturation_p;
  real_T rtb_Product2_i;
  real_T rtb_Gain1;
  real_T rtb_Desiredtorquedifferenceatfr;
  real_T rtb_Gain_e;
  real_T rtb_Desiredtorquedifferenceatre;
  real_T rtb_Igains;
  real_T rtb_Product_fk;
  real_T rtb_Product_a;
  real_T rtb_Minus;
  real_T rtb_Fcn;
  real_T rtb_Divide;
  boolean_T rtb_TCS_active_FL;
  real_T rtb_defuzzifiedOutputs_p;
  real_T rtb_Product_lr;
  real_T rtb_Product_ny;
  real_T rtb_Minus_d;
  real_T rtb_Fcn_a;
  real_T rtb_Divide_k;
  boolean_T rtb_TCS_active_FR;
  real_T rtb_defuzzifiedOutputs_o;
  real_T rtb_Product_e;
  real_T rtb_Product_mk;
  real_T rtb_Minus_m;
  real_T rtb_Fcn_g;
  real_T rtb_Divide_a;
  boolean_T rtb_TCS_active_RL;
  real_T rtb_defuzzifiedOutputs_ds;
  real_T rtb_Product_fz;
  real_T rtb_Product_c;
  real_T rtb_Minus_kk;
  real_T rtb_Fcn_p;
  real_T rtb_Divide_bk;
  boolean_T rtb_TCS_active_RR;
  real_T rtb_defuzzifiedOutputs;
  real_T rtb_Product_h;
  real_T rtb_Divide_i;
  real_T rtb_Product_m;
  real_T rtb_Product2_a;
  real_T rtb_Divide_m4;
  boolean_T rtb_AND1;
  real_T rtb_Product3_na;
  real_T rtb_Divide_j;
  real_T rtb_Product_mu;
  real_T rtb_Product2_m;
  real_T rtb_Divide_f;
  real_T rtb_Product3_l;
  real_T rtb_Divide_m;
  real_T rtb_Product_g;
  real_T rtb_Product2_d;
  real_T rtb_Divide_k0;
  real_T rtb_Product3_o;
  real_T rtb_Divide_h;
  real_T rtb_Product_o;
  real_T rtb_Product2_f;
  real_T rtb_Divide_ak;
  real_T rtb_Product3_j;
  real_T rtb_DeadZone;
  real_T rtb_Abs;
  boolean_T rtb_Compare;
  real_T rtb_Sign;
  boolean_T rtb_TVactivation;
  real_T rtb_Square;
  real_T rtb_Gain;
  real_T rtb_Gain2;
  real_T rtb_Vehicleyawratedegs;
  real_T rtb_yaw_rate_reference;
  real_T rtb_DiscreteTimeIntegrator;
  real_T rtb_Sum;
  real_T rtb_Sum_g;
  real_T rtb_Yawmomentsaturation_n;
  real_T rtb_Gain3;
  real_T rtb_Gain2_m;
  real_T rtb_Gain_m;
  real_T rtb_DeadZone_oo;
  boolean_T rtb_NotEqual;
  real_T rtb_Sign_e;
  int8_T rtb_DataTypeConversion;
  real_T rtb_Sign1;
  int8_T rtb_DataTypeConversion1;
  boolean_T rtb_Equal;
  boolean_T rtb_AND_p;
  boolean_T rtb_Compare_i;
  boolean_T rtb_Compare_f;
  boolean_T rtb_AND_l;
  real_T rtb_Max;
  real_T rtb_SlipActiveFR_e;
  real_T rtb_Minus_ph;
  real_T rtb_Saturation1;
  real_T rtb_TSamp;
  real_T rtb_Uk1;
  real_T rtb_Diff;
  real_T rtb_Saturation;
  real_T rtb_Max_b;
  real_T rtb_SlipActiveFR;
  real_T rtb_Minus_p;
  real_T rtb_Saturation1_l;
  real_T rtb_TSamp_a;
  real_T rtb_Uk1_b;
  real_T rtb_Diff_o;
  real_T rtb_Saturation_g;
  real_T rtb_Max_j;
  int32_T rtb_SlipActiveRL;
  real_T rtb_Minus_i3;
  real_T rtb_Saturation1_h;
  real_T rtb_TSamp_n;
  real_T rtb_Uk1_c;
  real_T rtb_Diff_ns;
  real_T rtb_Saturation_l;
  real_T rtb_Max_hv;
  int32_T rtb_SlipActiveRR;
  real_T rtb_Minus_e;
  real_T rtb_Saturation1_d;
  real_T rtb_TSamp_k;
  real_T rtb_Uk1_m;
  real_T rtb_Diff_i;
  real_T rtb_Saturation_n;
  boolean_T rtb_RelationalOperator;
  boolean_T rtb_RelationalOperator1;
  boolean_T rtb_RelationalOperator_i4;
  boolean_T rtb_RelationalOperator1_eh;
  boolean_T rtb_RelationalOperator_f;
  boolean_T rtb_RelationalOperator1_ao;
  boolean_T rtb_RelationalOperator_ie;
  boolean_T rtb_RelationalOperator1_d;
  boolean_T rtb_RelationalOperator_g;
  boolean_T rtb_RelationalOperator1_ks;
  boolean_T rtb_RelationalOperator_n;
  boolean_T rtb_RelationalOperator1_gj;
  boolean_T rtb_RelationalOperator_fd;
  boolean_T rtb_RelationalOperator1_i;
  boolean_T rtb_RelationalOperator_d;
  boolean_T rtb_RelationalOperator1_e;
  real_T rtb_Switch;
  real_T rtb_Yawrateerror;
  real_T rtb_antecedentOutputs_f[28];
  real_T rtb_sumAntecedentOutputs_p;
  real_T rtb_antecedentOutputs_o[28];
  real_T rtb_sumAntecedentOutputs_c;
  real_T rtb_antecedentOutputs_g[28];
  real_T rtb_sumAntecedentOutputs_b;
  real_T rtb_antecedentOutputs[28];
  real_T rtb_sumAntecedentOutputs;
  real_T rtb_aggregatedOutputs_bd[101];
  real_T rtb_aggregatedOutputs_n[101];
  real_T rtb_aggregatedOutputs_d[101];
  real_T rtb_aggregatedOutputs[101];
  real_T x_idx_0;
  static const int8_T b[56] = { 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3,
    3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 1, 2, 4, 7, 5, 6, 3, 1, 4, 7, 2, 5, 6, 6,
    1, 4, 7, 2, 5, 6, 3, 1, 4, 7, 2, 5, 6, 3 };

  /* Outputs for Atomic SubSystem: '<Root>/TV_TC_Controller' */
  /* DeadZone: '<S6>/Dead Zone' incorporates:
   *  Inport: '<Root>/bus_Vehicle_str_ang'
   */
  if (rtU.bus_Vehicle_str_ang > 10.0) {
    rtb_DeadZone = rtU.bus_Vehicle_str_ang - 10.0;
  } else if (rtU.bus_Vehicle_str_ang >= -10.0) {
    rtb_DeadZone = 0.0;
  } else {
    rtb_DeadZone = rtU.bus_Vehicle_str_ang - -10.0;
  }

  /* End of DeadZone: '<S6>/Dead Zone' */

  /* Abs: '<S6>/Abs' */
  rtb_Abs = fabs(rtb_DeadZone);

  /* Signum: '<S6>/Sign' */
  if (rtb_Abs > 0.0) {
    rtb_Sign = 1.0;
  } else if (rtb_Abs == 0.0) {
    rtb_Sign = 0.0;
  } else {
    rtb_Sign = (rtNaN);
  }

  /* End of Signum: '<S6>/Sign' */

  /* RelationalOperator: '<S5>/Compare' incorporates:
   *  Constant: '<S5>/Constant'
   *  Inport: '<Root>/bus_Vehicle_velocity'
   */
  rtb_Compare = (rtU.bus_Vehicle_velocity >= 2.5);

  /* Logic: '<S2>/AND' incorporates:
   *  Inport: '<Root>/bus_Torque_vectoring_active'
   */
  rtb_TVactivation = ((rtb_Sign != 0.0) && rtb_Compare &&
                      (rtU.bus_Torque_vectoring_active != 0.0));

  /* Logic: '<S11>/AND' incorporates:
   *  Inport: '<Root>/bus_feedback_active'
   */
  rtb_AND = (rtU.bus_feedback_active && rtb_TVactivation);

  /* Lookup_n-D: '<S13>/P gains' incorporates:
   *  Inport: '<Root>/bus_Vehicle_velocity'
   */
  rtb_Pgains = look1_binlx(rtU.bus_Vehicle_velocity, rtConstP.pooled10,
    rtConstP.Pgains_tableData, 12U);

  /* Gain: '<S9>/Gain1' incorporates:
   *  Inport: '<Root>/bus_Vehicle_str_ang'
   */
  rtb_Steeringangleatwheelsdegs = 0.2 * rtU.bus_Vehicle_str_ang;

  /* Product: '<S9>/Product' incorporates:
   *  Inport: '<Root>/bus_Vehicle_velocity'
   */
  rtb_Product = rtb_Steeringangleatwheelsdegs * rtU.bus_Vehicle_velocity;

  /* Math: '<S9>/Square' incorporates:
   *  Inport: '<Root>/bus_Vehicle_velocity'
   */
  rtb_Square = rtU.bus_Vehicle_velocity * rtU.bus_Vehicle_velocity;

  /* Gain: '<S9>/Gain' */
  rtb_Gain = 0.0 * rtb_Square;

  /* Sum: '<S9>/Plus' incorporates:
   *  Constant: '<S9>/Constant'
   */
  rtb_Plus = rtb_Gain + 1.53;

  /* Product: '<S9>/Divide' */
  rtb_Desiredyawratereferencedegs = rtb_Product / rtb_Plus;

  /* Product: '<S9>/Divide1' incorporates:
   *  Inport: '<Root>/bus_Vehicle_velocity'
   */
  rtb_Upperlimitrads = 1.0 / rtU.bus_Vehicle_velocity * 19.62;

  /* Gain: '<S9>/Gain2' */
  rtb_Gain2 = 57.295779513082323 * rtb_Upperlimitrads;

  /* MATLAB Function: '<S9>/MATLAB Function' */
  if (fabs(rtb_Desiredyawratereferencedegs) < rtb_Gain2) {
    rtb_yaw_rate_reference = rtb_Desiredyawratereferencedegs;
  } else {
    if (rtb_Desiredyawratereferencedegs < 0.0) {
      area = -1.0;
    } else if (rtb_Desiredyawratereferencedegs > 0.0) {
      area = 1.0;
    } else if (rtb_Desiredyawratereferencedegs == 0.0) {
      area = 0.0;
    } else {
      area = (rtNaN);
    }

    rtb_yaw_rate_reference = rtb_Gain2 * area;
  }

  /* End of MATLAB Function: '<S9>/MATLAB Function' */

  /* Gain: '<S13>/Gain' incorporates:
   *  Inport: '<Root>/bus_Vehicle_yaw_rate'
   */
  rtb_Vehicleyawratedegs = 57.295779513082323 * rtU.bus_Vehicle_yaw_rate;

  /* Sum: '<S13>/Sum1' */
  rtb_Yawrateerror = rtb_yaw_rate_reference - rtb_Vehicleyawratedegs;

  /* DeadZone: '<S13>/Dead Zone' */
  if (rtb_Yawrateerror > 2.0) {
    rtb_DeadZone_j = rtb_Yawrateerror - 2.0;
  } else if (rtb_Yawrateerror >= -2.0) {
    rtb_DeadZone_j = 0.0;
  } else {
    rtb_DeadZone_j = rtb_Yawrateerror - -2.0;
  }

  /* End of DeadZone: '<S13>/Dead Zone' */

  /* Product: '<S13>/Product1' */
  rtb_Product1 = rtb_Pgains * rtb_DeadZone_j;

  /* DiscreteIntegrator: '<S13>/Discrete-Time Integrator' */
  rtb_DiscreteTimeIntegrator = rtDW.DiscreteTimeIntegrator_DSTATE;

  /* Sum: '<S13>/Sum' */
  rtb_Sum = rtb_Product1 + rtb_DiscreteTimeIntegrator;

  /* Saturate: '<S11>/Yaw moment saturation' */
  if (rtb_Sum > 3600.0) {
    rtb_Yawmomentsaturation = 3600.0;
  } else if (rtb_Sum < -3600.0) {
    rtb_Yawmomentsaturation = -3600.0;
  } else {
    rtb_Yawmomentsaturation = rtb_Sum;
  }

  /* End of Saturate: '<S11>/Yaw moment saturation' */

  /* Product: '<S11>/Product2' */
  rtb_Product2 = (real_T)rtb_AND * rtb_Yawmomentsaturation;

  /* Logic: '<S12>/AND' incorporates:
   *  Inport: '<Root>/bus_feedforward_active'
   */
  rtb_AND_d = (rtU.bus_feedforward_active && rtb_TVactivation);

  /* Product: '<S12>/Product' incorporates:
   *  Constant: '<S12>/Constant'
   *  Inport: '<Root>/bus_Vehicle_str_ang'
   */
  rtb_Product_i = rtU.bus_Vehicle_str_ang * 10.0;

  /* Saturate: '<S12>/Yaw moment saturation' */
  if (rtb_Product_i > 3600.0) {
    rtb_Yawmomentsaturation_p = 3600.0;
  } else if (rtb_Product_i < -3600.0) {
    rtb_Yawmomentsaturation_p = -3600.0;
  } else {
    rtb_Yawmomentsaturation_p = rtb_Product_i;
  }

  /* End of Saturate: '<S12>/Yaw moment saturation' */

  /* Product: '<S12>/Product2' */
  rtb_Product2_i = (real_T)rtb_AND_d * rtb_Yawmomentsaturation_p;

  /* Sum: '<S8>/Sum' */
  rtb_Sum_g = rtb_Product2 + rtb_Product2_i;

  /* Saturate: '<S8>/Yaw moment saturation' */
  if (rtb_Sum_g > 3600.0) {
    rtb_Yawmomentsaturation_n = 3600.0;
  } else if (rtb_Sum_g < -3600.0) {
    rtb_Yawmomentsaturation_n = -3600.0;
  } else {
    rtb_Yawmomentsaturation_n = rtb_Sum_g;
  }

  /* End of Saturate: '<S8>/Yaw moment saturation' */

  /* Gain: '<S10>/Gain3' */
  rtb_Gain3 = 0.5 * rtb_Yawmomentsaturation_n;

  /* Gain: '<S10>/Gain1' */
  rtb_Gain1 = 0.205 * rtb_Gain3;

  /* Product: '<S10>/Divide1' incorporates:
   *  Constant: '<S10>/Constant'
   */
  rtb_Desiredtorquedifferenceatfr = rtb_Gain1 / 14.82;

  /* Gain: '<S7>/Gain' */
  rtb_Gain_k = 0.5 * rtb_Desiredtorquedifferenceatfr;

  /* Gain: '<S10>/Gain2' */
  rtb_Gain2_m = 0.5 * rtb_Yawmomentsaturation_n;

  /* Gain: '<S10>/Gain' */
  rtb_Gain_e = 0.205 * rtb_Gain2_m;

  /* Product: '<S10>/Divide2' incorporates:
   *  Constant: '<S10>/Constant1'
   */
  rtb_Desiredtorquedifferenceatre = rtb_Gain_e / 14.82;

  /* Gain: '<S7>/Gain1' */
  rtb_Gain1_a = 0.5 * rtb_Desiredtorquedifferenceatre;

  /* Gain: '<S14>/Gain' */
  rtb_Gain_m = 0.0 * rtb_Sum;

  /* DeadZone: '<S14>/Dead Zone' */
  if (rtb_Sum > 0.5) {
    rtb_DeadZone_oo = rtb_Sum - 0.5;
  } else if (rtb_Sum >= -0.5) {
    rtb_DeadZone_oo = 0.0;
  } else {
    rtb_DeadZone_oo = rtb_Sum - -0.5;
  }

  /* End of DeadZone: '<S14>/Dead Zone' */

  /* RelationalOperator: '<S14>/NotEqual' */
  rtb_NotEqual = (rtb_Gain_m != rtb_DeadZone_oo);

  /* Signum: '<S14>/Sign' */
  if (rtb_DeadZone_oo < 0.0) {
    rtb_Sign_e = -1.0;
  } else if (rtb_DeadZone_oo > 0.0) {
    rtb_Sign_e = 1.0;
  } else if (rtb_DeadZone_oo == 0.0) {
    rtb_Sign_e = 0.0;
  } else {
    rtb_Sign_e = (rtNaN);
  }

  /* End of Signum: '<S14>/Sign' */

  /* DataTypeConversion: '<S14>/Data Type Conversion' */
  rtb_DataTypeConversion = (int8_T)rtb_Sign_e;

  /* Lookup_n-D: '<S13>/I gains' incorporates:
   *  Inport: '<Root>/bus_Vehicle_velocity'
   */
  rtb_Igains = look1_binlx(rtU.bus_Vehicle_velocity, rtConstP.pooled10,
    rtConstP.Igains_tableData, 12U);

  /* Product: '<S13>/Product' */
  rtb_Product_fk = rtb_DeadZone_j * rtb_Igains;

  /* Signum: '<S14>/Sign1' */
  if (rtb_Product_fk < 0.0) {
    rtb_Sign1 = -1.0;
  } else if (rtb_Product_fk > 0.0) {
    rtb_Sign1 = 1.0;
  } else if (rtb_Product_fk == 0.0) {
    rtb_Sign1 = 0.0;
  } else {
    rtb_Sign1 = (rtNaN);
  }

  /* End of Signum: '<S14>/Sign1' */

  /* DataTypeConversion: '<S14>/Data Type Conversion1' */
  rtb_DataTypeConversion1 = (int8_T)rtb_Sign1;

  /* RelationalOperator: '<S14>/Equal' */
  rtb_Equal = (rtb_DataTypeConversion == rtb_DataTypeConversion1);

  /* Logic: '<S14>/AND' */
  rtb_AND_p = (rtb_NotEqual && rtb_Equal);

  /* Switch: '<S13>/Switch' incorporates:
   *  Constant: '<S13>/Constant'
   */
  if (rtb_AND_p) {
    rtb_Switch = 0.0;
  } else {
    rtb_Switch = rtb_Product_fk;
  }

  /* End of Switch: '<S13>/Switch' */

  /* RelationalOperator: '<S17>/Compare' incorporates:
   *  Constant: '<S17>/Constant'
   *  Inport: '<Root>/bus_Vehicle_velocity'
   */
  rtb_Compare_i = (rtU.bus_Vehicle_velocity > 2.5);

  /* RelationalOperator: '<S18>/Compare' incorporates:
   *  Constant: '<S18>/Constant'
   *  Inport: '<Root>/bus_Pedal_torque_position'
   */
  rtb_Compare_f = (rtU.bus_Pedal_torque_position >= 0.1);

  /* Logic: '<S3>/AND' incorporates:
   *  Inport: '<Root>/bus_Traction_control_active'
   */
  rtb_AND_l = (rtU.bus_Traction_control_active && rtb_Compare_f && rtb_Compare_i);

  /* Product: '<S28>/Product' incorporates:
   *  Constant: '<S28>/Dynamic rolling radius '
   *  Inport: '<Root>/bus_rotation_speed_FL'
   */
  rtb_Product_a = 0.205 * rtU.bus_rotation_speed_FL;

  /* Sum: '<S28>/Minus' incorporates:
   *  Inport: '<Root>/bus_Vehicle_velocity'
   */
  rtb_Minus = rtb_Product_a - rtU.bus_Vehicle_velocity;

  /* MinMax: '<S28>/Max' incorporates:
   *  Inport: '<Root>/bus_Vehicle_velocity'
   */
  rtb_Max = fmax(rtb_Product_a, rtU.bus_Vehicle_velocity);

  /* Fcn: '<S28>/Fcn' */
  rtb_Fcn = ((real_T)(rtb_Max == 0.0) + rtb_Max) + 2.2204460492503131e-16;

  /* Product: '<S28>/Divide' */
  rtb_Divide = rtb_Minus / rtb_Fcn;

  /* MATLAB Function: '<S23>/MATLAB Function' incorporates:
   *  Constant: '<S27>/Constant'
   *  Constant: '<S27>/Constant1'
   */
  MATLABFunction(0.24, 0.2, rtb_Divide, &rtb_SlipActiveFR_e);

  /* Logic: '<S23>/AND' */
  rtb_TCS_active_FL = (rtb_AND_l && (rtb_SlipActiveFR_e != 0.0));

  /* Sum: '<S23>/Minus' incorporates:
   *  Constant: '<S27>/Constant'
   */
  rtb_Minus_ph = rtb_Divide - 0.24;

  /* Saturate: '<S25>/Saturation1' */
  if (rtb_Minus_ph > 1.0) {
    rtb_Saturation1 = 1.0;
  } else if (rtb_Minus_ph < 0.0) {
    rtb_Saturation1 = 0.0;
  } else {
    rtb_Saturation1 = rtb_Minus_ph;
  }

  /* End of Saturate: '<S25>/Saturation1' */

  /* SampleTimeMath: '<S29>/TSamp'
   *
   * About '<S29>/TSamp':
   *  y = u * K where K = 1 / ( w * Ts )
   */
  rtb_TSamp = rtb_Minus_ph * 100.0;

  /* UnitDelay: '<S29>/UD'
   *
   * Block description for '<S29>/UD':
   *
   *  Store in Global RAM
   */
  rtb_Uk1 = rtDW.UD_DSTATE;

  /* Sum: '<S29>/Diff'
   *
   * Block description for '<S29>/Diff':
   *
   *  Add in CPU
   */
  rtb_Diff = rtb_TSamp - rtb_Uk1;

  /* Saturate: '<S25>/Saturation' */
  if (rtb_Diff > 6.0) {
    rtb_Saturation = 6.0;
  } else if (rtb_Diff < -6.0) {
    rtb_Saturation = -6.0;
  } else {
    rtb_Saturation = rtb_Diff;
  }

  /* End of Saturate: '<S25>/Saturation' */

  /* Outputs for Atomic SubSystem: '<S25>/Fuzzy Logic  Controller' */
  /* MATLAB Function: '<S30>/Evaluate Rule Antecedents' */
  rtb_sumAntecedentOutputs_p = 0.0;
  inputMFCache[0] = exp(-((rtb_Saturation1 - -0.00529) * (rtb_Saturation1 -
    -0.00529)) / 0.0182405);
  inputMFCache[1] = exp(-((rtb_Saturation1 - 0.312) * (rtb_Saturation1 - 0.312))
                        / 0.021114074832244877);
  inputMFCache[2] = exp(-((rtb_Saturation1 - 0.631) * (rtb_Saturation1 - 0.631))
                        / 0.025036530807642978);
  inputMFCache[3] = exp(-((rtb_Saturation1 - 1.0) * (rtb_Saturation1 - 1.0)) /
                        0.019105985912503457);
  inputMFCache[4] = exp(-((rtb_Saturation - -6.0) * (rtb_Saturation - -6.0)) /
                        1.2800000000000002);
  inputMFCache[5] = exp(-(rtb_Saturation * rtb_Saturation) / 1.2800000000000002);
  inputMFCache[6] = exp(-((rtb_Saturation - 6.0) * (rtb_Saturation - 6.0)) /
                        1.2800000000000002);
  inputMFCache[7] = exp(-((rtb_Saturation - -4.0) * (rtb_Saturation - -4.0)) /
                        1.2800000000000002);
  inputMFCache[8] = exp(-((rtb_Saturation - 2.0) * (rtb_Saturation - 2.0)) /
                        1.2800000000000002);
  inputMFCache[9] = exp(-((rtb_Saturation - 4.0) * (rtb_Saturation - 4.0)) /
                        1.2800000000000002);
  inputMFCache[10] = exp(-((rtb_Saturation - -2.0) * (rtb_Saturation - -2.0)) /
    1.2800000000000002);
  for (ruleID = 0; ruleID < 28; ruleID++) {
    area = inputMFCache[b[ruleID] - 1];
    if (!(1.0 > area)) {
      area = 1.0;
    }

    x_idx_0 = area;
    area = inputMFCache[b[ruleID + 28] + 3];
    if (!(x_idx_0 > area)) {
      area = x_idx_0;
    }

    rtb_sumAntecedentOutputs_p += area;
    rtb_antecedentOutputs_f[ruleID] = area;
  }

  /* End of MATLAB Function: '<S30>/Evaluate Rule Antecedents' */

  /* MATLAB Function: '<S30>/Evaluate Rule Consequents' incorporates:
   *  Constant: '<S30>/Output Sample Points'
   */
  EvaluateRuleConsequents(rtb_antecedentOutputs_f, rtConstP.pooled2,
    rtb_aggregatedOutputs_bd);

  /* MATLAB Function: '<S30>/Defuzzify Outputs' incorporates:
   *  Constant: '<S30>/Output Sample Points'
   */
  if (rtb_sumAntecedentOutputs_p == 0.0) {
    rtb_defuzzifiedOutputs_p = 0.5;
  } else {
    rtb_defuzzifiedOutputs_p = 0.0;
    area = 0.0;
    for (ruleID = 0; ruleID < 101; ruleID++) {
      area += rtb_aggregatedOutputs_bd[ruleID];
    }

    if (area == 0.0) {
      rtb_defuzzifiedOutputs_p = 0.5;
    } else {
      for (ruleID = 0; ruleID < 101; ruleID++) {
        rtb_defuzzifiedOutputs_p += rtConstP.pooled2[ruleID] *
          rtb_aggregatedOutputs_bd[ruleID];
      }

      rtb_defuzzifiedOutputs_p *= 1.0 / area;
    }
  }

  /* End of MATLAB Function: '<S30>/Defuzzify Outputs' */
  /* End of Outputs for SubSystem: '<S25>/Fuzzy Logic  Controller' */

  /* Product: '<S23>/Product' */
  rtb_Product_lr = (real_T)rtb_TCS_active_FL * rtb_defuzzifiedOutputs_p;

  /* Product: '<S16>/Product' incorporates:
   *  Inport: '<Root>/bus_Torque_FL'
   */
  rtb_Product_dt = rtU.bus_Torque_FL * rtb_Product_lr;

  /* Product: '<S40>/Product' incorporates:
   *  Constant: '<S40>/Dynamic rolling radius '
   *  Inport: '<Root>/bus_rotation_speed_FR'
   */
  rtb_Product_ny = 0.205 * rtU.bus_rotation_speed_FR;

  /* Sum: '<S40>/Minus' incorporates:
   *  Inport: '<Root>/bus_Vehicle_velocity'
   */
  rtb_Minus_d = rtb_Product_ny - rtU.bus_Vehicle_velocity;

  /* MinMax: '<S40>/Max' incorporates:
   *  Inport: '<Root>/bus_Vehicle_velocity'
   */
  rtb_Max_b = fmax(rtb_Product_ny, rtU.bus_Vehicle_velocity);

  /* Fcn: '<S40>/Fcn' */
  rtb_Fcn_a = ((real_T)(rtb_Max_b == 0.0) + rtb_Max_b) + 2.2204460492503131e-16;

  /* Product: '<S40>/Divide' */
  rtb_Divide_k = rtb_Minus_d / rtb_Fcn_a;

  /* MATLAB Function: '<S35>/MATLAB Function' incorporates:
   *  Constant: '<S39>/Constant'
   *  Constant: '<S39>/Constant1'
   */
  MATLABFunction(0.24, 0.2, rtb_Divide_k, &rtb_SlipActiveFR);

  /* Logic: '<S35>/AND' */
  rtb_TCS_active_FR = (rtb_AND_l && (rtb_SlipActiveFR != 0.0));

  /* Sum: '<S35>/Minus' incorporates:
   *  Constant: '<S39>/Constant'
   */
  rtb_Minus_p = rtb_Divide_k - 0.24;

  /* Saturate: '<S37>/Saturation1' */
  if (rtb_Minus_p > 1.0) {
    rtb_Saturation1_l = 1.0;
  } else if (rtb_Minus_p < 0.0) {
    rtb_Saturation1_l = 0.0;
  } else {
    rtb_Saturation1_l = rtb_Minus_p;
  }

  /* End of Saturate: '<S37>/Saturation1' */

  /* SampleTimeMath: '<S41>/TSamp'
   *
   * About '<S41>/TSamp':
   *  y = u * K where K = 1 / ( w * Ts )
   */
  rtb_TSamp_a = rtb_Minus_p * 100.0;

  /* UnitDelay: '<S41>/UD'
   *
   * Block description for '<S41>/UD':
   *
   *  Store in Global RAM
   */
  rtb_Uk1_b = rtDW.UD_DSTATE_a;

  /* Sum: '<S41>/Diff'
   *
   * Block description for '<S41>/Diff':
   *
   *  Add in CPU
   */
  rtb_Diff_o = rtb_TSamp_a - rtb_Uk1_b;

  /* Saturate: '<S37>/Saturation' */
  if (rtb_Diff_o > 6.0) {
    rtb_Saturation_g = 6.0;
  } else if (rtb_Diff_o < -6.0) {
    rtb_Saturation_g = -6.0;
  } else {
    rtb_Saturation_g = rtb_Diff_o;
  }

  /* End of Saturate: '<S37>/Saturation' */

  /* Outputs for Atomic SubSystem: '<S37>/Fuzzy Logic  Controller' */
  /* MATLAB Function: '<S42>/Evaluate Rule Antecedents' */
  rtb_sumAntecedentOutputs_c = 0.0;
  inputMFCache[0] = exp(-((rtb_Saturation1_l - -0.00529) * (rtb_Saturation1_l -
    -0.00529)) / 0.0182405);
  inputMFCache[1] = exp(-((rtb_Saturation1_l - 0.312) * (rtb_Saturation1_l -
    0.312)) / 0.021114074832244877);
  inputMFCache[2] = exp(-((rtb_Saturation1_l - 0.631) * (rtb_Saturation1_l -
    0.631)) / 0.025036530807642978);
  inputMFCache[3] = exp(-((rtb_Saturation1_l - 1.0) * (rtb_Saturation1_l - 1.0))
                        / 0.019105985912503457);
  inputMFCache[4] = exp(-((rtb_Saturation_g - -6.0) * (rtb_Saturation_g - -6.0))
                        / 1.2800000000000002);
  inputMFCache[5] = exp(-(rtb_Saturation_g * rtb_Saturation_g) /
                        1.2800000000000002);
  inputMFCache[6] = exp(-((rtb_Saturation_g - 6.0) * (rtb_Saturation_g - 6.0)) /
                        1.2800000000000002);
  inputMFCache[7] = exp(-((rtb_Saturation_g - -4.0) * (rtb_Saturation_g - -4.0))
                        / 1.2800000000000002);
  inputMFCache[8] = exp(-((rtb_Saturation_g - 2.0) * (rtb_Saturation_g - 2.0)) /
                        1.2800000000000002);
  inputMFCache[9] = exp(-((rtb_Saturation_g - 4.0) * (rtb_Saturation_g - 4.0)) /
                        1.2800000000000002);
  inputMFCache[10] = exp(-((rtb_Saturation_g - -2.0) * (rtb_Saturation_g - -2.0))
    / 1.2800000000000002);
  for (ruleID = 0; ruleID < 28; ruleID++) {
    area = inputMFCache[b[ruleID] - 1];
    if (!(1.0 > area)) {
      area = 1.0;
    }

    x_idx_0 = area;
    area = inputMFCache[b[ruleID + 28] + 3];
    if (!(x_idx_0 > area)) {
      area = x_idx_0;
    }

    rtb_sumAntecedentOutputs_c += area;
    rtb_antecedentOutputs_o[ruleID] = area;
  }

  /* End of MATLAB Function: '<S42>/Evaluate Rule Antecedents' */

  /* MATLAB Function: '<S42>/Evaluate Rule Consequents' incorporates:
   *  Constant: '<S42>/Output Sample Points'
   */
  EvaluateRuleConsequents(rtb_antecedentOutputs_o, rtConstP.pooled2,
    rtb_aggregatedOutputs_n);

  /* MATLAB Function: '<S42>/Defuzzify Outputs' incorporates:
   *  Constant: '<S42>/Output Sample Points'
   */
  if (rtb_sumAntecedentOutputs_c == 0.0) {
    rtb_defuzzifiedOutputs_o = 0.5;
  } else {
    rtb_defuzzifiedOutputs_o = 0.0;
    area = 0.0;
    for (ruleID = 0; ruleID < 101; ruleID++) {
      area += rtb_aggregatedOutputs_n[ruleID];
    }

    if (area == 0.0) {
      rtb_defuzzifiedOutputs_o = 0.5;
    } else {
      for (ruleID = 0; ruleID < 101; ruleID++) {
        rtb_defuzzifiedOutputs_o += rtConstP.pooled2[ruleID] *
          rtb_aggregatedOutputs_n[ruleID];
      }

      rtb_defuzzifiedOutputs_o *= 1.0 / area;
    }
  }

  /* End of MATLAB Function: '<S42>/Defuzzify Outputs' */
  /* End of Outputs for SubSystem: '<S37>/Fuzzy Logic  Controller' */

  /* Product: '<S35>/Product' */
  rtb_Product_e = (real_T)rtb_TCS_active_FR * rtb_defuzzifiedOutputs_o;

  /* Product: '<S16>/Product1' incorporates:
   *  Inport: '<Root>/bus_Torque_FR'
   */
  rtb_Product1_a = rtU.bus_Torque_FR * rtb_Product_e;

  /* Product: '<S52>/Product' incorporates:
   *  Constant: '<S52>/Dynamic rolling radius '
   *  Inport: '<Root>/bus_rotation_speed_RL'
   */
  rtb_Product_mk = 0.205 * rtU.bus_rotation_speed_RL;

  /* Sum: '<S52>/Minus' incorporates:
   *  Inport: '<Root>/bus_Vehicle_velocity'
   */
  rtb_Minus_m = rtb_Product_mk - rtU.bus_Vehicle_velocity;

  /* MinMax: '<S52>/Max' incorporates:
   *  Inport: '<Root>/bus_Vehicle_velocity'
   */
  rtb_Max_j = fmax(rtb_Product_mk, rtU.bus_Vehicle_velocity);

  /* Fcn: '<S52>/Fcn' */
  rtb_Fcn_g = ((real_T)(rtb_Max_j == 0.0) + rtb_Max_j) + 2.2204460492503131e-16;

  /* Product: '<S52>/Divide' */
  rtb_Divide_a = rtb_Minus_m / rtb_Fcn_g;

  /* MATLAB Function: '<S47>/MATLAB Function' incorporates:
   *  Constant: '<S51>/Constant'
   */
  rtb_SlipActiveRL = 0;
  if (rtb_Divide_a > 0.24) {
    rtb_SlipActiveRL = 1;
  }

  /* End of MATLAB Function: '<S47>/MATLAB Function' */

  /* Logic: '<S47>/AND' */
  rtb_TCS_active_RL = (rtb_AND_l && (rtb_SlipActiveRL != 0));

  /* Sum: '<S47>/Minus' incorporates:
   *  Constant: '<S51>/Constant'
   */
  rtb_Minus_i3 = rtb_Divide_a - 0.24;

  /* Saturate: '<S49>/Saturation1' */
  if (rtb_Minus_i3 > 1.0) {
    rtb_Saturation1_h = 1.0;
  } else if (rtb_Minus_i3 < 0.0) {
    rtb_Saturation1_h = 0.0;
  } else {
    rtb_Saturation1_h = rtb_Minus_i3;
  }

  /* End of Saturate: '<S49>/Saturation1' */

  /* SampleTimeMath: '<S53>/TSamp'
   *
   * About '<S53>/TSamp':
   *  y = u * K where K = 1 / ( w * Ts )
   */
  rtb_TSamp_n = rtb_Minus_i3 * 100.0;

  /* UnitDelay: '<S53>/UD'
   *
   * Block description for '<S53>/UD':
   *
   *  Store in Global RAM
   */
  rtb_Uk1_c = rtDW.UD_DSTATE_j;

  /* Sum: '<S53>/Diff'
   *
   * Block description for '<S53>/Diff':
   *
   *  Add in CPU
   */
  rtb_Diff_ns = rtb_TSamp_n - rtb_Uk1_c;

  /* Saturate: '<S49>/Saturation' */
  if (rtb_Diff_ns > 6.0) {
    rtb_Saturation_l = 6.0;
  } else if (rtb_Diff_ns < -6.0) {
    rtb_Saturation_l = -6.0;
  } else {
    rtb_Saturation_l = rtb_Diff_ns;
  }

  /* End of Saturate: '<S49>/Saturation' */

  /* Outputs for Atomic SubSystem: '<S49>/Fuzzy Logic  Controller' */
  /* MATLAB Function: '<S54>/Evaluate Rule Antecedents' */
  rtb_sumAntecedentOutputs_b = 0.0;
  inputMFCache[0] = exp(-((rtb_Saturation1_h - -0.00529) * (rtb_Saturation1_h -
    -0.00529)) / 0.0182405);
  inputMFCache[1] = exp(-((rtb_Saturation1_h - 0.312) * (rtb_Saturation1_h -
    0.312)) / 0.021114074832244877);
  inputMFCache[2] = exp(-((rtb_Saturation1_h - 0.631) * (rtb_Saturation1_h -
    0.631)) / 0.025036530807642978);
  inputMFCache[3] = exp(-((rtb_Saturation1_h - 1.0) * (rtb_Saturation1_h - 1.0))
                        / 0.019105985912503457);
  inputMFCache[4] = exp(-((rtb_Saturation_l - -6.0) * (rtb_Saturation_l - -6.0))
                        / 1.2800000000000002);
  inputMFCache[5] = exp(-(rtb_Saturation_l * rtb_Saturation_l) /
                        1.2800000000000002);
  inputMFCache[6] = exp(-((rtb_Saturation_l - 6.0) * (rtb_Saturation_l - 6.0)) /
                        1.2800000000000002);
  inputMFCache[7] = exp(-((rtb_Saturation_l - -4.0) * (rtb_Saturation_l - -4.0))
                        / 1.2800000000000002);
  inputMFCache[8] = exp(-((rtb_Saturation_l - 2.0) * (rtb_Saturation_l - 2.0)) /
                        1.2800000000000002);
  inputMFCache[9] = exp(-((rtb_Saturation_l - 4.0) * (rtb_Saturation_l - 4.0)) /
                        1.2800000000000002);
  inputMFCache[10] = exp(-((rtb_Saturation_l - -2.0) * (rtb_Saturation_l - -2.0))
    / 1.2800000000000002);
  for (ruleID = 0; ruleID < 28; ruleID++) {
    area = inputMFCache[b[ruleID] - 1];
    if (!(1.0 > area)) {
      area = 1.0;
    }

    x_idx_0 = area;
    area = inputMFCache[b[ruleID + 28] + 3];
    if (!(x_idx_0 > area)) {
      area = x_idx_0;
    }

    rtb_sumAntecedentOutputs_b += area;
    rtb_antecedentOutputs_g[ruleID] = area;
  }

  /* End of MATLAB Function: '<S54>/Evaluate Rule Antecedents' */

  /* MATLAB Function: '<S54>/Evaluate Rule Consequents' incorporates:
   *  Constant: '<S54>/Output Sample Points'
   */
  EvaluateRuleConsequents(rtb_antecedentOutputs_g, rtConstP.pooled2,
    rtb_aggregatedOutputs_d);

  /* MATLAB Function: '<S54>/Defuzzify Outputs' incorporates:
   *  Constant: '<S54>/Output Sample Points'
   */
  if (rtb_sumAntecedentOutputs_b == 0.0) {
    rtb_defuzzifiedOutputs_ds = 0.5;
  } else {
    rtb_defuzzifiedOutputs_ds = 0.0;
    area = 0.0;
    for (ruleID = 0; ruleID < 101; ruleID++) {
      area += rtb_aggregatedOutputs_d[ruleID];
    }

    if (area == 0.0) {
      rtb_defuzzifiedOutputs_ds = 0.5;
    } else {
      for (ruleID = 0; ruleID < 101; ruleID++) {
        rtb_defuzzifiedOutputs_ds += rtConstP.pooled2[ruleID] *
          rtb_aggregatedOutputs_d[ruleID];
      }

      rtb_defuzzifiedOutputs_ds *= 1.0 / area;
    }
  }

  /* End of MATLAB Function: '<S54>/Defuzzify Outputs' */
  /* End of Outputs for SubSystem: '<S49>/Fuzzy Logic  Controller' */

  /* Product: '<S47>/Product' */
  rtb_Product_fz = (real_T)rtb_TCS_active_RL * rtb_defuzzifiedOutputs_ds;

  /* Product: '<S16>/Product2' incorporates:
   *  Inport: '<Root>/bus_Torque_RL'
   */
  rtb_Product2_h = rtU.bus_Torque_RL * rtb_Product_fz;

  /* Product: '<S64>/Product' incorporates:
   *  Constant: '<S64>/Dynamic rolling radius '
   *  Inport: '<Root>/bus_rotation_speed_RR'
   */
  rtb_Product_c = 0.205 * rtU.bus_rotation_speed_RR;

  /* Sum: '<S64>/Minus' incorporates:
   *  Inport: '<Root>/bus_Vehicle_velocity'
   */
  rtb_Minus_kk = rtb_Product_c - rtU.bus_Vehicle_velocity;

  /* MinMax: '<S64>/Max' incorporates:
   *  Inport: '<Root>/bus_Vehicle_velocity'
   */
  rtb_Max_hv = fmax(rtb_Product_c, rtU.bus_Vehicle_velocity);

  /* Fcn: '<S64>/Fcn' */
  rtb_Fcn_p = ((real_T)(rtb_Max_hv == 0.0) + rtb_Max_hv) +
    2.2204460492503131e-16;

  /* Product: '<S64>/Divide' */
  rtb_Divide_bk = rtb_Minus_kk / rtb_Fcn_p;

  /* MATLAB Function: '<S59>/MATLAB Function' incorporates:
   *  Constant: '<S63>/Constant'
   */
  rtb_SlipActiveRR = 0;
  if (rtb_Divide_bk > 0.24) {
    rtb_SlipActiveRR = 1;
  }

  /* End of MATLAB Function: '<S59>/MATLAB Function' */

  /* Logic: '<S59>/AND' */
  rtb_TCS_active_RR = (rtb_AND_l && (rtb_SlipActiveRR != 0));

  /* Sum: '<S59>/Minus' incorporates:
   *  Constant: '<S63>/Constant'
   */
  rtb_Minus_e = rtb_Divide_bk - 0.24;

  /* Saturate: '<S61>/Saturation1' */
  if (rtb_Minus_e > 1.0) {
    rtb_Saturation1_d = 1.0;
  } else if (rtb_Minus_e < 0.0) {
    rtb_Saturation1_d = 0.0;
  } else {
    rtb_Saturation1_d = rtb_Minus_e;
  }

  /* End of Saturate: '<S61>/Saturation1' */

  /* SampleTimeMath: '<S65>/TSamp'
   *
   * About '<S65>/TSamp':
   *  y = u * K where K = 1 / ( w * Ts )
   */
  rtb_TSamp_k = rtb_Minus_e * 100.0;

  /* UnitDelay: '<S65>/UD'
   *
   * Block description for '<S65>/UD':
   *
   *  Store in Global RAM
   */
  rtb_Uk1_m = rtDW.UD_DSTATE_g;

  /* Sum: '<S65>/Diff'
   *
   * Block description for '<S65>/Diff':
   *
   *  Add in CPU
   */
  rtb_Diff_i = rtb_TSamp_k - rtb_Uk1_m;

  /* Saturate: '<S61>/Saturation' */
  if (rtb_Diff_i > 6.0) {
    rtb_Saturation_n = 6.0;
  } else if (rtb_Diff_i < -6.0) {
    rtb_Saturation_n = -6.0;
  } else {
    rtb_Saturation_n = rtb_Diff_i;
  }

  /* End of Saturate: '<S61>/Saturation' */

  /* Outputs for Atomic SubSystem: '<S61>/Fuzzy Logic  Controller' */
  /* MATLAB Function: '<S66>/Evaluate Rule Antecedents' */
  rtb_sumAntecedentOutputs = 0.0;
  inputMFCache[0] = exp(-((rtb_Saturation1_d - -0.00529) * (rtb_Saturation1_d -
    -0.00529)) / 0.0182405);
  inputMFCache[1] = exp(-((rtb_Saturation1_d - 0.312) * (rtb_Saturation1_d -
    0.312)) / 0.021114074832244877);
  inputMFCache[2] = exp(-((rtb_Saturation1_d - 0.631) * (rtb_Saturation1_d -
    0.631)) / 0.025036530807642978);
  inputMFCache[3] = exp(-((rtb_Saturation1_d - 1.0) * (rtb_Saturation1_d - 1.0))
                        / 0.019105985912503457);
  inputMFCache[4] = exp(-((rtb_Saturation_n - -6.0) * (rtb_Saturation_n - -6.0))
                        / 1.2800000000000002);
  inputMFCache[5] = exp(-(rtb_Saturation_n * rtb_Saturation_n) /
                        1.2800000000000002);
  inputMFCache[6] = exp(-((rtb_Saturation_n - 6.0) * (rtb_Saturation_n - 6.0)) /
                        1.2800000000000002);
  inputMFCache[7] = exp(-((rtb_Saturation_n - -4.0) * (rtb_Saturation_n - -4.0))
                        / 1.2800000000000002);
  inputMFCache[8] = exp(-((rtb_Saturation_n - 2.0) * (rtb_Saturation_n - 2.0)) /
                        1.2800000000000002);
  inputMFCache[9] = exp(-((rtb_Saturation_n - 4.0) * (rtb_Saturation_n - 4.0)) /
                        1.2800000000000002);
  inputMFCache[10] = exp(-((rtb_Saturation_n - -2.0) * (rtb_Saturation_n - -2.0))
    / 1.2800000000000002);
  for (ruleID = 0; ruleID < 28; ruleID++) {
    area = inputMFCache[b[ruleID] - 1];
    if (!(1.0 > area)) {
      area = 1.0;
    }

    x_idx_0 = area;
    area = inputMFCache[b[ruleID + 28] + 3];
    if (!(x_idx_0 > area)) {
      area = x_idx_0;
    }

    rtb_sumAntecedentOutputs += area;
    rtb_antecedentOutputs[ruleID] = area;
  }

  /* End of MATLAB Function: '<S66>/Evaluate Rule Antecedents' */

  /* MATLAB Function: '<S66>/Evaluate Rule Consequents' incorporates:
   *  Constant: '<S66>/Output Sample Points'
   */
  EvaluateRuleConsequents(rtb_antecedentOutputs, rtConstP.pooled2,
    rtb_aggregatedOutputs);

  /* MATLAB Function: '<S66>/Defuzzify Outputs' incorporates:
   *  Constant: '<S66>/Output Sample Points'
   */
  if (rtb_sumAntecedentOutputs == 0.0) {
    rtb_defuzzifiedOutputs = 0.5;
  } else {
    rtb_defuzzifiedOutputs = 0.0;
    area = 0.0;
    for (ruleID = 0; ruleID < 101; ruleID++) {
      area += rtb_aggregatedOutputs[ruleID];
    }

    if (area == 0.0) {
      rtb_defuzzifiedOutputs = 0.5;
    } else {
      for (ruleID = 0; ruleID < 101; ruleID++) {
        rtb_defuzzifiedOutputs += rtConstP.pooled2[ruleID] *
          rtb_aggregatedOutputs[ruleID];
      }

      rtb_defuzzifiedOutputs *= 1.0 / area;
    }
  }

  /* End of MATLAB Function: '<S66>/Defuzzify Outputs' */
  /* End of Outputs for SubSystem: '<S61>/Fuzzy Logic  Controller' */

  /* Product: '<S59>/Product' */
  rtb_Product_h = (real_T)rtb_TCS_active_RR * rtb_defuzzifiedOutputs;

  /* Product: '<S16>/Product3' incorporates:
   *  Inport: '<Root>/bus_Torque_RR'
   */
  rtb_Product3 = rtU.bus_Torque_RR * rtb_Product_h;

  /* Logic: '<S3>/AND1' incorporates:
   *  Inport: '<Root>/bus_Velocity_control_active'
   */
  rtb_AND1 = (rtb_AND_l && rtU.bus_Velocity_control_active);

  /* Product: '<S34>/Divide' incorporates:
   *  Constant: '<S34>/Constant'
   *  Inport: '<Root>/bus_Vehicle_velocity'
   */
  rtb_Divide_i = rtU.bus_Vehicle_velocity / 0.205;

  /* Product: '<S24>/Product' */
  rtb_Product_m = rtb_Divide_i * 1.24;

  /* Product: '<S24>/Product2' incorporates:
   *  Constant: '<S24>/Constant2'
   *  Constant: '<S24>/Constant5'
   */

  /* Product: '<S24>/Divide' */
  rtb_Divide_m4 = rtb_Product_m * 117.933812831 / 0.6440264;

  /* Product: '<S24>/Product3' */
  rtb_Product3_na = (real_T)rtb_AND1 * rtb_Divide_m4;

  /* Saturate: '<S24>/Limit to max RPM' */
  if (rtb_Product3_na > 20000.0) {
    rtb_LimittomaxRPM = 20000.0;
  } else if (rtb_Product3_na < 0.0) {
    rtb_LimittomaxRPM = 0.0;
  } else {
    rtb_LimittomaxRPM = rtb_Product3_na;
  }

  /* End of Saturate: '<S24>/Limit to max RPM' */

  /* Product: '<S46>/Divide' incorporates:
   *  Constant: '<S46>/Constant'
   *  Inport: '<Root>/bus_Vehicle_velocity'
   */
  rtb_Divide_j = rtU.bus_Vehicle_velocity / 0.205;

  /* Product: '<S36>/Product' */
  rtb_Product_mu = rtb_Divide_j * 1.24;


  /* Product: '<S36>/Divide' */
  rtb_Divide_f = rtb_Product_mu * 117.933812831 / 0.6440264;

  /* Product: '<S36>/Product3' */
  rtb_Product3_l = (real_T)rtb_AND1 * rtb_Divide_f;

  /* Saturate: '<S36>/Limit to max RPM' */
  if (rtb_Product3_l > 20000.0) {
    rtb_LimittomaxRPM_d = 20000.0;
  } else if (rtb_Product3_l < 0.0) {
    rtb_LimittomaxRPM_d = 0.0;
  } else {
    rtb_LimittomaxRPM_d = rtb_Product3_l;
  }

  /* End of Saturate: '<S36>/Limit to max RPM' */

  /* Product: '<S58>/Divide' incorporates:
   *  Constant: '<S58>/Constant'
   *  Inport: '<Root>/bus_Vehicle_velocity'
   */
  rtb_Divide_m = rtU.bus_Vehicle_velocity / 0.205;

  /* Product: '<S48>/Product' */
  rtb_Product_g = rtb_Divide_m * 1.24;

  /* Product: '<S48>/Divide' */
  rtb_Divide_k0 = rtb_Product_g * 117.933812831 / 0.6440264;

  /* Product: '<S48>/Product3' */
  rtb_Product3_o = (real_T)rtb_AND1 * rtb_Divide_k0;

  /* Saturate: '<S48>/Limit to max RPM' */
  if (rtb_Product3_o > 20000.0) {
    rtb_LimittomaxRPM_a = 20000.0;
  } else if (rtb_Product3_o < 0.0) {
    rtb_LimittomaxRPM_a = 0.0;
  } else {
    rtb_LimittomaxRPM_a = rtb_Product3_o;
  }

  /* End of Saturate: '<S48>/Limit to max RPM' */

  /* Product: '<S70>/Divide' incorporates:
   *  Constant: '<S70>/Constant'
   *  Inport: '<Root>/bus_Vehicle_velocity'
   */
  rtb_Divide_h = rtU.bus_Vehicle_velocity / 0.205;

  /* Product: '<S60>/Product' */
  rtb_Product_o = rtb_Divide_h * 1.24;

  rtb_Product2_f = rtb_Product_o * 30.0 * 12.35;

  /* Product: '<S60>/Divide' */
  rtb_Divide_ak = rtb_Product2_f * 117.933812831 / 0.6440264;

  /* Product: '<S60>/Product3' */
  rtb_Product3_j = (real_T)rtb_AND1 * rtb_Divide_ak;

  /* Saturate: '<S60>/Limit to max RPM' */
  if (rtb_Product3_j > 20000.0) {
    rtb_LimittomaxRPM_n = 20000.0;
  } else if (rtb_Product3_j < 0.0) {
    rtb_LimittomaxRPM_n = 0.0;
  } else {
    rtb_LimittomaxRPM_n = rtb_Product3_j;
  }

  /* End of Saturate: '<S60>/Limit to max RPM' */

  /* RelationalOperator: '<S71>/Relational Operator' incorporates:
   *  Constant: '<S71>/Lower limit'
   *  Inport: '<Root>/bus_Vehicle_acceleration'
   */
  rtb_RelationalOperator = (-23.0 <= rtU.bus_Vehicle_acceleration);

  /* RelationalOperator: '<S71>/Relational Operator1' incorporates:
   *  Constant: '<S71>/Upper limit'
   *  Inport: '<Root>/bus_Vehicle_acceleration'
   */
  rtb_RelationalOperator1 = (rtU.bus_Vehicle_acceleration <= 23.0);

  /* Logic: '<S71>/AND' */
  rtb_AND_hu = (rtb_RelationalOperator && rtb_RelationalOperator1);

  /* RelationalOperator: '<S72>/Relational Operator' incorporates:
   *  Constant: '<S72>/Lower limit'
   *  Inport: '<Root>/bus_Vehicle_velocity'
   */
  rtb_RelationalOperator_i4 = (0.0 <= rtU.bus_Vehicle_velocity);

  /* RelationalOperator: '<S72>/Relational Operator1' incorporates:
   *  Constant: '<S72>/Upper limit'
   *  Inport: '<Root>/bus_Vehicle_velocity'
   */
  rtb_RelationalOperator1_eh = (rtU.bus_Vehicle_velocity <= 40.0);

  /* Logic: '<S72>/AND' */
  rtb_AND_oh = (rtb_RelationalOperator_i4 && rtb_RelationalOperator1_eh);

  /* RelationalOperator: '<S73>/Relational Operator' incorporates:
   *  Constant: '<S73>/Lower limit'
   *  Inport: '<Root>/bus_rotation_speed_FL'
   */
  rtb_RelationalOperator_f = (0.0 <= rtU.bus_rotation_speed_FL);

  /* RelationalOperator: '<S73>/Relational Operator1' incorporates:
   *  Constant: '<S73>/Upper limit'
   *  Inport: '<Root>/bus_rotation_speed_FL'
   */
  rtb_RelationalOperator1_ao = (rtU.bus_rotation_speed_FL <= 175.0);

  /* Logic: '<S73>/AND' */
  rtb_AND_f = (rtb_RelationalOperator_f && rtb_RelationalOperator1_ao);

  /* RelationalOperator: '<S74>/Relational Operator' incorporates:
   *  Constant: '<S74>/Lower limit'
   *  Inport: '<Root>/bus_rotation_speed_FR'
   */
  rtb_RelationalOperator_ie = (0.0 <= rtU.bus_rotation_speed_FR);

  /* RelationalOperator: '<S74>/Relational Operator1' incorporates:
   *  Constant: '<S74>/Upper limit'
   *  Inport: '<Root>/bus_rotation_speed_FR'
   */
  rtb_RelationalOperator1_d = (rtU.bus_rotation_speed_FR <= 175.0);

  /* Logic: '<S74>/AND' */
  rtb_AND_i = (rtb_RelationalOperator_ie && rtb_RelationalOperator1_d);

  /* RelationalOperator: '<S75>/Relational Operator' incorporates:
   *  Constant: '<S75>/Lower limit'
   *  Inport: '<Root>/bus_rotation_speed_RL'
   */
  rtb_RelationalOperator_g = (0.0 <= rtU.bus_rotation_speed_RL);

  /* RelationalOperator: '<S75>/Relational Operator1' incorporates:
   *  Constant: '<S75>/Upper limit'
   *  Inport: '<Root>/bus_rotation_speed_RL'
   */
  rtb_RelationalOperator1_ks = (rtU.bus_rotation_speed_RL <= 175.0);

  /* Logic: '<S75>/AND' */
  rtb_AND_am = (rtb_RelationalOperator_g && rtb_RelationalOperator1_ks);

  /* RelationalOperator: '<S76>/Relational Operator' incorporates:
   *  Constant: '<S76>/Lower limit'
   *  Inport: '<Root>/bus_rotation_speed_RR'
   */
  rtb_RelationalOperator_n = (0.0 <= rtU.bus_rotation_speed_RR);

  /* RelationalOperator: '<S76>/Relational Operator1' incorporates:
   *  Constant: '<S76>/Upper limit'
   *  Inport: '<Root>/bus_rotation_speed_RR'
   */
  rtb_RelationalOperator1_gj = (rtU.bus_rotation_speed_RR <= 175.0);

  /* Logic: '<S76>/AND' */
  rtb_AND_c = (rtb_RelationalOperator_n && rtb_RelationalOperator1_gj);

  /* RelationalOperator: '<S77>/Relational Operator' incorporates:
   *  Constant: '<S77>/Lower limit'
   *  Inport: '<Root>/bus_Vehicle_str_ang'
   */
  rtb_RelationalOperator_fd = (-145.0 <= rtU.bus_Vehicle_str_ang);

  /* RelationalOperator: '<S77>/Relational Operator1' incorporates:
   *  Constant: '<S77>/Upper limit'
   *  Inport: '<Root>/bus_Vehicle_str_ang'
   */
  rtb_RelationalOperator1_i = (rtU.bus_Vehicle_str_ang <= 145.0);

  /* Logic: '<S77>/AND' */
  rtb_AND_k3 = (rtb_RelationalOperator_fd && rtb_RelationalOperator1_i);

  /* RelationalOperator: '<S78>/Relational Operator' incorporates:
   *  Constant: '<S78>/Lower limit'
   *  Inport: '<Root>/bus_Vehicle_yaw_rate'
   */
  rtb_RelationalOperator_d = (-2.094 <= rtU.bus_Vehicle_yaw_rate);

  /* RelationalOperator: '<S78>/Relational Operator1' incorporates:
   *  Constant: '<S78>/Upper limit'
   *  Inport: '<Root>/bus_Vehicle_yaw_rate'
   */
  rtb_RelationalOperator1_e = (rtU.bus_Vehicle_yaw_rate <= 2.094);

  /* Logic: '<S78>/AND' */
  rtb_AND_g = (rtb_RelationalOperator_d && rtb_RelationalOperator1_e);

  /* Update for DiscreteIntegrator: '<S13>/Discrete-Time Integrator' */
  rtDW.DiscreteTimeIntegrator_DSTATE += 0.01 * rtb_Switch;

  /* Update for UnitDelay: '<S29>/UD'
   *
   * Block description for '<S29>/UD':
   *
   *  Store in Global RAM
   */
  rtDW.UD_DSTATE = rtb_TSamp;

  /* Update for UnitDelay: '<S41>/UD'
   *
   * Block description for '<S41>/UD':
   *
   *  Store in Global RAM
   */
  rtDW.UD_DSTATE_a = rtb_TSamp_a;

  /* Update for UnitDelay: '<S53>/UD'
   *
   * Block description for '<S53>/UD':
   *
   *  Store in Global RAM
   */
  rtDW.UD_DSTATE_j = rtb_TSamp_n;

  /* Update for UnitDelay: '<S65>/UD'
   *
   * Block description for '<S65>/UD':
   *
   *  Store in Global RAM
   */
  rtDW.UD_DSTATE_g = rtb_TSamp_k;

  /* End of Outputs for SubSystem: '<Root>/TV_TC_Controller' */

  /* Outport: '<Root>/TCS_TCS_FL' */
  rtY.TCS_TCS_FL = rtb_Product_dt;

  /* Outport: '<Root>/TCS_TCS_FR' */
  rtY.TCS_TCS_FR = rtb_Product1_a;

  /* Outport: '<Root>/TCS_TCS_RL' */
  rtY.TCS_TCS_RL = rtb_Product2_h;

  /* Outport: '<Root>/TCS_TCS_RR' */
  rtY.TCS_TCS_RR = rtb_Product3;

  /* Outport: '<Root>/TCS_RPMmaxFL' */
  rtY.TCS_RPMmaxFL = rtb_LimittomaxRPM;

  /* Outport: '<Root>/TCS_RPMmaxFR' */
  rtY.TCS_RPMmaxFR = rtb_LimittomaxRPM_d;

  /* Outport: '<Root>/TCS_RPMmaxRL' */
  rtY.TCS_RPMmaxRL = rtb_LimittomaxRPM_a;

  /* Outport: '<Root>/TCS_RPMmaxRR' */
  rtY.TCS_RPMmaxRR = rtb_LimittomaxRPM_n;

  /* Outport: '<Root>/Errors_VelocityError' */
  rtY.Errors_VelocityError = rtb_AND_oh;

  /* Outport: '<Root>/Errors_AccelerationError' */
  rtY.Errors_AccelerationError = rtb_AND_hu;

  /* Outport: '<Root>/Errors_rotationErrorFL' */
  rtY.Errors_rotationErrorFL = rtb_AND_f;

  /* Outport: '<Root>/Errors_rotationErrorFR' */
  rtY.Errors_rotationErrorFR = rtb_AND_i;

  /* Outport: '<Root>/Errors_rotationErrorRL' */
  rtY.Errors_rotationErrorRL = rtb_AND_am;

  /* Outport: '<Root>/Errors_rotationErrorRR' */
  rtY.Errors_rotationErrorRR = rtb_AND_c;

  /* Outport: '<Root>/Errors_YawrateError' */
  rtY.Errors_YawrateError = rtb_AND_g;

  /* Outport: '<Root>/Errors_StrAngError' */
  rtY.Errors_StrAngError = rtb_AND_k3;

  /* Outport: '<Root>/TV_TV_torqueFL' */
  rtY.TV_TV_torqueFL = rtb_Gain_k;

  /* Outport: '<Root>/TV_TV_torqueFR' */
  rtY.TV_TV_torqueFR = rtb_Gain_k;

  /* Outport: '<Root>/TV_TV_torqueRL' */
  rtY.TV_TV_torqueRL = rtb_Gain1_a;

  /* Outport: '<Root>/TV_TV_torqueRR' */
  rtY.TV_TV_torqueRR = rtb_Gain1_a;
}

/* Model initialize function */
void SubsystemModelReference_initialize(void)
{
  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
