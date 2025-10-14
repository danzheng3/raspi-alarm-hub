add_library(utils
    MainPage.cpp
)

target_include_directories(utils PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(utils PUBLIC ${PROJECT_SOURCE_DIR}/include/utils)