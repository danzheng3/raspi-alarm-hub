add_library(devices
    digitalPot.cpp
    rtcModule.cpp
    sdCard.cpp
    strobe.cpp
)

target_include_directories(devices PUBLIC ${PROJECT_SOURCE_DIR}/include)