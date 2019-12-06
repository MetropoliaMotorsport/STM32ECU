EESchema Schematic File Version 4
LIBS:STM32_Shield-cache
EELAYER 26 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 8
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text GLabel 3050 3300 2    50   Input ~ 0
PC8
Text GLabel 3050 3400 2    50   Input ~ 0
PC9
Text GLabel 3050 3500 2    50   Input ~ 0
PC10
Text GLabel 3050 3600 2    50   Input ~ 0
PC11
Text GLabel 3050 3700 2    50   Input ~ 0
PC12
Text GLabel 3050 3800 2    50   Input ~ 0
PD2
Text GLabel 3050 3900 2    50   Input ~ 0
PG2
Text GLabel 3050 4000 2    50   Input ~ 0
PG3
$Comp
L power:GND #PWR04
U 1 1 5C068CFB
P 2350 3900
F 0 "#PWR04" H 2350 3650 50  0001 C CNN
F 1 "GND" V 2355 3772 50  0000 R CNN
F 2 "" H 2350 3900 50  0001 C CNN
F 3 "" H 2350 3900 50  0001 C CNN
	1    2350 3900
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR03
U 1 1 5C068D19
P 2350 3800
F 0 "#PWR03" H 2350 3550 50  0001 C CNN
F 1 "GND" V 2355 3672 50  0000 R CNN
F 2 "" H 2350 3800 50  0001 C CNN
F 3 "" H 2350 3800 50  0001 C CNN
	1    2350 3800
	0    1    1    0   
$EndComp
$Comp
L power:+5V #PWR02
U 1 1 5C069600
P 1750 3700
F 0 "#PWR02" H 1750 3550 50  0001 C CNN
F 1 "+5V" V 1765 3828 50  0000 L CNN
F 2 "" H 1750 3700 50  0001 C CNN
F 3 "" H 1750 3700 50  0001 C CNN
	1    1750 3700
	0    -1   -1   0   
$EndComp
$Comp
L power:+3.3V #PWR01
U 1 1 5C069667
P 2250 3600
F 0 "#PWR01" H 2250 3450 50  0001 C CNN
F 1 "+3.3V" V 2265 3728 50  0000 L CNN
F 2 "" H 2250 3600 50  0001 C CNN
F 3 "" H 2250 3600 50  0001 C CNN
	1    2250 3600
	0    -1   -1   0   
$EndComp
Wire Wire Line
	2350 3800 2450 3800
Wire Wire Line
	2350 3900 2450 3900
Wire Wire Line
	3050 4000 2950 4000
Wire Wire Line
	3050 3900 2950 3900
Wire Wire Line
	3050 3800 2950 3800
Wire Wire Line
	3050 3700 2950 3700
Wire Wire Line
	2950 3600 3050 3600
Wire Wire Line
	3050 3500 2950 3500
Wire Wire Line
	3050 3400 2950 3400
Wire Wire Line
	3050 3300 2950 3300
$Comp
L Connector_Generic:Conn_02x08_Odd_Even J1
U 1 1 5C0757DA
P 2650 3600
F 0 "J1" H 2700 4117 50  0000 C CNN
F 1 "Conn_02x08_Odd_Even" H 2700 4026 50  0000 C CNN
F 2 "STM32_Shield:2x08" H 2650 3600 50  0001 C CNN
F 3 "~" H 2650 3600 50  0001 C CNN
	1    2650 3600
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x15_Odd_Even J2
U 1 1 5C0758ED
P 2650 5100
F 0 "J2" H 2700 6017 50  0000 C CNN
F 1 "Conn_02x15_Odd_Even" H 2700 5926 50  0000 C CNN
F 2 "STM32_Shield:2x15" H 2650 5100 50  0001 C CNN
F 3 "~" H 2650 5100 50  0001 C CNN
	1    2650 5100
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x17_Odd_Even J5
U 1 1 5C0759E1
P 4350 5000
F 0 "J5" H 4400 6017 50  0000 C CNN
F 1 "Conn_02x17_Odd_Even" H 4400 5926 50  0000 C CNN
F 2 "STM32_Shield:2x17" H 4350 5000 50  0001 C CNN
F 3 "~" H 4350 5000 50  0001 C CNN
	1    4350 5000
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x10_Odd_Even J4
U 1 1 5C075B51
P 4350 3350
F 0 "J4" H 4400 3967 50  0000 C CNN
F 1 "Conn_02x10_Odd_Even" H 4400 3876 50  0000 C CNN
F 2 "STM32_Shield:2x10" H 4350 3350 50  0001 C CNN
F 3 "~" H 4350 3350 50  0001 C CNN
	1    4350 3350
	1    0    0    -1  
