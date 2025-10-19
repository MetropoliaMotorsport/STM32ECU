# TODO: CANBUS

- Group the error and state messages
- Name the components better according to what they do
- Define
- Group errors by device (component)
- Base id + device id to create the message
- Inconsistent naming pattern -> name everything with a singular structure

# Components

## ECU

### Todo

- Separate them into controlword, statusword, etc
- Group all the errors into a single 64 bit message
  - Alternatively: group them by error type

### Signals: 30 in total

- PN1_Command -> PowerNode1
  - Select_PWM_GPIO
  - Select_output
  - State
- PN2_Command -> PowerNode2
  - Select_PWM_GPIO
  - Select_output
  - State
- Entering_Idle_State
- TS_OFF
- TS_BAD
- Critical_Error
- Ready_to_enable_TS
- TS_Activation_Req_Ready
- TS_Activation_Req_NReady
- Entering_Startup_State
- BMS_Fail
- IVT_Fail
- Readyness_Power_Fail
- Entering_Readyness_Check_State
- Errorplace_too_many_loops
- Entering_PreOperational_State
- Power_request
- Power_off_to_inverters_req
- Start_Pressed
- RDTM_Pressed
- Entering_RDTM_State
- RTDM_TA
- Dissabling_Regen (intentional mispronunciation)
- Entering_TS_Activate_State
- TS_Lov
- TS_On
- RDTM_Activation_with_no_braking
- Waiting_Precharge
- Idle_State_Req
- Entering_Error_State

#### Errors

- Critical_Error
-

## PowerNode1

- Over_Current_PN1 -> Error
- Under_Current_PN1 -> Error
- Warning_Current_PN1 -> Error
- PN1_Device_Status

## PowerNode2

- Over_Current_PN2 -> Error
- Under_Current_PN2 -> Error
- Warning_Current_PN2 -> Error
- PN2_Device_Status

## AnalogNode1 -> Controls the front (all have a message signal)

- APPS1
- APPS2
- BPPS
- BrakeFont
- Rolls1
- HeavesFront
- Steering_Angle
- BTN1
- BTN2
- BTN3

## AnalogNode2 -> Controls the rear (all have a message signal)

- BrakeRear
- WaterLevel
- HeavesRear
- Rolls2
