/*
 * Sponsored License - for use in support of a program or activity
 * sponsored by MathWorks.  Not for government, commercial or other
 * non-sponsored organizational use.
 *
 * File: Regeneration.c
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

#include "Regeneration.h"
#include <math.h>
#include "rtwtypes.h"

/* Block signals and states (default storage) */
Regeneration_DW Regeneration_DW_l;

/* External inputs (root inport signals with default storage) */
Regeneration_ExtU Regeneration_U;

/* External outputs (root outports fed by signals with default storage) */
Regeneration_ExtY Regeneration_Y;

/* Real-time model */
static Regeneration_RT_MODEL Regeneration_M_;
Regeneration_RT_MODEL *const Regeneration_M = &Regeneration_M_;
static real_T look1_binlx(real_T Regeneration_u0, const real_T Regeneration_bp0[],
  const real_T Regeneration_table[], uint32_T Regeneration_maxIndex);
static real_T look1_binlx(real_T Regeneration_u0, const real_T Regeneration_bp0[],
  const real_T Regeneration_table[], uint32_T Regeneration_maxIndex)
{
  real_T Regeneration_frac;
  real_T Regeneration_yL_0d0;
  uint32_T Regeneration_iLeft;

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
  if (Regeneration_u0 <= Regeneration_bp0[0U]) {
    Regeneration_iLeft = 0U;
    Regeneration_frac = (Regeneration_u0 - Regeneration_bp0[0U]) /
      (Regeneration_bp0[1U] - Regeneration_bp0[0U]);
  } else if (Regeneration_u0 < Regeneration_bp0[Regeneration_maxIndex]) {
    uint32_T Regeneration_bpIdx;
    uint32_T Regeneration_iRght;

    /* Binary Search */
    Regeneration_bpIdx = Regeneration_maxIndex >> 1U;
    Regeneration_iLeft = 0U;
    Regeneration_iRght = Regeneration_maxIndex;
    while (Regeneration_iRght - Regeneration_iLeft > 1U) {
      if (Regeneration_u0 < Regeneration_bp0[Regeneration_bpIdx]) {
        Regeneration_iRght = Regeneration_bpIdx;
      } else {
        Regeneration_iLeft = Regeneration_bpIdx;
      }

      Regeneration_bpIdx = (Regeneration_iRght + Regeneration_iLeft) >> 1U;
    }

    Regeneration_frac = (Regeneration_u0 - Regeneration_bp0[Regeneration_iLeft])
      / (Regeneration_bp0[Regeneration_iLeft + 1U] -
         Regeneration_bp0[Regeneration_iLeft]);
  } else {
    Regeneration_iLeft = Regeneration_maxIndex - 1U;
    Regeneration_frac = (Regeneration_u0 -
                         Regeneration_bp0[Regeneration_maxIndex - 1U]) /
      (Regeneration_bp0[Regeneration_maxIndex] -
       Regeneration_bp0[Regeneration_maxIndex - 1U]);
  }

  /* Column-major Interpolation 1-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'off'
     Overflow mode: 'wrapping'
   */
  Regeneration_yL_0d0 = Regeneration_table[Regeneration_iLeft];
  return (Regeneration_table[Regeneration_iLeft + 1U] - Regeneration_yL_0d0) *
    Regeneration_frac + Regeneration_yL_0d0;
}

