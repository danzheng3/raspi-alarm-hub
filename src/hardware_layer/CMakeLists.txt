add_library(hardware_layer
    I2CBus.cpp
    SPIBus.cpp
    GPIO.cpp
    BluetoothAdapter.cpp
    WifiAdapter.cpp
)

target_include_directories(hardware_layer PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(hardware_layer PUBLIC ${PROJECT_SOURCE_DIR}/include/hardware_layer)