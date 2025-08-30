# Vendor libraries CMakeLists.txt
cmake_minimum_required(VERSION 3.5.1)

# Include mbedtls configuration
include(${CMAKE_CURRENT_SOURCE_DIR}/mbedtls.cmake)

# Include websocket-parser configuration
include(${CMAKE_CURRENT_SOURCE_DIR}/websocket-parser.cmake)

message(STATUS "Vendor libraries configured")