$EndComp
Text GLabel 2350 4400 0    50   Input ~ 0
PA3
Text GLabel 2350 4600 0    50   Input ~ 0
PC3
Text GLabel 2350 4500 0    50   Input ~ 0
PC0
Text GLabel 3050 4400 2    50   Input ~ 0
PD7
Text GLabel 3050 4500 2    50   Input ~ 0
PD6
Text GLabel 3050 4600 2    50   Input ~ 0
PD5
Text GLabel 3050 4700 2    50   Input ~ 0
PD4
Text GLabel 3050 4800 2    50   Input ~ 0
PD3
$Comp
L power:GND #PWR06
U 1 1 5C075EC0
P 3050 4900
F 0 "#PWR06" H 3050 4650 50  0001 C CNN
F 1 "GND" V 3055 4772 50  0000 R CNN
F 2 "" H 3050 4900 50  0001 C CNN
F 3 "" H 3050 4900 50  0001 C CNN
	1    3050 4900
	0    -1   -1   0   
$EndComp
Text GLabel 3050 5100 2    50   Input ~ 0
PE4
Text GLabel 3050 5200 2    50   Input ~ 0
PE5
Text GLabel 3050 5300 2    50   Input ~ 0
PE6
Text GLabel 3050 5400 2    50   Input ~ 0
PE3
Text GLabel 3050 5500 2    50   Input ~ 0
PF8
Text GLabel 3050 5600 2    50   Input ~ 0
PF7
Text GLabel 3050 5700 2    50   Input ~ 0
PF9
Text GLabel 3050 5800 2    50   Input ~ 0
PG1
Text GLabel 2350 5800 0    50   Input ~ 0
PG0
Text GLabel 2350 5700 0    50   Input ~ 0
PD1
Text GLabel 2350 5600 0    50   Input ~ 0
PD0
$Comp
L power:GND #PWR05
U 1 1 5C0761D3
P 2350 5500
F 0 "#PWR05" H 2350 5250 50  0001 C CNN
F 1 "GND" V 2355 5372 50  0000 R CNN
F 2 "" H 2350 5500 50  0001 C CNN
F 3 "" H 2350 5500 50  0001 C CNN
	1    2350 5500
	0    1    1    0   
$EndComp
Wire Wire Line
	2450 4400 2350 4400
Wire Wire Line
	2450 4500 2350 4500
Wire Wire Line
	2350 4600 2450 4600
Wire Wire Line
	2450 5500 2350 5500
Wire Wire Line
	2450 5600 2350 5600
Wire Wire Line
	2450 5700 2350 5700
Wire Wire Line
	2950 5800 3050 5800
Wire Wire Line
	3050 5700 2950 5700
Wire Wire Line
	2950 5600 3050 5600
Wire Wire Line
	3050 5500 2950 5500
Wire Wire Line
	3050 5400 2950 5400
Wire Wire Line
	2950 5300 3050 5300
Wire Wire Line
	2950 5200 3050 5200
Wire Wire Line
	3050 5100 2950 5100
Wire Wire Line
	3050 5000 2950 5000
Wire Wire Line
	3050 4900 2950 4900
Wire Wire Line
	3050 4800 2950 4800
Wire Wire Line
	3050 4700 2950 4700
Wire Wire Line
	2950 4600 3050 4600
Wire Wire Line
	3050 4500 2950 4500
Wire Wire Line
	2950 4400 3050 4400
