
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
bool processBPPS(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle){
	uint64_t messaga;
		uint16_t data = 0;
		for (int i = 0; i < datahandle->dlcsize; i++) {
			messaga |= CANRxData[i] << (i * 8);
		}
		for (int i = datahandle->bitpos; i < datahandle->length; i++) {
			data |= (messaga >> i) & 1;
		}
		datahandle->data = data;
}

bool processAPPS1(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle){
	return true;
}
bool processAPPS2(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle){
	return true;
}
bool processSteeringAngle(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle){
	return true;
}
bool processWaterLevel(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle){
    return true;
}
bool processHeavesFront(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle){
    return true;
}
bool processHeavesRear(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle){
    return true;
}
bool processRolls1(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle){
    return true;
}
bool processRolls2(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle){
    return true;
}
bool processBrakeFront(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle){
	return true;
}
bool processBrakeRear(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle){
    return true;
}


CANData BPPS = { &DeviceState.BPPS, BPPS_ID, 8, processBPPS, NULL, 0 };
CANData APPS1 = { &DeviceState.APPS1, APPS1_ID, 8, processAPPS1, NULL, 0 };
CANData APPS2 = { &DeviceState.APPS2, APPS2_ID, 8, processAPPS2, NULL, 0 };
CANData SteeringAngle = { &DeviceState.SteeringAngle, SteeringAngle_ID, 8, processSteeringAngle, NULL, 0 };
CANData WaterLevel = { &DeviceState.WaterLevel, WaterLevel_ID, 8, processWaterLevel, NULL, 0 };
CANData HeavesFront = { &DeviceState.HeavesFront, HeavesFront_ID, 8, processHeavesFront, NULL, 0 };
CANData Rolls1 = { &DeviceState.Rolls1, Rolls1_ID, 8, processRolls1, NULL, 0 };
CANData Rolls2 = { &DeviceState.Rolls2, Rolls2_ID, 8, processRolls2, NULL, 0 };
CANData BrakeFront = { &DeviceState.BrakeFront, BrakeFront_ID, 8, processBrakeFront, NULL, 0 };
CANData BrakeRear = { &DeviceState.BrakeRear, BrakeRear_ID, 8, processBrakeRear, NULL, 0 };




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

