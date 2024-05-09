cmake_minimum_required(VERSION 3.20)
project("ECU" LANGUAGES C CXX ASM)

# Specify the path to the toolchain file
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/gcc-arm-none-eabi.cmake)

include(cmake/st-project.cmake)

# Define the executable
add_executable(${PROJECT_NAME}.elf)

# Include directories
target_include_directories(${PROJECT_NAME}.elf PRIVATE
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
  "Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/*.c"
  "Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/*.c"
  "Src/*.c"
)

file(GLOB_RECURSE HEADERS
  "Core/Inc/*.h"
  "Drivers/STM32H7xx_HAL_Driver/Inc/*.h"
  "Drivers/STM32H7xx_HAL_Driver/Inc/Legacy/*.h"
  "Drivers/CMSIS/Device/ST/STM32H7xx/Include/*.h"
  "Drivers/CMSIS/Include/*.h"
  "Middlewares/Third_Party/FreeRTOS/Source/include/*.h"
  "Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/*.h"
  "Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/*.h"
  "Inc/*.h"
)

target_sources(${PROJECT_NAME}.elf PRIVATE ${SOURCES})

# ASM startup file
set(ASM_SOURCES startup/startup_stm32h743xx.s)
target_sources(${PROJECT_NAME}.elf PRIVATE ${ASM_SOURCES})

# Post-build commands
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.hex
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.bin
)

# Compiler options, linker scripts, etc, should be specified too
# Example: target_compile_options, target_link_libraries, etc.

