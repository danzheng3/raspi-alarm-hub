add_library(managers 
    display_manager.cpp
    alarmManager.cpp
    storageManager.cpp
    timeManager.cpp
    connectivityManager.cpp
    audioManager.cpp
    equalizerManager.cpp
    weatherManager.cpp
    powerManager.cpp
)

find_package(CURL REQUIRED)

target_include_directories(managers PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(managers PUBLIC ${PROJECT_SOURCE_DIR}/include/managers)
target_link_libraries(managers PUBLIC utils CURL::libcurl)