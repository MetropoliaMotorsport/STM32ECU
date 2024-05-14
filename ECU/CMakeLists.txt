cmake_minimum_required(VERSION 3.20)

# Set the project name and specify the languages used
project("ECU" C CXX ASM)

# Include custom CMake scripts
include(cmake/st-project.cmake)

# Glob all header files in the Inc directories
file(GLOB_RECURSE USER_INC
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/*.h
)

# Glob all source files in the Src directories
file(GLOB_RECURSE USER_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/startup/*.s  # Assembly files
)

# Add executable target
add_executable(${PROJECT_NAME} ${USER_INC} ${USER_SRC})

# Add properties and target-specific configurations
add_st_target_properties(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
)