Text GLabel 4050 2950 0    50   Input ~ 0
PC6
Text GLabel 4050 3050 0    50   Input ~ 0
PB15
Text GLabel 4050 3150 0    50   Input ~ 0
PB13
Text GLabel 4050 3250 0    50   Input ~ 0
PB12
Text GLabel 4050 3350 0    50   Input ~ 0
PA15
Text GLabel 4050 3450 0    50   Input ~ 0
PC7
Text GLabel 4050 3550 0    50   Input ~ 0
PB5
Text GLabel 4050 3650 0    50   Input ~ 0
PB3
Text GLabel 4050 3750 0    50   Input ~ 0
PA4
Text GLabel 4050 3850 0    50   Input ~ 0
PB4
Text GLabel 4750 2950 2    50   Input ~ 0
PB8
Text GLabel 4750 3050 2    50   Input ~ 0
PB9
$Comp
L power:GND #PWR010
U 1 1 5C0957AF
P 4750 3250
F 0 "#PWR010" H 4750 3000 50  0001 C CNN
F 1 "GND" V 4755 3122 50  0000 R CNN
F 2 "" H 4750 3250 50  0001 C CNN
F 3 "" H 4750 3250 50  0001 C CNN
	1    4750 3250
	0    -1   -1   0   
$EndComp
Text GLabel 4750 3350 2    50   Input ~ 0
PA5
Text GLabel 4750 3450 2    50   Input ~ 0
PA6
Text GLabel 4750 3650 2    50   Input ~ 0
PD14
Text GLabel 4750 3750 2    50   Input ~ 0
PD15
Text GLabel 4750 3850 2    50   Input ~ 0
PF12
Wire Wire Line
	4150 2950 4050 2950
Wire Wire Line
	4050 3050 4150 3050
Wire Wire Line
	4150 3150 4050 3150
Wire Wire Line
	4050 3250 4150 3250
Wire Wire Line
	4150 3350 4050 3350
Wire Wire Line
	4050 3450 4150 3450
Wire Wire Line
	4050 3550 4150 3550
Wire Wire Line
	4150 3650 4050 3650
Wire Wire Line
	4150 3750 4050 3750
Wire Wire Line
	4050 3850 4150 3850
Wire Wire Line
	4750 3850 4650 3850
Wire Wire Line
	4750 3750 4650 3750
Wire Wire Line
	4650 3650 4750 3650
Wire Wire Line
	4750 3450 4650 3450
Wire Wire Line
	4650 3350 4750 3350
Wire Wire Line
	4650 3250 4750 3250
Wire Wire Line
	4750 3050 4650 3050
Wire Wire Line
	4650 2950 4750 2950
$Comp
L power:GND #PWR07
U 1 1 5C0A4D80
P 4050 4400
F 0 "#PWR07" H 4050 4150 50  0001 C CNN
F 1 "GND" V 4055 4272 50  0000 R CNN
F 2 "" H 4050 4400 50  0001 C CNN
F 3 "" H 4050 4400 50  0001 C CNN
	1    4050 4400
	0    1    1    0   
$EndComp
Text GLabel 4050 4500 0    50   Input ~ 0
PB1
Text GLabel 4050 4600 0    50   Input ~ 0
PC2
Text GLabel 4050 4800 0    50   Input ~ 0
PB6
Text GLabel 4050 4900 0    50   Input ~ 0
PB2
$Comp
L power:GND #PWR08
U 1 1 5C0A4ED3
P 4050 5000
F 0 "#PWR08" H 4050 4750 50  0001 C CNN
F 1 "GND" V 4055 4872 50  0000 R CNN
F 2 "" H 4050 5000 50  0001 C CNN
F 3 "" H 4050 5000 50  0001 C CNN
	1    4050 5000
	0    1    1    0   
$EndComp
Text GLabel 4050 5400 0    50   Input ~ 0
PE2
$Comp
L power:GND #PWR09
U 1 1 5C0A4F71
P 4050 5500
F 0 "#PWR09" H 4050 5250 50  0001 C CNN
F 1 "GND" V 4055 5372 50  0000 R CNN
F 2 "" H 4050 5500 50  0001 C CNN
F 3 "" H 4050 5500 50  0001 C CNN
	1    4050 5500
	0    1    1    0   
