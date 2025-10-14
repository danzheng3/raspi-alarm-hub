add_library(bus 
    I2CBus.cpp
    SPIBus.cpp
    GPIO.cpp
)

target_include_directories(bus PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(bus PUBLIC ${PROJECT_SOURCE_DIR}/include/hardware_layer)