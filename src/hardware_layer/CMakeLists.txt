add_subdirectory(bus devices)
add_subdirectory(devices)
add_library(hardware_layer
    bus
    devices
)

target_include_directories(hardware_layer PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(hardware_layer PUBLIC ${PROJECT_SOURCE_DIR}/include/hardware_layer)