$EndComp
Text GLabel 4050 5700 0    50   Input ~ 0
PB0
Text GLabel 4050 5800 0    50   Input ~ 0
PE0
Text GLabel 4750 4200 2    50   Input ~ 0
PF13
Text GLabel 4750 4300 2    50   Input ~ 0
PE9
Text GLabel 4750 4400 2    50   Input ~ 0
PE11
Text GLabel 4750 4500 2    50   Input ~ 0
PF14
Text GLabel 4750 4600 2    50   Input ~ 0
PE13
Text GLabel 4750 4700 2    50   Input ~ 0
PF15
Text GLabel 4750 5000 2    50   Input ~ 0
PE8
Text GLabel 4750 5100 2    50   Input ~ 0
PE7
Text GLabel 4750 5300 2    50   Input ~ 0
PE10
Text GLabel 4750 5400 2    50   Input ~ 0
PE12
Text GLabel 4750 5500 2    50   Input ~ 0
PE14
Text GLabel 4750 5600 2    50   Input ~ 0
PE15
Text GLabel 4750 5700 2    50   Input ~ 0
PB10
Text GLabel 4750 5800 2    50   Input ~ 0
PB11
$Comp
L power:GND #PWR011
U 1 1 5C0A53FA
P 4750 5200
F 0 "#PWR011" H 4750 4950 50  0001 C CNN
F 1 "GND" V 4755 5072 50  0000 R CNN
F 2 "" H 4750 5200 50  0001 C CNN
F 3 "" H 4750 5200 50  0001 C CNN
	1    4750 5200
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4150 4400 4050 4400
Wire Wire Line
	4050 4500 4150 4500
Wire Wire Line
	4150 4600 4050 4600
Wire Wire Line
	4150 4800 4050 4800
Wire Wire Line
	4050 4900 4150 4900
Wire Wire Line
	4150 5000 4050 5000
Wire Wire Line
	4050 5400 4150 5400
Wire Wire Line
	4150 5500 4050 5500
Wire Wire Line
	4050 5700 4150 5700
Wire Wire Line
	4150 5800 4050 5800
Wire Wire Line
	4650 5800 4750 5800
Wire Wire Line
	4750 5700 4650 5700
Wire Wire Line
	4650 5600 4750 5600
Wire Wire Line
	4750 5500 4650 5500
Wire Wire Line
	4650 5400 4750 5400
Wire Wire Line
	4750 5300 4650 5300
Wire Wire Line
	4750 5200 4650 5200
Wire Wire Line
	4750 5100 4650 5100
Wire Wire Line
	4650 5000 4750 5000
Wire Wire Line
	4750 4700 4650 4700
Wire Wire Line
	4750 4600 4650 4600
Wire Wire Line
	4650 4500 4750 4500
Wire Wire Line
	4750 4400 4650 4400
Wire Wire Line
	4650 4300 4750 4300
Wire Wire Line
	4750 4200 4650 4200
$Sheet
S 9150 850  500  150 
U 5C0D56DF
F0 "CAN1" 50
F1 "CAN1.sch" 50
$EndSheet
$Sheet
S 9150 1200 500  150 
U 5C0D5742
F0 "CAN2" 50
F1 "CAN2.sch" 50
$EndSheet
$Sheet
S 9150 1650 500  150 
U 5C0DECD3
F0 "voltage_dividers" 50
F1 "voltage_dividers.sch" 50
$EndSheet
$Sheet
S 9150 2100 500  150 
U 5C0AF670
F0 "output_drivers" 50
F1 "output_drivers.sch" 50
$EndSheet
$Comp
L Connector_Generic:Conn_01x34 J3
U 1 1 5C193F97
P 10450 4550
F 0 "J3" H 10530 4542 50  0000 L CNN
F 1 "Conn_01x34" H 10530 4451 50  0000 L CNN
F 2 "STM32_Shield:SUPERSEAL_34POS" H 10450 4550 50  0001 C CNN
F 3 "~" H 10450 4550 50  0001 C CNN
	1    10450 4550
	1    0    0    -1  
