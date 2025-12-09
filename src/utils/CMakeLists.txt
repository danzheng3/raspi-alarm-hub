add_library(utils
    MainPage.cpp
    MusicPage.cpp
    SettingsPage.cpp
    AlarmPage.cpp
)

target_include_directories(utils PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(utils PUBLIC ${PROJECT_SOURCE_DIR}/include/utils)
target_include_directories(utils PUBLIC ${PROJECT_SOURCE_DIR}/include/managers)

target_link_libraries(utils PUBLIC managers)