/* Model step function */
void Regeneration_step(void)
{
  real_T Regenerat_rtb_DataTypeConversion2_idx_0;
  real_T Regenerat_rtb_DataTypeConversion2_idx_1;
  real_T Regenerat_rtb_DataTypeConversion2_idx_2;
  real_T Regenerat_rtb_DataTypeConversion2_idx_3;
  real_T rtb_Merge1;
  real_T rtb_Merge2;
  real_T rtb_Product_h;
  real_T rtb_Switch_on;
  int32_T Regeneration_u0;

  /* Sum: '<S5>/Sum2' incorporates:
   *  Constant: '<S5>/Max permissible negative slip'
   *  Inport: '<Root>/Slip_FL'
   *  Inport: '<Root>/Slip_FR'
   *  Inport: '<Root>/Slip_RL'
   *  Inport: '<Root>/Slip_RR'
   */
  Regenerat_rtb_DataTypeConversion2_idx_0 =
    Regeneration_P.Maxpermissiblenegativeslip_Value + Regeneration_U.Slip_FL;
  Regenerat_rtb_DataTypeConversion2_idx_1 =
    Regeneration_P.Maxpermissiblenegativeslip_Value + Regeneration_U.Slip_FR;
  Regenerat_rtb_DataTypeConversion2_idx_2 =
    Regeneration_P.Maxpermissiblenegativeslip_Value + Regeneration_U.Slip_RR;
  Regenerat_rtb_DataTypeConversion2_idx_3 =
    Regeneration_P.Maxpermissiblenegativeslip_Value + Regeneration_U.Slip_RL;

  /* Abs: '<S5>/Abs' incorporates:
   *  Inport: '<Root>/max_regen_torque'
   */
  rtb_Product_h = fabs(Regeneration_U.max_regen_torque);

  /* Saturate: '<S5>/Saturation' incorporates:
   *  Constant: '<S18>/Constant'
   *  RelationalOperator: '<S18>/Relational Operator'
   *  RelationalOperator: '<S18>/Relational Operator1'
   *  Sum: '<S18>/Sum'
   */
  Regeneration_u0 = (Regenerat_rtb_DataTypeConversion2_idx_0 >
                     Regeneration_P.Constant_Value_f) -
    (Regenerat_rtb_DataTypeConversion2_idx_0 < Regeneration_P.Constant_Value_f);
  if (Regeneration_u0 > Regeneration_P.Saturation_UpperSat) {
    rtb_Merge1 = Regeneration_P.Saturation_UpperSat;
  } else if (Regeneration_u0 < Regeneration_P.Saturation_LowerSat) {
    rtb_Merge1 = Regeneration_P.Saturation_LowerSat;
  } else {
    rtb_Merge1 = Regeneration_u0;
  }

  /* Product: '<S5>/Product1' incorporates:
   *  Inport: '<Root>/regen_optimizer_on'
   *  Product: '<S5>/Product'
   *  Saturate: '<S5>/Saturation'
   */
  Regenerat_rtb_DataTypeConversion2_idx_0 = rtb_Merge1 *
    Regeneration_U.regen_optimizer_on * rtb_Product_h;

  /* Saturate: '<S5>/Saturation' incorporates:
   *  Constant: '<S18>/Constant'
   *  RelationalOperator: '<S18>/Relational Operator'
   *  RelationalOperator: '<S18>/Relational Operator1'
   *  Sum: '<S18>/Sum'
   */
  Regeneration_u0 = (Regenerat_rtb_DataTypeConversion2_idx_1 >
                     Regeneration_P.Constant_Value_f) -
    (Regenerat_rtb_DataTypeConversion2_idx_1 < Regeneration_P.Constant_Value_f);
  if (Regeneration_u0 > Regeneration_P.Saturation_UpperSat) {
    rtb_Merge1 = Regeneration_P.Saturation_UpperSat;
  } else if (Regeneration_u0 < Regeneration_P.Saturation_LowerSat) {
    rtb_Merge1 = Regeneration_P.Saturation_LowerSat;
  } else {
    rtb_Merge1 = Regeneration_u0;
  }

  /* Product: '<S5>/Product1' incorporates:
   *  Inport: '<Root>/regen_optimizer_on'
   *  Product: '<S5>/Product'
   *  Saturate: '<S5>/Saturation'
   */
  Regenerat_rtb_DataTypeConversion2_idx_1 = rtb_Merge1 *
    Regeneration_U.regen_optimizer_on * rtb_Product_h;

  /* Saturate: '<S5>/Saturation' incorporates:
   *  Constant: '<S18>/Constant'
   *  RelationalOperator: '<S18>/Relational Operator'
   *  RelationalOperator: '<S18>/Relational Operator1'
   *  Sum: '<S18>/Sum'
   */
  Regeneration_u0 = (Regenerat_rtb_DataTypeConversion2_idx_2 >
                     Regeneration_P.Constant_Value_f) -
    (Regenerat_rtb_DataTypeConversion2_idx_2 < Regeneration_P.Constant_Value_f);
  if (Regeneration_u0 > Regeneration_P.Saturation_UpperSat) {
    rtb_Merge1 = Regeneration_P.Saturation_UpperSat;
  } else if (Regeneration_u0 < Regeneration_P.Saturation_LowerSat) {
    rtb_Merge1 = Regeneration_P.Saturation_LowerSat;
  } else {
    rtb_Merge1 = Regeneration_u0;
  }

  /* Product: '<S5>/Product1' incorporates:
   *  Inport: '<Root>/regen_optimizer_on'
   *  Product: '<S5>/Product'
   *  Saturate: '<S5>/Saturation'
   */
  Regenerat_rtb_DataTypeConversion2_idx_2 = rtb_Merge1 *
    Regeneration_U.regen_optimizer_on * rtb_Product_h;

  /* Saturate: '<S5>/Saturation' incorporates:
   *  Constant: '<S18>/Constant'
   *  RelationalOperator: '<S18>/Relational Operator'
   *  RelationalOperator: '<S18>/Relational Operator1'
   *  Sum: '<S18>/Sum'
   */
  Regeneration_u0 = (Regenerat_rtb_DataTypeConversion2_idx_3 >
                     Regeneration_P.Constant_Value_f) -
    (Regenerat_rtb_DataTypeConversion2_idx_3 < Regeneration_P.Constant_Value_f);
  if (Regeneration_u0 > Regeneration_P.Saturation_UpperSat) {
    rtb_Merge1 = Regeneration_P.Saturation_UpperSat;
  } else if (Regeneration_u0 < Regeneration_P.Saturation_LowerSat) {
    rtb_Merge1 = Regeneration_P.Saturation_LowerSat;
  } else {
    rtb_Merge1 = Regeneration_u0;
  }

  /* Product: '<S5>/Product1' incorporates:
   *  Inport: '<Root>/regen_optimizer_on'
   *  Product: '<S5>/Product'
   *  Saturate: '<S5>/Saturation'
   */
  Regenerat_rtb_DataTypeConversion2_idx_3 = rtb_Merge1 *
    Regeneration_U.regen_optimizer_on * rtb_Product_h;

  /* SwitchCase: '<S2>/Switch Case' incorporates:
   *  Constant: '<S11>/Constant'
   *  DataTypeConversion: '<S2>/Cast To Double'
   *  Inport: '<Root>/select_operating_mode'
   *  Inport: '<Root>/speed'
   *  Product: '<S2>/Product'
   *  RelationalOperator: '<S11>/Compare'
   */
  switch ((int32_T)((real_T)(Regeneration_U.speed >
            Regeneration_P.Deactivateregenifspeedbelow9kmh_const) *
                    Regeneration_U.select_operating_mode)) {
   case 1:
    /* Outputs for IfAction SubSystem: '<S2>/Autocross mode' incorporates:
     *  ActionPort: '<S10>/Action Port'
     */
    /* Lookup_n-D: '<S10>/Map pedal to regen ' incorporates:
     *  Inport: '<Root>/brake_pedal_position'
     */
    rtb_Switch_on = look1_binlx(Regeneration_U.brake_pedal_position,
      Regeneration_P.Mappedaltoregen_bp01Data,
      Regeneration_P.Mappedaltoregen_tableData, 25U);

    /* Switch: '<S14>/Switch2' incorporates:
     *  Constant: '<S10>/Constant'
     *  Inport: '<Root>/max_regen_torque'
     *  RelationalOperator: '<S14>/LowerRelop1'
     *  RelationalOperator: '<S14>/UpperRelop'
     *  Switch: '<S14>/Switch'
     */
    if (rtb_Switch_on > Regeneration_P.Constant_Value) {
      rtb_Switch_on = Regeneration_P.Constant_Value;
    } else if (rtb_Switch_on < Regeneration_U.max_regen_torque) {
      /* Switch: '<S14>/Switch' incorporates:
       *  Inport: '<Root>/max_regen_torque'
       */
      rtb_Switch_on = Regeneration_U.max_regen_torque;
    }

    /* Product: '<S10>/Product' incorporates:
     *  Inport: '<Root>/Torque_pedal'
     *  Switch: '<S14>/Switch2'
     */
    rtb_Product_h = rtb_Switch_on * Regeneration_U.Torque_pedal;

    /* SignalConversion generated from: '<S10>/regenFR' */
    rtb_Merge1 = rtb_Product_h;

    /* SignalConversion generated from: '<S10>/regenRL' */
    rtb_Merge2 = rtb_Product_h;

    /* SignalConversion generated from: '<S10>/regenRR' */
    rtb_Switch_on = rtb_Product_h;

    /* End of Outputs for SubSystem: '<S2>/Autocross mode' */
    break;

   case 2:
    {
      boolean_T rtb_GreaterThan1;

      /* Outputs for IfAction SubSystem: '<S2>/Endurance mode' incorporates:
       *  ActionPort: '<S12>/Action Port'
       */
      /* RelationalOperator: '<S12>/GreaterThan1' incorporates:
       *  Inport: '<Root>/Torque_pedal'
       *  Inport: '<Root>/pedal_rege_thresh_endurance_max'
       */
      rtb_GreaterThan1 = (Regeneration_U.Torque_pedal >=
                          Regeneration_U.pedal_rege_thresh_endurance_max);

      /* Chart: '<S12>/Chart' incorporates:
       *  DataTypeConversion: '<S12>/Cast To Double'
       */
      rtb_Switch_on = Regeneration_DW_l.u_start;
      Regeneration_DW_l.u_start = rtb_GreaterThan1;
      if (!Regeneration_DW_l.doneDoubleBufferReInit) {
        Regeneration_DW_l.doneDoubleBufferReInit = true;
        rtb_Switch_on = rtb_GreaterThan1;
      }

      /* Product: '<S12>/Product' incorporates:
       *  Chart: '<S12>/Chart'
       *  Inport: '<Root>/max_regen_torque'
       *  MATLAB Function: '<S12>/MATLAB Function'
       */
      rtb_Product_h = (real_T)((rtb_Switch_on != Regeneration_DW_l.u_start) &&
        (Regeneration_DW_l.u_start == 0.0)) * Regeneration_U.max_regen_torque;

      /* SignalConversion generated from: '<S12>/regenFR' */
      rtb_Merge1 = rtb_Product_h;

      /* SignalConversion generated from: '<S12>/regenRL' */
      rtb_Merge2 = rtb_Product_h;

      /* SignalConversion generated from: '<S12>/regenRR' */
      rtb_Switch_on = rtb_Product_h;

      /* End of Outputs for SubSystem: '<S2>/Endurance mode' */
    }
    break;

   default:
    /* Outputs for IfAction SubSystem: '<S2>/No regeneration' incorporates:
     *  ActionPort: '<S13>/Action Port'
     */
    /* SignalConversion generated from: '<S13>/regenFL' incorporates:
     *  Constant: '<S13>/Constant'
     */
    rtb_Product_h = Regeneration_P.Constant_Value_j;

    /* SignalConversion generated from: '<S13>/regenFR' incorporates:
     *  Constant: '<S13>/Constant'
     */
    rtb_Merge1 = Regeneration_P.Constant_Value_j;

    /* SignalConversion generated from: '<S13>/regenRL' incorporates:
     *  Constant: '<S13>/Constant'
     */
    rtb_Merge2 = Regeneration_P.Constant_Value_j;

    /* SignalConversion generated from: '<S13>/regenRR' incorporates:
     *  Constant: '<S13>/Constant'
     */
    rtb_Switch_on = Regeneration_P.Constant_Value_j;

    /* End of Outputs for SubSystem: '<S2>/No regeneration' */
    break;
  }

  /* End of SwitchCase: '<S2>/Switch Case' */

  /* Sum: '<S1>/Sum' */
  rtb_Product_h -= Regenerat_rtb_DataTypeConversion2_idx_0;

  /* Switch: '<S6>/Switch2' incorporates:
   *  Constant: '<S1>/Constant'
   *  Inport: '<Root>/max_regen_torque'
   *  RelationalOperator: '<S6>/LowerRelop1'
   *  RelationalOperator: '<S6>/UpperRelop'
   *  Switch: '<S6>/Switch'
   */
  if (rtb_Product_h > Regeneration_P.Constant_Value_i) {
    /* Outport: '<Root>/regenFL' */
    Regeneration_Y.regenFL = Regeneration_P.Constant_Value_i;
  } else if (rtb_Product_h < Regeneration_U.max_regen_torque) {
    /* Switch: '<S6>/Switch' incorporates:
     *  Inport: '<Root>/max_regen_torque'
     *  Outport: '<Root>/regenFL'
     */
    Regeneration_Y.regenFL = Regeneration_U.max_regen_torque;
  } else {
    /* Outport: '<Root>/regenFL' incorporates:
     *  Switch: '<S6>/Switch'
     */
    Regeneration_Y.regenFL = rtb_Product_h;
  }

  /* End of Switch: '<S6>/Switch2' */

  /* Sum: '<S1>/Sum1' */
  rtb_Merge1 -= Regenerat_rtb_DataTypeConversion2_idx_1;

  /* Switch: '<S7>/Switch2' incorporates:
   *  Constant: '<S1>/Constant'
   *  Inport: '<Root>/max_regen_torque'
   *  RelationalOperator: '<S7>/LowerRelop1'
   *  RelationalOperator: '<S7>/UpperRelop'
   *  Switch: '<S7>/Switch'
   */
  if (rtb_Merge1 > Regeneration_P.Constant_Value_i) {
    /* Outport: '<Root>/regenFR' */
    Regeneration_Y.regenFR = Regeneration_P.Constant_Value_i;
  } else if (rtb_Merge1 < Regeneration_U.max_regen_torque) {
    /* Switch: '<S7>/Switch' incorporates:
     *  Inport: '<Root>/max_regen_torque'
     *  Outport: '<Root>/regenFR'
     */
    Regeneration_Y.regenFR = Regeneration_U.max_regen_torque;
  } else {
    /* Outport: '<Root>/regenFR' incorporates:
     *  Switch: '<S7>/Switch'
     */
    Regeneration_Y.regenFR = rtb_Merge1;
  }

  /* End of Switch: '<S7>/Switch2' */

  /* Sum: '<S1>/Sum2' */
  rtb_Merge2 -= Regenerat_rtb_DataTypeConversion2_idx_2;

  /* Switch: '<S8>/Switch2' incorporates:
   *  Constant: '<S1>/Constant'
   *  Inport: '<Root>/max_regen_torque'
   *  RelationalOperator: '<S8>/LowerRelop1'
   *  RelationalOperator: '<S8>/UpperRelop'
   *  Switch: '<S8>/Switch'
   */
  if (rtb_Merge2 > Regeneration_P.Constant_Value_i) {
    /* Outport: '<Root>/regenRL' */
    Regeneration_Y.regenRL = Regeneration_P.Constant_Value_i;
  } else if (rtb_Merge2 < Regeneration_U.max_regen_torque) {
    /* Switch: '<S8>/Switch' incorporates:
     *  Inport: '<Root>/max_regen_torque'
     *  Outport: '<Root>/regenRL'
     */
    Regeneration_Y.regenRL = Regeneration_U.max_regen_torque;
  } else {
    /* Outport: '<Root>/regenRL' incorporates:
     *  Switch: '<S8>/Switch'
     */
    Regeneration_Y.regenRL = rtb_Merge2;
  }

  /* End of Switch: '<S8>/Switch2' */

  /* Sum: '<S1>/Sum3' */
  rtb_Switch_on -= Regenerat_rtb_DataTypeConversion2_idx_3;

  /* Switch: '<S9>/Switch2' incorporates:
   *  Constant: '<S1>/Constant'
   *  Inport: '<Root>/max_regen_torque'
   *  RelationalOperator: '<S9>/LowerRelop1'
   *  RelationalOperator: '<S9>/UpperRelop'
   *  Switch: '<S9>/Switch'
   */
  if (rtb_Switch_on > Regeneration_P.Constant_Value_i) {
    /* Outport: '<Root>/regenRR' */
    Regeneration_Y.regenRR = Regeneration_P.Constant_Value_i;
  } else if (rtb_Switch_on < Regeneration_U.max_regen_torque) {
    /* Switch: '<S9>/Switch' incorporates:
     *  Inport: '<Root>/max_regen_torque'
     *  Outport: '<Root>/regenRR'
     */
    Regeneration_Y.regenRR = Regeneration_U.max_regen_torque;
  } else {
    /* Outport: '<Root>/regenRR' incorporates:
     *  Switch: '<S9>/Switch'
     */
    Regeneration_Y.regenRR = rtb_Switch_on;
  }

  /* End of Switch: '<S9>/Switch2' */

  /* Gain: '<S4>/to kWh' incorporates:
   *  Inport: '<Root>/IVT_WhCalculated'
   */
  rtb_Switch_on = Regeneration_P.tokWh_Gain * Regeneration_U.IVT_WhCalculated;

  /* Outport: '<Root>/SOC' incorporates:
   *  Constant: '<S3>/max kWh'
   *  Gain: '<S3>/Gain'
   *  Product: '<S3>/Divide'
   */
  Regeneration_Y.SOC = rtb_Switch_on / Regeneration_P.maxkWh_Value *
    Regeneration_P.Gain_Gain;

  /* Sum: '<S4>/Sum1' incorporates:
   *  Constant: '<S4>/Critical voltage difference'
   *  Inport: '<Root>/U_cell_max_mV'
   *  Inport: '<Root>/U_cell_max_possible_mV'
   *  Sum: '<S4>/Check critical difference between max cell voltage and max voltage limit'
   */
  rtb_Merge1 = Regeneration_P.Criticalvoltagedifference_Value -
    (Regeneration_U.U_cell_max_possible_mV - Regeneration_U.U_cell_max_mV);

  /* MATLAB Function: '<S4>/check_regen' */
  if (!Regeneration_DW_l.isActive_not_empty) {
    if (rtb_Merge1 > 0.0) {
      rtb_Merge2 = 1.0;
      rtb_Merge1 = rtb_Merge1 * 0.0036 + 2.2204460492503131E-16;
    } else {
      rtb_Merge2 = 0.0;
      rtb_Merge1 = 2.2204460492503131E-16;
    }

    Regeneration_DW_l.isActive.regen_control = rtb_Merge2;
    Regeneration_DW_l.isActive_not_empty = true;
    Regeneration_DW_l.isActive.energy = rtb_Merge1;
  } else {
    rtb_Merge2 = Regeneration_DW_l.isActive.regen_control;
    rtb_Merge1 = Regeneration_DW_l.isActive.energy;
  }

  /* Switch: '<S4>/Switch' incorporates:
   *  Inport: '<Root>/static_P_min_lim'
   *  MATLAB Function: '<S4>/check_regen'
   */
  if (rtb_Merge2 > Regeneration_P.Switch_Threshold) {
    /* Bias: '<S4>/Safety offset' incorporates:
     *  Gain: '<S4>/Proportional to maximum regen power'
     *  Product: '<S4>/Divide by the energy when we can regen fully'
     */
    rtb_Switch_on = rtb_Switch_on / rtb_Merge1 *
      Regeneration_P.Proportionaltomaximumregenpower_Gain +
      Regeneration_P.Safetyoffset_Bias;

    /* Saturate: '<S4>/Saturate to max regen of -100 kW' */
    if (rtb_Switch_on > Regeneration_P.Saturatetomaxregenof100kW_UpperSat) {
      rtb_Switch_on = Regeneration_P.Saturatetomaxregenof100kW_UpperSat;
    } else if (rtb_Switch_on < Regeneration_P.Saturatetomaxregenof100kW_LowerSat)
    {
      rtb_Switch_on = Regeneration_P.Saturatetomaxregenof100kW_LowerSat;
    }

    /* End of Saturate: '<S4>/Saturate to max regen of -100 kW' */
  } else {
    rtb_Switch_on = Regeneration_U.static_P_min_lim;
  }

  /* End of Switch: '<S4>/Switch' */

  /* Outport: '<Root>/MotorRegenPowerLimNegkW' incorporates:
   *  Gain: '<S1>/Gain'
   */
  Regeneration_Y.MotorRegenPowerLimNegkW = Regeneration_P.Gain_Gain_m *
    rtb_Switch_on;
}

/* Model initialize function */
void Regeneration_initialize(void)
{
  /* (no initialization code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