$EndComp
Wire Wire Line
	10250 2950 10150 2950
Wire Wire Line
	10250 3050 10150 3050
Wire Wire Line
	10250 3150 10150 3150
Wire Wire Line
	10250 3250 10150 3250
Wire Wire Line
	10250 3350 10150 3350
Wire Wire Line
	10250 3450 10150 3450
Wire Wire Line
	10250 3550 10150 3550
Wire Wire Line
	10250 3650 10150 3650
Wire Wire Line
	10250 3850 10150 3850
Wire Wire Line
	10250 3950 10150 3950
Wire Wire Line
	10250 4050 10150 4050
Wire Wire Line
	10250 4150 10150 4150
Wire Wire Line
	10250 4250 10150 4250
Wire Wire Line
	10250 4350 10150 4350
Wire Wire Line
	10150 4450 10250 4450
Wire Wire Line
	10250 4550 10150 4550
Wire Wire Line
	10250 4650 10150 4650
Wire Wire Line
	10250 4750 10150 4750
Wire Wire Line
	10250 4850 10150 4850
Wire Wire Line
	10150 4950 10250 4950
Wire Wire Line
	10250 5050 10150 5050
Wire Wire Line
	10250 5150 10150 5150
Wire Wire Line
	10150 5250 10250 5250
Wire Wire Line
	10250 5350 10150 5350
Wire Wire Line
	10250 5550 10150 5550
Wire Wire Line
	10250 5650 10150 5650
Wire Wire Line
	10250 5750 10150 5750
Wire Wire Line
	10150 5850 10250 5850
Wire Wire Line
	10150 5950 10250 5950
Wire Wire Line
	10250 6050 10150 6050
Wire Wire Line
	10250 6150 10150 6150
Wire Wire Line
	10250 6250 10150 6250
Text GLabel 10150 6150 0    50   Input ~ 0
CANHI2
Text GLabel 10150 6250 0    50   Input ~ 0
CANLO2
Text GLabel 10150 5950 0    50   Input ~ 0
CANHI1
Text GLabel 10150 6050 0    50   Input ~ 0
CANLO1
Text GLabel 10150 3450 0    50   Input ~ 0
AI0
Text GLabel 10150 3550 0    50   Input ~ 0
AI1
Text GLabel 10150 3350 0    50   Input ~ 0
AI2
Text GLabel 10150 3650 0    50   Input ~ 0
AI3
Text GLabel 10150 3250 0    50   Input ~ 0
AI4
Text GLabel 10150 3150 0    50   Input ~ 0
AI5
Text GLabel 10150 3050 0    50   Input ~ 0
AI6
Text GLabel 10150 2950 0    50   Input ~ 0
AI7
$Sheet
S 9150 2550 500  150 
U 5C3FB407
F0 "digital_in" 50
F1 "digital_in.sch" 50
$EndSheet
Text GLabel 10150 3850 0    50   Input ~ 0
DO0
Text GLabel 10150 3950 0    50   Input ~ 0
DO1
Text GLabel 10150 4050 0    50   Input ~ 0
DO2
Text GLabel 10150 4150 0    50   Input ~ 0
DO3
Text GLabel 10150 4250 0    50   Input ~ 0
DO4
Text GLabel 10150 4350 0    50   Input ~ 0
DO5
Text GLabel 10150 4450 0    50   Input ~ 0
DO6
Text GLabel 10150 4550 0    50   Input ~ 0
DO7
Text GLabel 4750 3550 2    50   Input ~ 0
PA7
Wire Wire Line
	4750 3550 4650 3550
Text GLabel 4050 4700 0    50   Input ~ 0
PF4
Wire Wire Line
	4050 4700 4150 4700
Text GLabel 4750 4800 2    50   Input ~ 0
PG14
Text GLabel 4750 4900 2    50   Input ~ 0
PG9
Wire Wire Line
	4750 4900 4650 4900
Wire Wire Line
	4750 4800 4650 4800
