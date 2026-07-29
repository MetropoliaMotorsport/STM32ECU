/*
 * configuration.c
 *
 *  Created on: 13 Apr 2019
 *      Author: Visa
 */

#include "configuration.h"
#include "FreeRTOS.h"
#include "canecu.h"
#include "ecumain.h"
#include "eeprom.h"
#include "input.h"
#include "inverter.h"
#include "node_device.h"
#include "operationalreadyness.h"
#include "power.h"
#include "semphr.h"
#include "taskpriorities.h"
#include "timerecu.h"
#include "torquecontrol.h"
#include <stdbool.h>
#include <stdint.h>

static StaticQueue_t ConfigInputStaticQueue;
uint8_t ConfigInputQueueStorageArea[ConfigInputQUEUE_LENGTH * ConfigInputITEMSIZE];

StaticTask_t xConfigTaskBuffer;
RAM_D1 StackType_t xConfigStack[ConfigSTACK_SIZE];

static uint8_t ECUConfigdata[8] = {0};
static bool ECUConfignewdata = false;
static uint32_t ECUConfigDataTime = 0;

CANData ECUConfig = {NULL, 0x21, 8, GetConfigCmd, NULL, 0};
TaskHandle_t ConfigTaskHandle = {0};
QueueHandle_t ConfigInputQueue = {0};

char ConfStr[40] = "";

static void ProcessCANConfigMessage(uint8_t msg[8]);
static bool configReset = false;
static bool redraw = false;
static bool debugconfig = false;

bool checkConfigReset(void)
{
  if (configReset)
  {
    configReset = false;
    return true;
  }

  return false;
}

void ConfigReset(void)
{
  configReset = false;
}

bool GetConfigCmd(const uint8_t CANRxData[8], const uint32_t DataLength, CANData* datahandle)
{
  if ((CANRxData[0] >= 8 && CANRxData[0] <= 11) || (CANRxData[0] == 30)) // eeprom command.
  {
    return GetEEPROMCmd(CANRxData, DataLength, datahandle);
  }
  else if (ECUConfignewdata)
  {
    // NOTE: temporary error message
    CAN_SendErrorStatus(49, 99, 404);
  }
  else
  {
    ECUConfigDataTime = gettimer();
    memcpy(ECUConfigdata, CANRxData, 8);
    ECUConfignewdata = true; // moved to end to ensure data is not read before updated.
  }
  return true;
}

void setCurConfig(void)
{
  //	EEPROMdata

  eepromdata* data = getEEPROMBlock(0);
  if (DeviceState.EEPROM == ENABLED)
  {
    CarState.PedalProfile = data->PedalProfile;
    SetupTorque(CarState.PedalProfile);
    CarState.LimpDisable = !data->LimpMode;
    CarState.Torque_Req_Max = data->MaxTorque;
    CarState.MaxTorque = data->MaxTorque;
    CarState.FanPowered = data->Fans;
    CarState.LimpNM = data->LimpNM;
    CarState.DrivingMode = data->DrivingMode;

    CarState.AllowTC = data->TractionControl;
    CarState.AllowTV = data->TorqueVectoring;
    CarState.AllowRegen = data->Regen;
    CarState.HV_on = data->alwaysHV;

    CarConfig.TorqueVectoringOn = data->TorqueVectoring;
    CarConfig.EnabledMotors = data->EnabledMotors;
    CarConfig.MotorCount = getMotorCount(data->EnabledMotors);
    CarConfig.RTDMBrakePressure = data->RTDMBrakePressure;

    CarConfig.MaxRegenPower = data->regenMax;
    CarConfig.MaxDrivePower = data->MaxDrivePower;
    CarConfig.RegenBrakingOn = data->Regen;
  }
  else
  {
    CarState.PedalProfile = 0;
    SetupTorque(CarState.PedalProfile);
    CarState.LimpDisable = 0;
    CarState.Torque_Req_Max = 5;
    CarState.FanPowered = true;
  }

  CarState.Torque_Req_CurrentMax = CarState.Torque_Req_Max;
}

char* GetPedalProfile(uint8_t profile, bool shortform)
{
  if (shortform)
  {
    switch (profile)
    {
    case 0:
      return "Lin";
    case 1:
      return "Low";
    case 2:
      return "Acc";
    }
  }
  else
  {
    switch (profile)
    {
    case 0: // Full EEPROM
      return "Linear";
    case 1: // Full EEPROM
      return "Low Range";
    case 2: // Full EEPROM
      return "Acceleration";
    }
  }
  return NULL;
}

