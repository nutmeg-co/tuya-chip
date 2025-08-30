cmake_minimum_required(VERSION 3.5.1)
project(tuya-chip-client C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add vendor libraries
add_subdirectory(vendor)

# Create executable
add_executable(tuya-client 
    src/main.c
    src/transport_tcp.c
    src/custom_rng.c
)

# Include directories
target_include_directories(tuya-client PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${MBEDTLS_INCLUDE_DIRS}
)

# Link against mbedtls libraries
target_link_libraries(tuya-client PRIVATE 
    ${MBEDTLS_LIBRARIES}
)

# Create test_websocket executable
add_executable(test_websocket
    src/test_websocket.c
)

# Include directories for test_websocket
target_include_directories(test_websocket PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${WEBSOCKET_PARSER_INCLUDE_DIRS}
)

# Link against websocket-parser library
target_link_libraries(test_websocket PRIVATE
    ${WEBSOCKET_PARSER_LIBRARIES}
)

# Platform-specific settings
if(WIN32)
    target_link_libraries(tuya-client PRIVATE ws2_32)
endif()

# Set output directory
set_target_properties(tuya-client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(test_websocket PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)