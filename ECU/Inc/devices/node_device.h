
#ifndef NODE_DEVICES_H_
#define NODE_DEVICES_H_

extern CANData BPPS;
extern CANData APPS1;
extern CANData APPS2;
extern CANData SteeringAngle;
extern CANData WaterLevel;
extern CANData HeavesFront;
extern CANData HeavesRear;
extern CANData Rolls1;
extern CANData Rolls2;
extern CANData BrakeFront;
extern CANData BrakeRear;

extern CANData BTN1;
extern CANData BTN2;
extern CANData BTN3;

//TODO finnish this

int getNodeWait( void );
uint32_t getAnalogueNodesOnline( void );

void initNodeDevices( void );

#endif /* NODE_DEVICES_H_ */