uint16_t APPSL_min = 0;
uint16_t APPSL_max = 0;
uint16_t APPSR_min = 0;
uint16_t APPSR_max = 0;
uint16_t REG_min = 0;
uint16_t REG_max = 0;

// values to define sane input range on APPS 's

#define MAXTHRESH (0.95)
#define MINTHRESH (0.5)

static inline void setMin(uint16_t* min, uint16_t minval)
{
  if (minval < *min)
    *min = minval;
}

void setMax(uint16_t* max, uint16_t maxval)
{
  if (maxval > *max)
    *max = maxval;
}

bool doPedalCalibration(uint16_t input)
{
  static uint32_t count = 0;

  if (count % 20 == 0)
    redraw = true;

  count++;

  bool baddata = false;

  // TODO implement
  if (APPS1.data > (UINT16_MAX * MAXTHRESH) || APPS1.data < 0 // (UINT16_MAX*MINTHRESH)
  )
  {
    baddata = true;
  }

  if (APPS2.data > (UINT16_MAX * MAXTHRESH) || APPS2.data < 0 //  (UINT16_MAX*MINTHRESH)
  )
  {
    baddata = true;
  }

  if (BPPS.data > (UINT16_MAX * MAXTHRESH) || BPPS.data < 0 //(UINT16_MAX*MINTHRESH)
  )
  {
    baddata = true;
  }

  if (baddata)
  {
    return input != KEY_ENTER;
  }

  setMin(&APPSL_min, APPS1.data);
  setMin(&APPSR_min, APPS2.data);
  setMin(&REG_min, BPPS.data);

  setMax(&APPSL_max, APPS1.data);
  setMax(&APPSR_max, APPS2.data);
  setMax(&REG_max, BPPS.data);

  int32_t APPSL_close = abs(APPSL_max - APPSL_min) < 500 ? 1 : 0;
  int32_t APPSR_close = abs(APPSR_max - APPSR_min) < 500 ? 1 : 0;
  int32_t REG_close = abs(REG_max - REG_min) < 50 ? 1 : 0;

  if (APPSL_close || APPSR_close)
  {
    if (debugconfig && redraw)
    {
      // DebugPrintf("Press APPS & Regen");
      // DebugPrintf(" No brake pressure!");
      // DebugPrintf(str);
    }
  }
  else if (REG_close)
  {
    if (debugconfig && redraw)
    {
      // DebugPrintf("");
      // DebugPrintf("Press Regen");
      // DebugPrintf(str);
    }
  }
  else
  {
    int APPSL = 100.0 / (APPSL_max - APPSL_min) * (APPS1.data - APPSL_min);
    if (APPSL > 99)
      APPSL = 99;

    int APPSR = 100.0 / (APPSR_max - APPSR_min) * (APPS2.data - APPSR_min);
    if (APPSR > 99)
      APPSR = 99;
  }

  if (input == KEY_ENTER)
  {

    eepromdata* data = getEEPROMBlock(0);

    if (APPSL_max == 0 || APPSR_max == 0)
    {
    }
    else
    {
      data->TorqueReqLInput[0] = APPSL_min;
      data->TorqueReqLInput[1] = APPSL_max;
      data->TorqueReqLInput[2] = 0;
      data->TorqueReqLInput[3] = 0;

      data->TorqueReqRInput[0] = APPSR_min;
      data->TorqueReqRInput[1] = APPSR_max;
      data->TorqueReqRInput[2] = 0;
      data->TorqueReqRInput[3] = 0;

      // store new APPS calibration to memory.
    }

    if (REG_max == 0)
    {
    }
    else
    {
      data->BrakeTravelInput[0] = REG_min;
      data->BrakeTravelInput[1] = REG_max;
      data->BrakeTravelInput[2] = 0;
      data->BrakeTravelInput[3] = 0;
      // store new Regen calibration to memory.
    }

    return false;
  }
  else
    return true;
}

