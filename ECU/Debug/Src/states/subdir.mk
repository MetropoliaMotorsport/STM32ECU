################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/states/configuration.c \
../Src/states/idleprocess.c \
../Src/states/operationalprocess.c \
../Src/states/operationreadyness.c \
../Src/states/preoperation.c \
../Src/states/runningprocess.c \
../Src/states/tsactiveprocess.c 

OBJS += \
./Src/states/configuration.o \
./Src/states/idleprocess.o \
./Src/states/operationalprocess.o \
./Src/states/operationreadyness.o \
./Src/states/preoperation.o \
./Src/states/runningprocess.o \
./Src/states/tsactiveprocess.o 

C_DEPS += \
./Src/states/configuration.d \
./Src/states/idleprocess.d \
./Src/states/operationalprocess.d \
./Src/states/operationreadyness.d \
./Src/states/preoperation.d \
./Src/states/runningprocess.d \
./Src/states/tsactiveprocess.d 


# Each subdirectory must supply rules for building sources it contributes
Src/states/configuration.o: ../Src/states/configuration.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Inc/states -I../Inc/devices -I../Inc -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/states/configuration.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/states/idleprocess.o: ../Src/states/idleprocess.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Inc/states -I../Inc/devices -I../Inc -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/states/idleprocess.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/states/operationalprocess.o: ../Src/states/operationalprocess.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Inc/states -I../Inc/devices -I../Inc -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/states/operationalprocess.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/states/operationreadyness.o: ../Src/states/operationreadyness.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Inc/states -I../Inc/devices -I../Inc -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/states/operationreadyness.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/states/preoperation.o: ../Src/states/preoperation.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Inc/states -I../Inc/devices -I../Inc -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/states/preoperation.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/states/runningprocess.o: ../Src/states/runningprocess.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Inc/states -I../Inc/devices -I../Inc -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/states/runningprocess.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/states/tsactiveprocess.o: ../Src/states/tsactiveprocess.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Inc/states -I../Inc/devices -I../Inc -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/states/tsactiveprocess.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

