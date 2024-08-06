#include "canecu.h"
#include "ecumain.h"
#include "eeprom.h"
#include "errors.h"
#include "ivt.h"
#include "node_device.h"
#include "powernode.h"
#include "timerecu.h"
#include "inverter.h"


bool process_config(const uint8_t CANRxData[8], const uint32_t DataLength, CANData *datahandle){

uint8_t select = CANRxData[0];

switch(select){
    case 0:
        // Set driving mode
        CarState.DrivingMode = CANRxData[1];
        break;
    case 1:
        // Set maximun torque
        CarState.MaxTorque = CANRxData[1] / 10;
        break;
    case 2:
        // Set maximun speed
        CarState.MaxSpeed = CANRxData[1] + (CANRxData[2] << 8);
        break;
    case 3:
        // Set Torque Vectoring on/off
        CarState.AllowTV = CANRxData[1];
        break;
    case 4:
        // Set Regen on/off
        CarState.AllowRegen = CANRxData[1];
        break; 
    case 5: 
        // Set Traction Control on/off
        CarState.AllowTC = CANRxData[1];
        break;
    case 6:
        // Set Power Balance
        CarState.PowerBalance = CANRxData[1];
        break;
    default:
        break;

	}
	return 1;
}

CANData config = {NULL, CONFIG_ID, 5, process_config};

void initConfig(){
    
    RegisterCan2Message(&config);
}
