/*
 * Sponsored License - for use in support of a program or activity
 * sponsored by MathWorks.  Not for government, commercial or other
 * non-sponsored organizational use.
 *
 * File: TorqueVectoring.c
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

#include "TorqueVectoring.h"
#include <math.h>
#include "rtwtypes.h"
#include <stddef.h>
#define NumBitsPerChar                 8U

/* Block signals and states (default storage) */
TorqueVectoring_DW TorqueVectoring_DW_l;

/* External inputs (root inport signals with default storage) */
TorqueVectoring_ExtU TorqueVectoring_U;

/* External outputs (root outports fed by signals with default storage) */
TorqueVectoring_ExtY TorqueVectoring_Y;

/* Real-time model */
static TorqueVectoring_RT_MODEL TorqueVectoring_M_;
TorqueVectoring_RT_MODEL *const TorqueVectoring_M = &TorqueVectoring_M_;
static real_T look1_binlx(real_T TorqueVectoring_u0, const real_T
  TorqueVectoring_bp0[], const real_T TorqueVectoring_table[], uint32_T
  TorqueVectoring_maxIndex);
static real_T rtGetNaN(void);
static real32_T rtGetNaNF(void);

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
  IEEESingle nanF = { { 0.0F } };

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

static real_T look1_binlx(real_T TorqueVectoring_u0, const real_T
  TorqueVectoring_bp0[], const real_T TorqueVectoring_table[], uint32_T
  TorqueVectoring_maxIndex)
{
  real_T TorqueVectoring_frac;
  real_T TorqueVectoring_yL_0d0;
  uint32_T TorqueVectoring_iLeft;

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
  if (TorqueVectoring_u0 <= TorqueVectoring_bp0[0U]) {
    TorqueVectoring_iLeft = 0U;
    TorqueVectoring_frac = (TorqueVectoring_u0 - TorqueVectoring_bp0[0U]) /
      (TorqueVectoring_bp0[1U] - TorqueVectoring_bp0[0U]);
  } else if (TorqueVectoring_u0 < TorqueVectoring_bp0[TorqueVectoring_maxIndex])
  {
    uint32_T TorqueVectoring_bpIdx;
    uint32_T TorqueVectoring_iRght;

    /* Binary Search */
    TorqueVectoring_bpIdx = TorqueVectoring_maxIndex >> 1U;
    TorqueVectoring_iLeft = 0U;
    TorqueVectoring_iRght = TorqueVectoring_maxIndex;
    while (TorqueVectoring_iRght - TorqueVectoring_iLeft > 1U) {
      if (TorqueVectoring_u0 < TorqueVectoring_bp0[TorqueVectoring_bpIdx]) {
        TorqueVectoring_iRght = TorqueVectoring_bpIdx;
      } else {
        TorqueVectoring_iLeft = TorqueVectoring_bpIdx;
      }

      TorqueVectoring_bpIdx = (TorqueVectoring_iRght + TorqueVectoring_iLeft) >>
        1U;
    }

    TorqueVectoring_frac = (TorqueVectoring_u0 -
      TorqueVectoring_bp0[TorqueVectoring_iLeft]) /
      (TorqueVectoring_bp0[TorqueVectoring_iLeft + 1U] -
       TorqueVectoring_bp0[TorqueVectoring_iLeft]);
  } else {
    TorqueVectoring_iLeft = TorqueVectoring_maxIndex - 1U;
    TorqueVectoring_frac = (TorqueVectoring_u0 -
      TorqueVectoring_bp0[TorqueVectoring_maxIndex - 1U]) /
      (TorqueVectoring_bp0[TorqueVectoring_maxIndex] -
       TorqueVectoring_bp0[TorqueVectoring_maxIndex - 1U]);
  }

  /* Column-major Interpolation 1-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'off'
     Overflow mode: 'wrapping'
   */
  TorqueVectoring_yL_0d0 = TorqueVectoring_table[TorqueVectoring_iLeft];
  return (TorqueVectoring_table[TorqueVectoring_iLeft + 1U] -
          TorqueVectoring_yL_0d0) * TorqueVectoring_frac +
    TorqueVectoring_yL_0d0;
}

