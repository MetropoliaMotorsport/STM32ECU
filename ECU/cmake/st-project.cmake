# THIS FILE IS AUTOMATICALLY GENERATED. DO NOT EDIT.
# BASED ON c:\Users\dovla.DESKTOP-S1IGPJL\Documents\STM32ECU\ECU

function(add_st_target_properties TARGET_NAME)

target_compile_definitions(
    ${TARGET_NAME} PRIVATE
    "$<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:USE_HAL_DRIVER>"
    "$<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:STM32H743xx>"
    "$<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:STM32F10X_MD>"
    "$<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:USE_STDPERIPH_DRIVER>"
)

target_include_directories(
    ${TARGET_NAME} PRIVATE
    "$<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:${PROJECT_SOURCE_DIR}/Inc\\states>"
    "$<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:${PROJECT_SOURCE_DIR}/Inc\\devices>"
    "$<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:${PROJECT_SOURCE_DIR}/Inc\\tasks>"
    "$<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:${PROJECT_SOURCE_DIR}/Inc>"
    "$<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:${PROJECT_SOURCE_DIR}/Src\\Vectoring\\TV_TCS_C_code_for implementation\\Generated_Code\\SubsystemModelReference_ert_rtw>"
    "$<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:${PROJECT_SOURCE_DIR}/Src\\SOC\\GhettoSOC_ert_rtw>"
    "$<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:${PROJECT_SOURCE_DIR}/Src\\regen\\RegenCS_ert_rtw>"
)

target_compile_options(
    ${TARGET_NAME} PRIVATE
    "$<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:ASM>>:-g3>"
    "$<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:-g3>"
    "$<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-g3>"
    "$<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:-Ofast>"
    "$<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:>"
    "$<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:>"
    "$<$<CONFIG:Debug>:-mcpu=cortex-m7>"
    "$<$<CONFIG:Debug>:-mfpu=fpv5-d16>"
    "$<$<CONFIG:Debug>:-mfloat-abi=hard>"
)

target_link_libraries(
    ${TARGET_NAME} PRIVATE
)

target_link_directories(
    ${TARGET_NAME} PRIVATE
)

target_link_options(
    ${TARGET_NAME} PRIVATE
    "$<$<CONFIG:Debug>:-mcpu=cortex-m7>"
    "$<$<CONFIG:Debug>:-mfpu=fpv5-d16>"
    "$<$<CONFIG:Debug>:-mfloat-abi=hard>"
    -T
    "$<$<CONFIG:Debug>:${PROJECT_SOURCE_DIR}/STM32H743ZI_FLASH.ld>"
)