bool DoMenuTorque(uint16_t input)
{
#define TORQUEMENU_WHEELS (1)
#define TORQUEMENU_TCS (2)
#define TORQUEMENU_VECTORING (3)
#define TORQUEMENU_TRACTION (4)
#define TORQUEMENU_VELOCITY (5)
#define TORQUEMENU_FEEDBACK (6)
#define TORQUEMENU_FEEDFWD (7)
#define TORQUEMENU_VELSOURCE (8)
#define TORQUEMENU_LAST (TORQUEMENU_FEEDFWD)
#define TORQUEMENUSIZE (TORQUEMENU_LAST + 1)

  static menustruct_t menu = {
      .inedit = false, .top = 0, .selection = 0, .menusize = TORQUEMENUSIZE};

  static char MenuLines[TORQUEMENUSIZE + 1][21] = {0};

  if (menu.selection == 0 && input == KEY_ENTER) // CheckButtonPressed(Config_Input) )
  {
    redraw = true;
    // DebugPrintf("Leaving torque menu");
    menu.inedit = false;
    return false;
  }

  strcpy(MenuLines[0], "Vectoring Menu:");
  sprintf(MenuLines[1], "%cBack...", (menu.selection == 0) ? '>' : ' ');
  if (debugconfig && redraw)
    // DebugPrintf(MenuLines[0]);

    for (int i = 0; i < 3; i++)
    {

      // DebugPrintf(MenuLines[i + menu.top + 1]);
    }
  redraw = false;

  return true; // done with menu
}

bool DoMenu(uint16_t input)
{
  static bool inmenu = false;
  static bool incalib = false;
  static bool dofullsave = false;
  static int8_t submenu = 0;

  static menustruct_t menu = {.inedit = false, .top = 0, .selection = 0, .menusize = MAINMENUSIZE};

  static char MenuLines[MAINMENUSIZE + 1][21] = {0};

  if (inmenu)
  {
    if (submenu == 0 && menu.selection == 0 &&
        input == KEY_ENTER) // CheckButtonPressed(Config_Input) )
    {
      inmenu = false;
      menu.inedit = false;
      // DebugPrintf("\nSaving settings\n");

      if (dofullsave)
      {
        writeFullConfigEEPROM();
      }
      else
      {
        writeEEPROMCurConf(); // enqueue write the data to eeprom.
      }

      return false;
    }

    if (submenu == 0) // only check for entering a menu if not in one.
    {
      if (menu.selection == MENU_TORQUE && input == KEY_ENTER)
      {
        redraw = true;
        submenu = MENU_TORQUE;
        input = 0;
      }
    }

    if (submenu != 0) // we're in a sub menu, process it instead of current menu.
    {
      switch (menu.selection) // run the sub menu.
      {
      case MENU_TORQUE:
        if (!DoMenuTorque(input))
        {
          redraw = true;
          submenu = 0; // check if sub menu is done.
        }
        break;
      default:
        submenu = 0;
      }
      input = 0; // in a sub menu, no input processing here.
      return true;
    }

    if (!incalib && menu.selection == MENU_CALIB &&
        input == KEY_ENTER) // CheckButtonPressed(Config_Input) )
    {
      if (DeviceState.CriticalSensors == OPERATIONAL)
      {
        redraw = true;
        incalib = true;
        input = 0;

        APPSL_min = UINT16_MAX;
        APPSL_max = 0;
        APPSR_min = UINT16_MAX;
        APPSR_max = 0;
        REG_min = UINT16_MAX;
        REG_max = 0;
      }
      else
      {
        // DebugPrintf("Err:  Not ready.");
        input = 0; // input has been seen, null it.
      }
    }

    if (incalib)
    {
      if (!doPedalCalibration(input))
      {
        redraw = true;
        incalib = false;
        dofullsave = true;
        SetupInterpolationTables(getEEPROMBlock(0));

        // set the current pedal calibration after calibration exited.
      }
      else
        return true;
    }

    strcpy(MenuLines[0], "Config Menu:");

    sprintf(MenuLines[1], "%cBack & Save", (menu.selection == 0) ? '>' : ' ');
    snprintf(MenuLines[1 + MENU_TORQUE], sizeof(MenuLines[0]), "%cTorqueVect...",
             (menu.selection == MENU_TORQUE) ? '>' : ' ');

    // TODO: condition never true?
    uint16_t currpm = getEEPROMBlock(0)->maxRpm;

    if (currpm != getEEPROMBlock(0)->maxRpm)
    {
      getEEPROMBlock(0)->maxRpm = currpm;
      // add rr
    }

    bool curfans = getEEPROMBlock(0)->Fans;
    if (curfans != getEEPROMBlock(0)->Fans)
    {
      getEEPROMBlock(0)->Fans = curfans;
    }

    uint8_t curfanmaxcur = ceil((100.0 / 255 * getEEPROMBlock(0)->FanMax)); // convert to %

    uint8_t curfanmax = curfanmaxcur;
    if (curfanmax != curfanmaxcur)
    { // value changed.
      getEEPROMBlock(0)->FanMax = floor(curfanmax * 2.55);
    }

    snprintf(MenuLines[1 + MENU_CALIB], sizeof(MenuLines[0]), "%cAPPS Calib",
             (menu.selection == MENU_CALIB) ? '>' : ' ');

    snprintf(MenuLines[1 + MENU_STEERING], sizeof(MenuLines[0]), "%cSteeringCalib %4lu",
             (menu.selection == MENU_STEERING) ? '>' : ' ', SteeringAngle.data);

    if (menu.selection == MENU_STEERING && input == KEY_ENTER)
    {
      if (SteeringAngle.data != 0xFFFF)
      {
        getEEPROMBlock(0)->steerCalib = 180 - SteeringAngle.data;
        // value should update on display. add a set message.
        // DebugPrintf("Steering angle calibrated to offset %d",
        // 180 - SteeringAngle.data);
      }
      else
      {
        // DebugPrintf("Steering angle no data to calibrate");
      }
    }

    uint8_t regenon = getEEPROMBlock(0)->Regen;

    if (regenon != getEEPROMBlock(0)->Regen)
    {
      getEEPROMBlock(0)->Regen = regenon;
    }

    uint8_t regenmax = getEEPROMBlock(0)->regenMax;
    if (regenmax != getEEPROMBlock(0)->regenMax)
    { // value changed.
      getEEPROMBlock(0)->regenMax = regenmax;
    }

    uint8_t regenmaxR = getEEPROMBlock(0)->regenMaxR;
    if (regenmaxR != getEEPROMBlock(0)->regenMaxR)
    { // value changed.
      getEEPROMBlock(0)->regenMaxR = regenmaxR;
    }

    // #if (MENU_LAST == MENU_HV)
    bool curhvState = getEEPROMBlock(0)->alwaysHV;
    if (curhvState != getEEPROMBlock(0)->alwaysHV)
    {
      getEEPROMBlock(0)->alwaysHV = curhvState;
      ShutdownCircuitSet(curhvState);
    }
    // #endif

    if (debugconfig && redraw)
      // DebugPrintf(MenuLines[0]);

      for (int i = 0; i < 3; i++)
      {

        // DebugPrintf(MenuLines[i + menu.top + 1]);
      }

    if (debugconfig && redraw)
      // DebugPrintf("------\n");

      redraw = false; // updated, unflag till something changes.
    return true;
  }

  if (!inmenu)
  {
    inmenu = true;
    submenu = 0;
    dofullsave = false;

    if (debugconfig)
    {
      redraw = true; // starting menu, draw it.
                     // DebugPrintf("------\n");
    }

    return true;
  }

  redraw = false;
  return false;
}

