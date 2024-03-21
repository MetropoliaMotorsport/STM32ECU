cmake_minimum_required(VERSION 3.20)
project("ECU" C CXX ASM)

include(cmake/st-project.cmake)
# Include directories
include_directories(
  Core/Inc
  Drivers/STM32H7xx_HAL_Driver/Inc
  Drivers/STM32H7xx_HAL_Driver/Inc/Legacy
  Drivers/CMSIS/Device/ST/STM32H7xx/Include
  Drivers/CMSIS/Include
  Middlewares/Third_Party/FreeRTOS/Source/include
  Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
  Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
  Src
  Inc
)

# Source files
file(GLOB_RECURSE SOURCES
  "Core/Src/*.c"
  "Drivers/STM32H7xx_HAL_Driver/Src/*.c"
  "Middlewares/Third_Party/FreeRTOS/Source/*.c"
  "Src/*.c"
)

# ASM startup file
set(ASM_SOURCES startup/startup_stm32h743xx.s)

# Define the executable
add_executable(${PROJECT_NAME}.elf ${SOURCES} ${ASM_SOURCES})

# Post-build commands
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.hex
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.bin
)
