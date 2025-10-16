add_subdirectory(utils)
add_subdirectory(managers)
add_subdirectory(hardware_layer)
add_subdirectory(devices)

set(SDL2_DIR "/usr/lib/x86_64-linux-gnu/cmake/SDL2")
set(SDL2_ttf_DIR "/usr/lib/x86_64-linux-gnu/cmake/SDL2_ttf")
set(SDL2_image_DIR "/usr/lib/x86_64-linux-gnu/cmake/SDL2_image")

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_image REQUIRED)

add_executable(Raspi-Alarm-Hub main.cpp)
target_link_libraries(Raspi-Alarm-Hub 
    PRIVATE
        utils 
        managers
        hardware_layer
        SDL2::SDL2
        SDL2_ttf::SDL2_ttf
        SDL2_image::SDL2_image 
)

target_include_directories(Raspi-Alarm-Hub PUBLIC ${PROJECT_SOURCE_DIR}/include)


