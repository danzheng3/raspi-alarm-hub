cmake_minimum_required(VERSION 3.10)

project(Raspi-Alarm-Hub CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_COMPILER /usr/bin/g++)

set(SDL2_DIR "/usr/lib/x86_64-linux-gnu/cmake/SDL2")
set(SDL2_ttf_DIR "/usr/lib/x86_64-linux-gnu/cmake/SDL2_ttf")
set(SDL2_image_DIR "/usr/lib/x86_64-linux-gnu/cmake/SDL2_image")

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_image REQUIRED)

include_directories(include)
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_ttf_INCLUDE_DIRS} ${SDL2_image_INCLUDE_DIRS})
file(GLOB_RECURSE SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)
add_executable(${PROJECT_NAME} ${SRC_FILES})


target_link_libraries(Raspi-Alarm-Hub ${SDL2_LIBRARIES} SDL2_ttf::SDL2_ttf SDL2_image::SDL2_image)
message("SDL2_ttf_LIBRARIES: ${SDL_ttf_LIBRARIES}")
