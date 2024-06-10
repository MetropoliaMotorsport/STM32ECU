
// Keep that file updated with the devices you want to use in the car

#include "ecumain.h"
#include "errors.h"
#include "canecu.h"
#include "bms.h"
#include "output.h"
#include "power.h"
#include "debug.h"
#include "node_device.h"


//TODO Keep it updated with the devices you want to use in the car
bool processNodeDevice(const uint8_t CANRxData[8], const uint32_t DataLength, CANData *datahandle){
	volatile uint16_t messaga = 0;

		for (int i = 0; i < datahandle->dlcsize; i++) {
			messaga |= CANRxData[i] << (i * 8);
		}

		datahandle->data = messaga;

		return 1;
}




volatile CANData BPPS = { &DeviceState.BPPS, BPPS_ID, 2, processNodeDevice, NULL, 0, processNodeDevice };
volatile CANData APPS1 = { &DeviceState.APPS1, APPS1_ID, 2, processNodeDevice, NULL, 0,processNodeDevice };
volatile CANData APPS2 = { &DeviceState.APPS2, APPS2_ID, 2, processNodeDevice, NULL, 0, processNodeDevice };
volatile CANData SteeringAngle = { &DeviceState.SteeringAngle, SteeringAngle_ID, 8, processNodeDevice, NULL, 0 };
CANData WaterLevel = { &DeviceState.WaterLevel, WaterLevel_ID, 8, processNodeDevice, NULL, 0 };
CANData HeavesRear = { &DeviceState.HeavesRear, HeavesRear_ID, 8, processNodeDevice, NULL, 0 };
CANData HeavesFront = { &DeviceState.HeavesFront, HeavesFront_ID, 8, processNodeDevice, NULL, 0 };
CANData Rolls1 = { &DeviceState.Rolls1, Rolls1_ID, 8, processNodeDevice, NULL, 0 };
CANData Rolls2 = { &DeviceState.Rolls2, Rolls2_ID, 8, processNodeDevice, NULL, 0 };
CANData BrakeFront = { &DeviceState.BrakeFront, BrakeFront_ID, 8, processNodeDevice, NULL, 0 };
CANData BrakeRear = { &DeviceState.BrakeRear, BrakeRear_ID, 8, processNodeDevice, NULL, 0 };



int getNodeWait(){
	return 0;
}

uint32_t getAnalogueNodesOnline(){
    return 0;
}

void initNodeDevices( void ){

    RegisterCan2Message( &BPPS );
    RegisterCan2Message( &APPS1 );
    RegisterCan2Message( &APPS2 );
    RegisterCan2Message( &SteeringAngle );
    RegisterCan2Message( &WaterLevel );
    RegisterCan2Message( &HeavesFront );
    RegisterCan2Message( &HeavesRear );
    RegisterCan2Message( &Rolls1 );
    RegisterCan2Message( &Rolls2 );
    RegisterCan2Message( &BrakeFront );
    RegisterCan2Message( &BrakeRear );


    //TODO add more devices
    return 0;

}