Text GLabel 4050 5100 0    50   Input ~ 0
PD13
Text GLabel 4050 5200 0    50   Input ~ 0
PD12
Text GLabel 4050 5300 0    50   Input ~ 0
PD11
Wire Wire Line
	4050 5300 4150 5300
Wire Wire Line
	4150 5200 4050 5200
Wire Wire Line
	4150 5100 4050 5100
Text GLabel 4050 5600 0    50   Input ~ 0
PA0
Wire Wire Line
	4050 5600 4150 5600
Text GLabel 2350 4700 0    50   Input ~ 0
PF3
Text GLabel 2350 4800 0    50   Input ~ 0
PF5
Text GLabel 2350 4900 0    50   Input ~ 0
PF10
Text GLabel 2350 5200 0    50   Input ~ 0
PF2
Text GLabel 2350 5300 0    50   Input ~ 0
PF1
Text GLabel 2350 5400 0    50   Input ~ 0
PF0
Wire Wire Line
	2350 5400 2450 5400
Wire Wire Line
	2350 5200 2450 5200
Wire Wire Line
	2450 4900 2350 4900
Wire Wire Line
	2450 4800 2350 4800
Wire Wire Line
	2450 4700 2350 4700
Text GLabel 10150 4650 0    50   Input ~ 0
DI0
Text GLabel 10150 4750 0    50   Input ~ 0
DI1
Text GLabel 10150 4850 0    50   Input ~ 0
DI2
Text GLabel 10150 4950 0    50   Input ~ 0
DI3
Text GLabel 10150 5050 0    50   Input ~ 0
DI4
Text GLabel 10150 5150 0    50   Input ~ 0
DI5
Text GLabel 10150 5250 0    50   Input ~ 0
DI6
Text GLabel 10150 5350 0    50   Input ~ 0
DI7
$Sheet
S 9150 3000 500  150 
U 5C451501
F0 "periphs" 50
F1 "periphs.sch" 50
$EndSheet
Text GLabel 10150 5850 0    50   Input ~ 0
I2C2SDA
Text GLabel 10150 5750 0    50   Input ~ 0
I2C2SCK
NoConn ~ 4150 4200
NoConn ~ 4150 4300
NoConn ~ 2450 5000
NoConn ~ 2450 5100
NoConn ~ 2450 3500
NoConn ~ 2450 3400
NoConn ~ 2450 3300
NoConn ~ 4650 3150
Text GLabel 10150 5550 0    50   Input ~ 0
ISOSPI_IP
Text GLabel 10150 5650 0    50   Input ~ 0
ISOSPI_IM
Text Notes 5750 4400 0    50   ~ 0
NOT CONNECTED PINS\n
Text GLabel 5950 4500 0    50   Input ~ 0
PG2
Text GLabel 5950 4600 0    50   Input ~ 0
PG3
Text GLabel 5950 4800 0    50   Input ~ 0
PD7
Text GLabel 6550 4500 0    50   Input ~ 0
PC6
Text GLabel 6550 4600 0    50   Input ~ 0
PB15
Text GLabel 6550 4700 0    50   Input ~ 0
PA15
Text GLabel 6550 4800 0    50   Input ~ 0
PC7
Text GLabel 6550 4900 0    50   Input ~ 0
PB5
Text GLabel 6550 5000 0    50   Input ~ 0
PB3
Text GLabel 6550 5100 0    50   Input ~ 0
PB4
Text GLabel 6550 5200 0    50   Input ~ 0
PD12
Text GLabel 6550 5300 0    50   Input ~ 0
PD11
Text GLabel 6550 5400 0    50   Input ~ 0
PB0
Text GLabel 6550 5500 0    50   Input ~ 0
PE0
Text GLabel 6550 5600 0    50   Input ~ 0
PE15
Text GLabel 6550 5700 0    50   Input ~ 0
PB11
NoConn ~ 6550 5500
NoConn ~ 6550 5600
NoConn ~ 6550 4600
NoConn ~ 6550 5300
NoConn ~ 6550 5200
NoConn ~ 5950 4500
NoConn ~ 5950 4600
NoConn ~ 6550 4800
NoConn ~ 6550 4500
NoConn ~ 6550 4700
NoConn ~ 5950 4800
NoConn ~ 6550 5100
NoConn ~ 6550 4900
Wire Wire Line
	6550 5000 6600 5000
