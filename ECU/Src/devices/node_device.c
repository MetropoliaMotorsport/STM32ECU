
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
	volatile uint16_t message = 0;

		for (int i = 0; i < datahandle->dlcsize; i++) {
			message |= CANRxData[i] << (i * 8);
		}

		datahandle->data = message;

		return 1;
}

bool processBPPS(const uint8_t CANRxData[8], const uint32_t DataLength, CANData *datahandle){
	volatile uint16_t message = 0;

		for (int i = 0; i < datahandle->dlcsize; i++) {
			message |= CANRxData[i] << (i * 8);
		}

		datahandle->data = message;

		uint8_t bData[2] = {0, 0};
		bData[0] = 4;

		if(datahandle->data > 10){

			bData[1] = 1;
		}
		else{

			bData[1] = 0;
		}

		CAN2Send(PNode2_CMD_ID, 2, bData);

		return 1;
}



bool processBTN(const uint8_t CANRxData[8], const uint32_t DataLength, CANData *datahandle){

    datahandle->data = CANRxData[0];

    return 1;
}




volatile CANData BPPS = { &DeviceState.BPPS, BPPS_ID, 2, processBPPS, NULL, 0, processBPPS };
volatile CANData APPS1 = { &DeviceState.APPS1, APPS1_ID, 2, processNodeDevice, NULL, 0,processNodeDevice };
volatile CANData APPS2 = { &DeviceState.APPS2, APPS2_ID, 2, processNodeDevice, NULL, 0, processNodeDevice };

volatile CANData SteeringAngle = { &DeviceState.SteeringAngle, SteeringAngle_ID, 8, processNodeDevice, NULL, 0 };
volatile CANData WaterLevel = { &DeviceState.WaterLevel, WaterLevel_ID, 8, processNodeDevice, NULL, 0 };
volatile CANData HeavesRear = { &DeviceState.HeavesRear, HeavesRear_ID, 8, processNodeDevice, NULL, 0 };
volatile CANData HeavesFront = { &DeviceState.HeavesFront, HeavesFront_ID, 8, processNodeDevice, NULL, 0 };
volatile CANData Rolls1 = { &DeviceState.Rolls1, Rolls1_ID, 8, processNodeDevice, NULL, 0 };
volatile CANData Rolls2 = { &DeviceState.Rolls2, Rolls2_ID, 8, processNodeDevice, NULL, 0 };
volatile CANData BrakeFront = { &DeviceState.BrakeFront, BrakeFront_ID, 8, processNodeDevice, NULL, 0 };
volatile CANData BrakeRear = { &DeviceState.BrakeRear, BrakeRear_ID, 8, processNodeDevice, NULL, 0 };

volatile CANData BTN1 = { &DeviceState.Dash_BTNs, BTN1_ID, 2, processBTN, NULL, 0 };
volatile CANData BTN2 = { &DeviceState.Dash_BTNs, BTN2_ID, 2, processBTN, NULL, 0 };
volatile CANData BTN3 = { &DeviceState.Dash_BTNs, BTN3_ID, 2, processBTN, NULL, 0 };

uint32_t getAnalogueNodesOnline(){
    return 0;
}

void initNodeDevices( void ){

    RegisterCan2Message( &BPPS );
    RegisterCan2Message( &APPS1 );
    RegisterCan2Message( &APPS2 );

    RegisterCan2Message( &BTN1 );
    RegisterCan2Message( &BTN2 );
    RegisterCan2Message( &BTN3 );



    RegisterCan2Message( &SteeringAngle );
    RegisterCan2Message( &WaterLevel );
    RegisterCan2Message( &HeavesFront );
    RegisterCan2Message( &HeavesRear );
    RegisterCan2Message( &Rolls1 );
    RegisterCan2Message( &Rolls2 );
    RegisterCan2Message( &BrakeFront );
    RegisterCan2Message( &BrakeRear );




    //TODO add more devices

}

