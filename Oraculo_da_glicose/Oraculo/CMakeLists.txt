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

project(Oraculo C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(Oraculo)

target_sources(Oraculo PUBLIC
main.c
lib/BLE/BLE.c
lib/Mqtt/Mqtt.c
)


pico_set_program_name(Oraculo "Oraculo")
pico_set_program_version(Oraculo "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Oraculo 1)
pico_enable_stdio_usb(Oraculo 1)

# Pull in FreeRTOS
include(lib/FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/lib/BLE)

# Add the standard library to the build
target_link_libraries(Oraculo
    FreeRTOS-Kernel 
    FreeRTOS-Kernel-Heap4
    pico_stdlib
    pico_btstack_ble
    pico_btstack_cyw43
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mqtt
    pico_mbedtls
    pico_lwip_mbedtls
    pico_lwip_http
    hardware_adc
        )

# Add the standard include files to the build
target_include_directories(Oraculo PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/.. # For our common btstack config
    lib/BLE/
    lib/Mqtt/
)

# Add any user requested libraries
# target_link_libraries(Oraculo 
        
#         )

target_compile_definitions(Oraculo PRIVATE
    RUNNING_AS_CLIENT=1
)

pico_add_extra_outputs(Oraculo)