/* Model step function */
void TorqueVectoring_step(void)
{
  real_T rtb_Desiredyawratereferencedegs;
  real_T rtb_IProdOut;
  real_T rtb_Igains;
  real_T rtb_Yawmomentsaturation;
  int8_T TorqueVectoring_tmp;
  int8_T TorqueVectoring_tmp_0;
  boolean_T rtb_AND_d;

  /* DeadZone: '<S3>/Dead Zone' incorporates:
   *  Inport: '<Root>/StrAngleDeg'
   */
  if (TorqueVectoring_U.StrAngleDeg > TorqueVectoring_P.SteeringDeadzonelimit) {
    rtb_Desiredyawratereferencedegs = TorqueVectoring_U.StrAngleDeg -
      TorqueVectoring_P.SteeringDeadzonelimit;
  } else if (TorqueVectoring_U.StrAngleDeg >=
             -TorqueVectoring_P.SteeringDeadzonelimit) {
    rtb_Desiredyawratereferencedegs = 0.0;
  } else {
    rtb_Desiredyawratereferencedegs = TorqueVectoring_U.StrAngleDeg -
      (-TorqueVectoring_P.SteeringDeadzonelimit);
  }

  /* Abs: '<S3>/Abs' incorporates:
   *  DeadZone: '<S3>/Dead Zone'
   */
  rtb_IProdOut = fabs(rtb_Desiredyawratereferencedegs);

  /* Signum: '<S3>/Sign' */
  if (rtIsNaN(rtb_IProdOut)) {
    rtb_Desiredyawratereferencedegs = (rtNaN);
  } else {
    rtb_Desiredyawratereferencedegs = (rtb_IProdOut > 0.0);
  }

  /* Logic: '<S1>/AND' incorporates:
   *  Constant: '<S2>/Constant'
   *  Inport: '<Root>/TorqueVectoringEnabled'
   *  Inport: '<Root>/VehicleSpeed'
   *  RelationalOperator: '<S2>/Compare'
   *  Signum: '<S3>/Sign'
   */
  rtb_AND_d = ((rtb_Desiredyawratereferencedegs != 0.0) &&
               (TorqueVectoring_U.VehicleSpeed >=
                TorqueVectoring_P.CompareToConstant_const) &&
               (TorqueVectoring_U.TorqueVectoringEnabled != 0.0) && (TorqueVectoring_U.TorquePedal <=
                   TorqueVectoring_P.CompareToConstant_torquepedal));

  /* Fcn: '<S6>/Fcn2' incorporates:
   *  Inport: '<Root>/VehicleSpeed'
   */
  rtb_IProdOut = ((real_T)(TorqueVectoring_U.VehicleSpeed == 0.0) +
                  TorqueVectoring_U.VehicleSpeed) + 2.2204e-16;

  /* Product: '<S6>/Divide' incorporates:
   *  Constant: '<S6>/Constant'
   *  Gain: '<S6>/Gain'
   *  Gain: '<S6>/Gain1'
   *  Inport: '<Root>/StrAngleDeg'
   *  Math: '<S6>/Square'
   *  Product: '<S6>/Product'
   *  Sum: '<S6>/Plus'
   */
  rtb_Desiredyawratereferencedegs = 1.0 / TorqueVectoring_P.Sgr *
    TorqueVectoring_U.StrAngleDeg * rtb_IProdOut / (rtb_IProdOut * rtb_IProdOut *
    TorqueVectoring_P.Ku + TorqueVectoring_P.l);

  /* Gain: '<S6>/Gain2' incorporates:
   *  Constant: '<S6>/Constant1'
   *  Constant: '<S6>/Constant2'
   *  Product: '<S6>/Divide1'
   *  Product: '<S6>/Product1'
   */
  rtb_IProdOut = 1.0 / rtb_IProdOut * (TorqueVectoring_P.myy *
    TorqueVectoring_P.g) * TorqueVectoring_P.Gain2_Gain;

  /* MATLAB Function: '<S6>/MATLAB Function' */
  if (!(fabs(rtb_Desiredyawratereferencedegs) < rtb_IProdOut)) {
    if (rtIsNaN(rtb_Desiredyawratereferencedegs)) {
      rtb_Desiredyawratereferencedegs = (rtNaN);
    } else if (rtb_Desiredyawratereferencedegs < 0.0) {
      rtb_Desiredyawratereferencedegs = -1.0;
    } else {
      rtb_Desiredyawratereferencedegs = (rtb_Desiredyawratereferencedegs > 0.0);
    }

    rtb_Desiredyawratereferencedegs *= rtb_IProdOut;
  }

  /* Sum: '<S10>/Sum1' incorporates:
   *  Gain: '<S10>/Gain'
   *  Inport: '<Root>/VehicleYawRate'
   *  MATLAB Function: '<S6>/MATLAB Function'
   */
  rtb_IProdOut = rtb_Desiredyawratereferencedegs - TorqueVectoring_P.Gain_Gain *
    TorqueVectoring_U.VehicleYawRate;

  /* DeadZone: '<S10>/Dead Zone' */
  if (rtb_IProdOut > TorqueVectoring_P.DeadzoneYawrateError) {
    rtb_IProdOut -= TorqueVectoring_P.DeadzoneYawrateError;
  } else if (rtb_IProdOut >= -TorqueVectoring_P.DeadzoneYawrateError) {
    rtb_IProdOut = 0.0;
  } else {
    rtb_IProdOut -= -TorqueVectoring_P.DeadzoneYawrateError;
  }

  /* End of DeadZone: '<S10>/Dead Zone' */

  /* Sum: '<S54>/Sum' incorporates:
   *  DiscreteIntegrator: '<S45>/Integrator'
   *  Gain: '<S10>/Gain1'
   *  Inport: '<Root>/VehicleSpeed'
   *  Lookup_n-D: '<S10>/P gains'
   *  Product: '<S50>/PProd Out'
   */
  rtb_Desiredyawratereferencedegs = 1.0 / TorqueVectoring_P.TuneGains *
    look1_binlx(TorqueVectoring_U.VehicleSpeed,
                TorqueVectoring_P.Pgains_bp01Data,
                TorqueVectoring_P.Pgains_tableData, 12U) * rtb_IProdOut +
    TorqueVectoring_DW_l.Integrator_DSTATE;

  /* Saturate: '<S52>/Saturation' */
  if (rtb_Desiredyawratereferencedegs > TorqueVectoring_P.UpperYawMoment) {
    rtb_Yawmomentsaturation = TorqueVectoring_P.UpperYawMoment;
  } else if (rtb_Desiredyawratereferencedegs < TorqueVectoring_P.LowerYawMoment)
  {
    rtb_Yawmomentsaturation = TorqueVectoring_P.LowerYawMoment;
  } else {
    rtb_Yawmomentsaturation = rtb_Desiredyawratereferencedegs;
  }

  /* End of Saturate: '<S52>/Saturation' */

  /* Product: '<S9>/Product' incorporates:
   *  Constant: '<S9>/Constant'
   *  Inport: '<Root>/StrAngleDeg'
   */
  rtb_Igains = TorqueVectoring_U.StrAngleDeg * TorqueVectoring_P.feedforwardgain;

  /* Saturate: '<S8>/Yaw moment saturation' */
  if (rtb_Yawmomentsaturation > TorqueVectoring_P.UpperYawMoment) {
    rtb_Yawmomentsaturation = TorqueVectoring_P.UpperYawMoment;
  } else if (rtb_Yawmomentsaturation < TorqueVectoring_P.LowerYawMoment) {
    rtb_Yawmomentsaturation = TorqueVectoring_P.LowerYawMoment;
  }

  /* Saturate: '<S9>/Yaw moment saturation' */
  if (rtb_Igains > TorqueVectoring_P.UpperYawMoment) {
    rtb_Igains = TorqueVectoring_P.UpperYawMoment;
  } else if (rtb_Igains < TorqueVectoring_P.LowerYawMoment) {
    rtb_Igains = TorqueVectoring_P.LowerYawMoment;
  }

  /* Sum: '<S5>/Sum' incorporates:
   *  Inport: '<Root>/FeedForwardEnabled'
   *  Inport: '<Root>/FeedbackEnabled'
   *  Logic: '<S8>/AND'
   *  Logic: '<S9>/AND'
   *  Product: '<S8>/Product2'
   *  Product: '<S9>/Product2'
   *  Saturate: '<S8>/Yaw moment saturation'
   *  Saturate: '<S9>/Yaw moment saturation'
   */
  rtb_Yawmomentsaturation = (real_T)(TorqueVectoring_U.FeedbackEnabled &&
    rtb_AND_d) * rtb_Yawmomentsaturation + (real_T)
    (TorqueVectoring_U.FeedForwardEnabled && rtb_AND_d) * rtb_Igains;

  /* Saturate: '<S5>/Yaw moment saturation' */
  if (rtb_Yawmomentsaturation > TorqueVectoring_P.UpperYawMoment) {
    rtb_Yawmomentsaturation = TorqueVectoring_P.UpperYawMoment;
  } else if (rtb_Yawmomentsaturation < TorqueVectoring_P.LowerYawMoment) {
    rtb_Yawmomentsaturation = TorqueVectoring_P.LowerYawMoment;
  }

  /* End of Saturate: '<S5>/Yaw moment saturation' */

  /* Gain: '<S4>/Gain' incorporates:
   *  Constant: '<S7>/Constant'
   *  Gain: '<S7>/Gain1'
   *  Gain: '<S7>/Gain3'
   *  Product: '<S7>/Divide1'
   */
  rtb_Igains = TorqueVectoring_P.Gain3_Gain * rtb_Yawmomentsaturation *
    TorqueVectoring_P.Rd / (2.0 * TorqueVectoring_P.wr * TorqueVectoring_P.Gr) *
    TorqueVectoring_P.Gain_Gain_d;

  /* Outport: '<Root>/TVFL' */
  TorqueVectoring_Y.TVFL = rtb_Igains;

  /* Outport: '<Root>/TVFR' */
  TorqueVectoring_Y.TVFR = rtb_Igains;

  /* Gain: '<S4>/Gain1' incorporates:
   *  Constant: '<S7>/Constant1'
   *  Gain: '<S7>/Gain'
   *  Gain: '<S7>/Gain2'
   *  Product: '<S7>/Divide2'
   */
  rtb_Igains = TorqueVectoring_P.Gain2_Gain_o * rtb_Yawmomentsaturation *
    TorqueVectoring_P.Rd / (2.0 * TorqueVectoring_P.wf * TorqueVectoring_P.Gr) *
    TorqueVectoring_P.Gain1_Gain;

  /* Outport: '<Root>/TVRL' */
  TorqueVectoring_Y.TVRL = rtb_Igains;

  /* Outport: '<Root>/TVRR' */
  TorqueVectoring_Y.TVRR = rtb_Igains;

  /* DeadZone: '<S38>/DeadZone' */
  if (rtb_Desiredyawratereferencedegs > TorqueVectoring_P.UpperYawMoment) {
    rtb_Desiredyawratereferencedegs -= TorqueVectoring_P.UpperYawMoment;
  } else if (rtb_Desiredyawratereferencedegs >= TorqueVectoring_P.LowerYawMoment)
  {
    rtb_Desiredyawratereferencedegs = 0.0;
  } else {
    rtb_Desiredyawratereferencedegs -= TorqueVectoring_P.LowerYawMoment;
  }

  /* End of DeadZone: '<S38>/DeadZone' */

  /* Product: '<S42>/IProd Out' incorporates:
   *  Gain: '<S10>/Gain2'
   *  Inport: '<Root>/VehicleSpeed'
   *  Lookup_n-D: '<S10>/I gains'
   */
  rtb_IProdOut *= 1.0 / TorqueVectoring_P.TuneGains * look1_binlx
    (TorqueVectoring_U.VehicleSpeed, TorqueVectoring_P.Igains_bp01Data,
     TorqueVectoring_P.Igains_tableData, 12U);

  /* Switch: '<S36>/Switch1' incorporates:
   *  Constant: '<S36>/Clamping_zero'
   *  Constant: '<S36>/Constant'
   *  Constant: '<S36>/Constant2'
   *  RelationalOperator: '<S36>/fix for DT propagation issue'
   */
  if (rtb_Desiredyawratereferencedegs > TorqueVectoring_P.Clamping_zero_Value) {
    TorqueVectoring_tmp = TorqueVectoring_P.Constant_Value;
  } else {
    TorqueVectoring_tmp = TorqueVectoring_P.Constant2_Value;
  }

  /* Switch: '<S36>/Switch2' incorporates:
   *  Constant: '<S36>/Clamping_zero'
   *  Constant: '<S36>/Constant3'
   *  Constant: '<S36>/Constant4'
   *  RelationalOperator: '<S36>/fix for DT propagation issue1'
   */
  if (rtb_IProdOut > TorqueVectoring_P.Clamping_zero_Value) {
    TorqueVectoring_tmp_0 = TorqueVectoring_P.Constant3_Value;
  } else {
    TorqueVectoring_tmp_0 = TorqueVectoring_P.Constant4_Value;
  }

  /* Switch: '<S36>/Switch' incorporates:
   *  Constant: '<S36>/Clamping_zero'
   *  Constant: '<S36>/Constant1'
   *  Logic: '<S36>/AND3'
   *  RelationalOperator: '<S36>/Equal1'
   *  RelationalOperator: '<S36>/Relational Operator'
   *  Switch: '<S36>/Switch1'
   *  Switch: '<S36>/Switch2'
   */
  if ((TorqueVectoring_P.Clamping_zero_Value != rtb_Desiredyawratereferencedegs)
      && (TorqueVectoring_tmp == TorqueVectoring_tmp_0)) {
    rtb_IProdOut = TorqueVectoring_P.Constant1_Value;
  }

  /* Update for DiscreteIntegrator: '<S45>/Integrator' incorporates:
   *  Switch: '<S36>/Switch'
   */
  TorqueVectoring_DW_l.Integrator_DSTATE += TorqueVectoring_P.Integrator_gainval
    * rtb_IProdOut;
}

/* Model initialize function */
void TorqueVectoring_initialize(void)
{
  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));

  /* InitializeConditions for DiscreteIntegrator: '<S45>/Integrator' */
  TorqueVectoring_DW_l.Integrator_DSTATE =
    TorqueVectoring_P.DiscretePIDController_InitialConditionF;
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
