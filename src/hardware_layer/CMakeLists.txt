add_library(hardware_layer
    I2CBus.cpp
    SPIBus.cpp
    GPIO.cpp
    BluetoothAdapter.cpp
    WifiAdapter.cpp
)

find_library(GPIOD_LIB gpiod)


target_include_directories(hardware_layer PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(hardware_layer PUBLIC ${PROJECT_SOURCE_DIR}/include/hardware_layer)
target_link_libraries(hardware_layer PUBLIC utils gpiod)