NoConn ~ 6550 5400
Text Notes 6600 5450 0    50   ~ 0
LED1
NoConn ~ 6550 5700
NoConn ~ 6600 5000
Wire Wire Line
	2350 5800 2450 5800
Wire Wire Line
	2350 5300 2450 5300
Text Notes 2050 5850 0    50   ~ 0
?
Wire Wire Line
	9750 3750 9850 3750
Wire Wire Line
	9850 3750 9850 3650
Connection ~ 9850 3750
Wire Wire Line
	9850 3750 10250 3750
$Comp
L power:+12V #PWR0101
U 1 1 5C404639
P 9650 5350
F 0 "#PWR0101" H 9650 5200 50  0001 C CNN
F 1 "+12V" H 9665 5523 50  0000 C CNN
F 2 "" H 9650 5350 50  0001 C CNN
F 3 "" H 9650 5350 50  0001 C CNN
	1    9650 5350
	1    0    0    -1  
$EndComp
$Comp
L power:PWR_FLAG #FLG0101
U 1 1 5C404675
P 9850 3650
F 0 "#FLG0101" H 9850 3725 50  0001 C CNN
F 1 "PWR_FLAG" H 9850 3824 50  0001 C CNN
F 2 "" H 9850 3650 50  0001 C CNN
F 3 "~" H 9850 3650 50  0001 C CNN
	1    9850 3650
	1    0    0    -1  
$EndComp
$Comp
L power:PWR_FLAG #FLG0102
U 1 1 5C4050ED
P 2300 3500
F 0 "#FLG0102" H 2300 3575 50  0001 C CNN
F 1 "PWR_FLAG" H 2300 3674 50  0001 C CNN
F 2 "" H 2300 3500 50  0001 C CNN
F 3 "~" H 2300 3500 50  0001 C CNN
	1    2300 3500
	1    0    0    -1  
$EndComp
Text GLabel 5950 4700 0    50   Input ~ 0
PF12
NoConn ~ 5950 4700
Wire Wire Line
	2250 3600 2300 3600
Wire Wire Line
	2300 3600 2300 3500
Connection ~ 2300 3600
Wire Wire Line
	2300 3600 2450 3600
$Comp
L power:PWR_FLAG #FLG0104
U 1 1 5C447E9F
P 9550 5350
F 0 "#FLG0104" H 9550 5425 50  0001 C CNN
F 1 "PWR_FLAG" H 9550 5524 50  0001 C CNN
F 2 "" H 9550 5350 50  0001 C CNN
F 3 "~" H 9550 5350 50  0001 C CNN
	1    9550 5350
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0102
U 1 1 5C448124
P 9750 3850
F 0 "#PWR0102" H 9750 3600 50  0001 C CNN
F 1 "GND" V 9755 3722 50  0000 R CNN
F 2 "" H 9750 3850 50  0001 C CNN
F 3 "" H 9750 3850 50  0001 C CNN
	1    9750 3850
	1    0    0    -1  
$EndComp
Wire Wire Line
	9550 5450 9550 5350
$Comp
L Regulator_Linear:L7805 U6
U 1 1 5C458C3D
P 2800 1200
F 0 "U6" H 2800 1442 50  0000 C CNN
F 1 "L7805" H 2800 1351 50  0000 C CNN
F 2 "TO_SOT_Packages_SMD:TO-252-2_Rectifier" H 2825 1050 50  0001 L CIN
F 3 "http://www.st.com/content/ccc/resource/technical/document/datasheet/41/4f/b3/b0/12/d4/47/88/CD00000444.pdf/files/CD00000444.pdf/jcr:content/translations/en.CD00000444.pdf" H 2800 1150 50  0001 C CNN
	1    2800 1200
	1    0    0    -1  
$EndComp
Wire Wire Line
	2500 1200 2400 1200
Wire Wire Line
	2400 1200 2400 1100