// Add message to uart message queue. Might be called from ISR so add a check.
bool ConfigInput(uint16_t input)
{
  uint32_t confmsg;

  confmsg = input;

  if (xPortIsInsideInterrupt())
    return xQueueSendFromISR(ConfigInputQueue, &confmsg, 0);
  else
    return xQueueSendToBack(ConfigInputQueue, &confmsg,
                            0); // send it to error state handler queue for display to user.
}

char* getConfStr(void)
{
  // TODO: add a mutex
  if (ConfStr[0] == 0)
    return NULL;
  else
    return ConfStr;
}

SemaphoreHandle_t xInConfig = NULL;
StaticSemaphore_t xInConfigBuffer;

uint8_t configstate = 0;

bool inConfig(void)
{
  return configstate; // uxSemaphoreGetCount( xInConfig );
}

void ConfigTask(void* argument)
{
  xEventGroupSync(xStartupSync, 0, 1, portMAX_DELAY);

  while (1)
  {
    if (ECUConfignewdata)
    {
      uint8_t msg[8];

      taskENTER_CRITICAL();
      memcpy(msg, ECUConfigdata, 8);
      ECUConfignewdata = false;
      taskEXIT_CRITICAL();

      ProcessCANConfigMessage(msg);
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void Test_Func(void)
{
  vTaskDelay(pdMS_TO_TICKS(1000));

  uint8_t config[8] = {CAN_MENU_SET_VALUE, MENU_FANMAX, 200, 0, 0, 0, 0, 0};

  taskENTER_CRITICAL();
  memcpy(ECUConfigdata, config, sizeof(config));
  ECUConfignewdata = true;
  taskEXIT_CRITICAL();

  vTaskDelay(pdMS_TO_TICKS(300));

  eepromdata* data = getEEPROMBlock(0);
  uint16_t fan_max = data->FanMax;

  config[0] = CAN_MENU_SAVE;
  config[1] = 0;
  config[2] = 0;
  config[3] = 0;

  taskENTER_CRITICAL();
  memcpy(ECUConfigdata, config, sizeof(config));
  ECUConfignewdata = true;
  taskEXIT_CRITICAL();

  vTaskDelay(pdMS_TO_TICKS(20));
}

static bool SetEEPROMBlockValue(uint8_t item, uint16_t value)
{

  eepromdata* data = getEEPROMBlock(0);

  if (data == NULL)
    return false;

  switch (item)
  {
  case MENU_NM:
    data->MaxTorque = value;
    break;

  case MENU_LIMPDIS:
    data->LimpMode = value ? true : false;
    break;

  case MENU_NMBAL:
    data->TorqueBal = value & 0xFF;
    break;

  case MENU_FANS:
    data->Fans = value ? true : false;
    break;

  case MENU_FANMAX:
    data->FanMax = value & 0xFF;
    break;

  case MENU_RPM:
    data->maxRpm = value;
    break;

  case MENU_REGEN:
    data->Regen = value ? true : false;
    break;

  case MENU_INV:
    data->InvEnabled = value ? true : false;
    break;

  case MENU_REGENMAX:
    data->regenMax = value & 0xFF;
    break;

  case MENU_REGENMAXR:
    data->regenMaxR = value & 0xFF;
    break;

  case MENU_TELEMETRY:
    data->Telemetry = value ? true : false;
    break;

  case MENU_HV:
    data->alwaysHV = value ? true : false;
    ShutdownCircuitSet(data->alwaysHV);
    break;
  case MENU_LIMPNM:
    data->LimpNM = value & 0xFF;
    break;

  case MENU_PEDAL_PROFILE:
    if (value <= 2)
      data->PedalProfile = value;
    else
      return false;
    break;

  case MENU_DRIVING_MODE:
    if (value <= ENDURANCE)
      data->DrivingMode = value;
    else
      return false;
    break;

  case MENU_ENABLED_MOTORS:
    data->EnabledMotors = value & 0x0F;
    break;

  case MENU_APPS_BRAKE_LIGHT:
    data->APPSBrakeLight = value & 0xFF;
    break;

  case MENU_APPS_BRAKE_HARD:
    data->APPSBrakeHard = value & 0xFF;
    break;

  case MENU_APPS_BRAKE_RELEASE:
    data->APPSBrakeRelease = value & 0xFF;
    break;

  case MENU_TORQUE_SLOPE:
    data->TorqueSlope = value & 0xFFFF;
    break;

  case MENU_RTDM_BRAKE_PRESSURE:
    data->RTDMBrakePressure = value & 0xFF;
    break;

  case MENU_TV_ENABLE:
    data->TorqueVectoring = value ? 1 : 0;
    break;

  case MENU_TC_ENABLE:
    data->TractionControl = value ? 1 : 0;
    break;

  case MENU_MAX_POWER:
    data->MaxOutputPower = value & 0xFF;
    break;

  case MENU_MAX_DRIVE_POWER:
    data->MaxDrivePower = value & 0xFF;
    break;

  default:
    return false;
  }

  setCurConfig();
  return true;
}

static void ProcessCANConfigMessage(uint8_t msg[8])
{
  // uint8_t* vals = NULL;
  // memcpy(vals, msg, 8);
  uint8_t cmd = msg[0];
  uint8_t item = msg[1];
  uint16_t value = msg[2] | (msg[3] << 8);
  // TODO: process unused 4 last bytes

  switch (cmd)
  {
  case CAN_MENU_SET_VALUE:
    SetEEPROMBlockValue(item, value);
    break;

  case CAN_MENU_SAVE:
    writeEEPROMCurConf();
    break;

  case CAN_MENU_FULLSAVE:
    writeFullConfigEEPROM();
    break;

  case CAN_MENU_APPLY:
    setCurConfig();
    break;

  default:
    break;
  }
}

bool initConfig(void)
{
  RegisterCan1Message(&ECUConfig);

  ConfigInputQueue = xQueueCreateStatic(ConfigInputQUEUE_LENGTH, ConfigInputITEMSIZE,
                                        ConfigInputQueueStorageArea, &ConfigInputStaticQueue);

  vQueueAddToRegistry(ConfigInputQueue, "Config Input");

  xInConfig = xSemaphoreCreateBinaryStatic(&xInConfigBuffer);

  ConfigTaskHandle = xTaskCreateStatic(ConfigTask, ConfigTASKNAME, ConfigSTACK_SIZE, (void*)1,
                                       ConfigTASKPRIORITY, xConfigStack, &xConfigTaskBuffer);

  vTaskDelay(pdMS_TO_TICKS(1000));
  setCurConfig();

  return true;
}