target_sources(
    ${TARGET_NAME} PRIVATE
    "/Users/visa/Code/STM32ECU/Delphi Stuff"
    "Core\\Src\\adc.c"
    "Core\\Src\\comp.c"
    "Core\\Src\\dma.c"
    "Core\\Src\\fdcan.c"
    "Core\\Src\\freertos.c"
    "Core\\Src\\gpio.c"
    "Core\\Src\\i2c.c"
    "Core\\Src\\main.c"
    "Core\\Src\\rng.c"
    "Core\\Src\\stm32h7xx_hal_msp.c"
    "Core\\Src\\stm32h7xx_hal_timebase_tim.c"
    "Core\\Src\\stm32h7xx_it.c"
    "Core\\Src\\syscalls.c"
    "Core\\Src\\system_stm32h7xx.c"
    "Core\\Src\\tim.c"
    "Core\\Src\\usart.c"
    "Core\\Src\\wwdg.c"
    "Drivers\\STM32H7xx_HAL_Driver\\Src\\stm32h7xx_hal_adc_ex.c"
    "Drivers\\STM32H7xx_HAL_Driver\\Src\\stm32h7xx_hal_adc.c"
    "Drivers\\STM32H7xx_HAL_Driver\\Src\\stm32h7xx_hal_comp.c"
    "Drivers\\STM32H7xx_HAL_Driver\\Src\\stm32h7xx_hal_cortex.c"
    "Drivers\\STM32H7xx_HAL_Driver\\Src\\stm32h7xx_hal_dma_ex.c"
    "Drivers\\STM32H7xx_HAL_Driver\\Src\\stm32h7xx_hal_dma.c"
    "Drivers\\STM32H7xx_HAL_Driver\\Src\\stm32h7xx_hal_exti.c"
    "Drivers\\STM32H7xx_HAL_Driver\\Src\\stm32h7xx_hal_fdcan.c"
    "Drivers\\STM32H7xx_HAL_Driver\\Src\\stm32h7xx_hal_flash_ex.c"
    "Drivers\\STM32H7xx_HAL_Driver\\Src\\stm32h7xx_hal_flash.c"
    "Drivers\\STM32H7xx_HAL_Driver\\Src\\stm32h7xx_hal_gpio.c"
    "Drivers\\STM32H7xx_HAL_Driver\\Src\\stm32h7xx_hal_hsem.c"
    "Drivers\\STM32H7xx_HAL_Driver\\Src\\stm32h7xx_hal_i2c_ex.c"
    "Drivers\\STM32H7xx_HAL_Driver\\Src\\stm32h7xx_hal_i2c.c"
    "Drivers\\STM32H7xx_HAL_Driver\\Src\\stm32h7xx_hal_mdma.c"
    "Drivers\\STM32H7xx_HAL_Driver\\Src\\stm32h7xx_hal_pwr_ex.c"
    "Drivers\\STM32H7xx_HAL_Driver\\Src\\stm32h7xx_hal_pwr.c"
    "Drivers\\STM32H7xx_HAL_Driver\\Src\\stm32h7xx_hal_rcc_ex.c"
    "Drivers\\STM32H7xx_HAL_Driver\\Src\\stm32h7xx_hal_rcc.c"
    "Drivers\\STM32H7xx_HAL_Driver\\Src\\stm32h7xx_hal_rng_ex.c"
    "Drivers\\STM32H7xx_HAL_Driver\\Src\\stm32h7xx_hal_rng.c"
    "Drivers\\STM32H7xx_HAL_Driver\\Src\\stm32h7xx_hal_tim_ex.c"
    "Drivers\\STM32H7xx_HAL_Driver\\Src\\stm32h7xx_hal_tim.c"
    "Drivers\\STM32H7xx_HAL_Driver\\Src\\stm32h7xx_hal_uart_ex.c"
    "Drivers\\STM32H7xx_HAL_Driver\\Src\\stm32h7xx_hal_uart.c"
    "Drivers\\STM32H7xx_HAL_Driver\\Src\\stm32h7xx_hal_wwdg.c"
    "Drivers\\STM32H7xx_HAL_Driver\\Src\\stm32h7xx_hal.c"
    "Middlewares\\Third_Party\\FreeRTOS\\Source\\CMSIS_RTOS_V2\\cmsis_os2.c"
    "Middlewares\\Third_Party\\FreeRTOS\\Source\\croutine.c"
    "Middlewares\\Third_Party\\FreeRTOS\\Source\\event_groups.c"
    "Middlewares\\Third_Party\\FreeRTOS\\Source\\list.c"
    "Middlewares\\Third_Party\\FreeRTOS\\Source\\portable\\GCC\\ARM_CM4F\\port.c"
    "Middlewares\\Third_Party\\FreeRTOS\\Source\\portable\\MemMang\\heap_1.c"
    "Middlewares\\Third_Party\\FreeRTOS\\Source\\queue.c"
    "Middlewares\\Third_Party\\FreeRTOS\\Source\\stream_buffer.c"
    "Middlewares\\Third_Party\\FreeRTOS\\Source\\tasks.c"
    "Middlewares\\Third_Party\\FreeRTOS\\Source\\timers.c"
    "Src\\devices\\analognode.c"
    "Src\\devices\\bms.c"
    "Src\\devices\\brake.c"
    "Src\\devices\\dwt_delay.c"
    "Src\\devices\\freertosstats.c"
    "Src\\devices\\imu.c"
    "Src\\devices\\interrupts.c"
    "Src\\devices\\ivt.c"
    "Src\\devices\\lenzeinverter.c"
    "Src\\devices\\memorator.c"
    "Src\\devices\\node.c"
    "Src\\devices\\pdm.c"
    "Src\\devices\\powerloss.c"
    "Src\\devices\\powernode.c"
    "Src\\devices\\timerecu.c"
    "Src\\devices\\torquecontrol.c"
    "Src\\devices\\uartecu.c"
    "Src\\ecu.c"
    "Src\\ecumain.c"
    "Src\\matlab\\Regeneration_ert_rtw\\Regeneration_data.c"
    "Src\\matlab\\Regeneration_ert_rtw\\Regeneration.c"
    "Src\\matlab\\TorqueVectoring_ert_rtw\\TorqueVectoring_data.c"
    "Src\\matlab\\TorqueVectoring_ert_rtw\\TorqueVectoring.c"
    "Src\\matlab\\TractionControl_ert_rtw\\TractionControl_data.c"
    "Src\\matlab\\TractionControl_ert_rtw\\TractionControl.c"
    "Src\\states\\errors.c"
    "Src\\states\\idleprocess.c"
    "Src\\states\\operationalprocess.c"
    "Src\\states\\operationalreadyness.c"
    "Src\\states\\preoperation.c"
    "Src\\states\\runningprocess.c"
    "Src\\states\\tsactiveprocess.c"
    "Src\\tasks\\adcecu.c"
    "Src\\tasks\\canecu.c"
    "Src\\tasks\\configuration.c"
    "Src\\tasks\\debug.c"
    "Src\\tasks\\eeprom.c"
    "Src\\tasks\\input.c"
    "Src\\tasks\\inverter.c"
    "Src\\tasks\\output.c"
    "Src\\tasks\\power.c"
    "Src\\tasks\\watchdog.c"
    "startup\\startup_stm32h743xx.s"
    "syscalls.c"
)

add_custom_command(
    TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${TARGET_NAME}>
)

add_custom_command(
    TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex
    $<TARGET_FILE:${TARGET_NAME}> ${TARGET_NAME}.hex
)

add_custom_command(
    TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary
    $<TARGET_FILE:${TARGET_NAME}> ${TARGET_NAME}.bin
)

endfunction()