$Comp
L power:+12V #PWR0130
U 1 1 5C469BCE
P 2400 1100
F 0 "#PWR0130" H 2400 950 50  0001 C CNN
F 1 "+12V" H 2415 1273 50  0000 C CNN
F 2 "" H 2400 1100 50  0001 C CNN
F 3 "" H 2400 1100 50  0001 C CNN
	1    2400 1100
	1    0    0    -1  
$EndComp
Wire Wire Line
	2800 1500 2800 1600
$Comp
L power:GND #PWR0131
U 1 1 5C4722B8
P 2800 1600
F 0 "#PWR0131" H 2800 1350 50  0001 C CNN
F 1 "GND" H 2900 1450 50  0000 R CNN
F 2 "" H 2800 1600 50  0001 C CNN
F 3 "" H 2800 1600 50  0001 C CNN
	1    2800 1600
	1    0    0    -1  
$EndComp
$Comp
L general:C C28
U 1 1 5C472E96
P 3200 1450
F 0 "C28" H 3315 1496 50  0000 L CNN
F 1 "100nF" H 3315 1405 50  0000 L CNN
F 2 "general:C_0805_HandSoldering" H 3238 1300 50  0001 C CNN
F 3 "" H 3200 1450 50  0001 C CNN
	1    3200 1450
	1    0    0    -1  
$EndComp
Wire Wire Line
	3100 1200 3200 1200
Wire Wire Line
	3200 1600 3200 1700
Wire Wire Line
	3200 1300 3200 1200
Connection ~ 3200 1200
Wire Wire Line
	3200 1200 3300 1200
$Comp
L power:GND #PWR0132
U 1 1 5C48C27F
P 3200 1700
F 0 "#PWR0132" H 3200 1450 50  0001 C CNN
F 1 "GND" H 3300 1550 50  0000 R CNN
F 2 "" H 3200 1700 50  0001 C CNN
F 3 "" H 3200 1700 50  0001 C CNN
	1    3200 1700
	1    0    0    -1  
$EndComp
$Comp
L general:C C19
U 1 1 5C48D591
P 2400 1450
F 0 "C19" H 2150 1500 50  0000 L CNN
F 1 "330nF" H 2050 1400 50  0000 L CNN
F 2 "general:C_0805_HandSoldering" H 2438 1300 50  0001 C CNN
F 3 "" H 2400 1450 50  0001 C CNN
	1    2400 1450
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0133
U 1 1 5C49E6E9
P 2400 1700
F 0 "#PWR0133" H 2400 1450 50  0001 C CNN
F 1 "GND" H 2500 1550 50  0000 R CNN
F 2 "" H 2400 1700 50  0001 C CNN
F 3 "" H 2400 1700 50  0001 C CNN
	1    2400 1700
	1    0    0    -1  
$EndComp
Wire Wire Line
	2400 1700 2400 1600
Wire Wire Line
	2400 1300 2400 1200
Connection ~ 2400 1200
Wire Wire Line
	9750 3850 9750 3750
Wire Wire Line
	9550 5450 9650 5450
Wire Wire Line
	9650 5450 9650 5350
Connection ~ 9650 5450
Wire Wire Line
	9650 5450 10250 5450
$Comp
L power:+5V #PWR0134
U 1 1 5C29B3F9
P 3300 1100
F 0 "#PWR0134" H 3300 950 50  0001 C CNN
F 1 "+5V" V 3315 1228 50  0000 L CNN
F 2 "" H 3300 1100 50  0001 C CNN
F 3 "" H 3300 1100 50  0001 C CNN
	1    3300 1100
	1    0    0    -1  
$EndComp
Wire Wire Line
	3300 1100 3300 1200
Wire Wire Line
	1750 3700 2450 3700
NoConn ~ 2450 4000
$Sheet
S 2150 7050 500  150 
U 5C29E476
F0 "stripboards" 50
F1 "stripboards.sch" 50
$EndSheet
NoConn ~ 3050 5000
Text Notes 3100 5050 0    50   ~ 0
PE2
$EndSCHEMATC
