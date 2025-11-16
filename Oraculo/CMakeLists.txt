# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(RTC C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(RTC)

target_sources(RTC
    PRIVATE
        main.c
        components/ds3231/ds3231.c 
        components/pico_logs/pico_logs.c
        components/wifi/wifi.c
        components/mqtt/mqtt_lib.c
        components/ntp/ntp.c
        components/ble/ble.c
        components/st7735/st7735.c
        components/st7735/hw.c
)

pico_set_program_name(RTC "RTC")
pico_set_program_version(RTC "0.1")




# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(RTC 1)
pico_enable_stdio_usb(RTC 1)

# Add the standard library to the build
target_link_libraries(RTC
        pico_stdlib)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

# Add the standard include files to the build
target_include_directories(RTC PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/components/ds3231
        ${CMAKE_CURRENT_LIST_DIR}/components/pico_logs
        ${CMAKE_CURRENT_LIST_DIR}/components/wifi
        ${CMAKE_CURRENT_LIST_DIR}/components/mqtt
        ${CMAKE_CURRENT_LIST_DIR}/components/ntp
        ${CMAKE_CURRENT_LIST_DIR}/components/ble
        ${CMAKE_CURRENT_LIST_DIR}/components/st7735
)

# Add any user requested libraries
target_link_libraries(RTC 
        hardware_i2c
        hardware_dma
        hardware_spi
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip_mqtt
        pico_mbedtls
        pico_lwip_mbedtls
        pico_lwip_http
        pico_btstack_ble
        pico_btstack_cyw43
        )

# Deve colocar esse comando para conseguir 
# compilar com a definição de cliente BLE
target_compile_definitions(RTC PRIVATE
    RUNNING_AS_CLIENT=1
)

pico_add_extra_outputs(RTC)

