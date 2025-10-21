add_library(devices
    MCP44X1.cpp
    MCP3021.cpp
    MCP7940N.cpp
)

target_include_directories(devices PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(devices PUBLIC utils)