add_library(devices
    MCP44X1.cpp
    MCP3021.cpp
    MCP7940N.cpp
    strobe.cpp
    pillbox.cpp
    sdCard.cpp
    LED.cpp
)

target_include_directories(devices PUBLIC ${PROJECT_SOURCE_DIR}/include)

if(NOT TEST_MODE)
    find_library(PIGPIO_LIB pigpio)
    target_link_libraries(devices PUBLIC utils hardware_layer ${PIGPIO_LIB})
else()
    target_link_libraries(devices PUBLIC utils hardware_layer)
endif()