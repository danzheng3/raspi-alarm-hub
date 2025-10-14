add_library(managers 
    display_manager.cpp

)

target_include_directories(managers PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(managers PUBLIC ${PROJECT_SOURCE_DIR}/include/managers)
target_link_libraries(managers PUBLIC utils)