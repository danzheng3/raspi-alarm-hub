cmake_minimum_required(VERSION 3.10)

project(Raspi-Alarm-Hub CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_COMPILER /usr/bin/g++)

option(TEST_MODE "Build for desktop testing" OFF)

if(TEST_MODE)
    add_definitions(-DTEST_MODE)
    message(STATUS "Building in TEST MODE (Hardware Simulated)")
else()
    message(STATUS "Building in HARDWARE MODE (Raspi Native)")
endif()

include_directories(include)
add_subdirectory(src)