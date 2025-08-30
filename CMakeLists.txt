cmake_minimum_required(VERSION 3.5.1)
project(tuya-chip-client C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add vendor libraries
add_subdirectory(vendor)

# Include tuya-client configuration
include(${CMAKE_CURRENT_SOURCE_DIR}/tuya-client.cmake)

# Include test-websocket configuration
include(${CMAKE_CURRENT_SOURCE_DIR}/test-websocket.cmake)