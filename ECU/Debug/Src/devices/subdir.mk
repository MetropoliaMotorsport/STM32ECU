################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/devices/adcecu.c \
../Src/devices/bms.c \
../Src/devices/canecu.c \
../Src/devices/dwt_delay.c \
../Src/devices/input.c \
../Src/devices/interrupts.c \
../Src/devices/inverter.c \
../Src/devices/ivt.c \
../Src/devices/output.c \
../Src/devices/pdm.c \
../Src/devices/sickencoder.c \
../Src/devices/timerecu.c \
../Src/devices/vhd44780.c 

OBJS += \
./Src/devices/adcecu.o \
./Src/devices/bms.o \
./Src/devices/canecu.o \
./Src/devices/dwt_delay.o \
./Src/devices/input.o \
./Src/devices/interrupts.o \
./Src/devices/inverter.o \
./Src/devices/ivt.o \
./Src/devices/output.o \
./Src/devices/pdm.o \
./Src/devices/sickencoder.o \
./Src/devices/timerecu.o \
./Src/devices/vhd44780.o 

C_DEPS += \
./Src/devices/adcecu.d \
./Src/devices/bms.d \
./Src/devices/canecu.d \
./Src/devices/dwt_delay.d \
./Src/devices/input.d \
./Src/devices/interrupts.d \
./Src/devices/inverter.d \
./Src/devices/ivt.d \
./Src/devices/output.d \
./Src/devices/pdm.d \
./Src/devices/sickencoder.d \
./Src/devices/timerecu.d \
./Src/devices/vhd44780.d 


# Each subdirectory must supply rules for building sources it contributes
Src/devices/adcecu.o: ../Src/devices/adcecu.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Inc/states -I../Inc/devices -I../Inc -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/devices/adcecu.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/devices/bms.o: ../Src/devices/bms.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Inc/states -I../Inc/devices -I../Inc -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/devices/bms.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/devices/canecu.o: ../Src/devices/canecu.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Inc/states -I../Inc/devices -I../Inc -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/devices/canecu.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/devices/dwt_delay.o: ../Src/devices/dwt_delay.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Inc/states -I../Inc/devices -I../Inc -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/devices/dwt_delay.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/devices/input.o: ../Src/devices/input.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Inc/states -I../Inc/devices -I../Inc -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/devices/input.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/devices/interrupts.o: ../Src/devices/interrupts.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Inc/states -I../Inc/devices -I../Inc -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/devices/interrupts.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/devices/inverter.o: ../Src/devices/inverter.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Inc/states -I../Inc/devices -I../Inc -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/devices/inverter.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/devices/ivt.o: ../Src/devices/ivt.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Inc/states -I../Inc/devices -I../Inc -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/devices/ivt.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/devices/output.o: ../Src/devices/output.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Inc/states -I../Inc/devices -I../Inc -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/devices/output.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/devices/pdm.o: ../Src/devices/pdm.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Inc/states -I../Inc/devices -I../Inc -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/devices/pdm.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/devices/sickencoder.o: ../Src/devices/sickencoder.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Inc/states -I../Inc/devices -I../Inc -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/devices/sickencoder.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/devices/timerecu.o: ../Src/devices/timerecu.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Inc/states -I../Inc/devices -I../Inc -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/devices/timerecu.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/devices/vhd44780.o: ../Src/devices/vhd44780.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Inc/states -I../Inc/devices -I../Inc -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/devices/vhd44780.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

