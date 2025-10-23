# Proposed layout

| CAN ID Range | Base Object / Message Type                 | Sender/Receiver Node(s)              |
| :----------- | :----------------------------------------- | :----------------------------------- |
| 1 - 5        | Errors For This Subtype                    | ECU, PowerNodes (PN), Inverters, BMS |
| 6 - 99       | Core Control & High-Priority Device Status | ECU, PowerNodes (PN), Inverters, BMS |
| 100 - 199    | Analog & Digital Sensor Data               | AnalogNodes (AN), DashBoard          |
| 200 - 299    | Driver/Dash Interface & User Input         | DashBoard, ECU                       |
| 300 - 305    | ECU Errors                                 | ECU                                  |
| 306 - 349    | ECU State Transitions                      | ECU                                  |
| 350 - 399    | ECU State Requests/Commands                | ECU                                  |
| 400 - 499    | Telemetry & External Devices               | IVT_Mod, AMS, GPS                    |
| 500++        | Absolutely Nothing                         | None                                 |

# AnalogNode1 -> Controls the front (all have a message signal)

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

# AnalogNode2 -> Controls the rear (all have a message signal)

- BrakeRear
- WaterLevel
- HeavesRear
- Rolls2
