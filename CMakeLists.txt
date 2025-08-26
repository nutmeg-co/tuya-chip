cmake_minimum_required(VERSION 3.5.1)
project(tuya-chip-client C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add mbedtls as a subdirectory
add_subdirectory(mbedtls)

# Create executable
add_executable(tuya-client 
    src/main.c
    src/transport_tcp.c
    src/custom_rng.c
)

# Include directories
target_include_directories(tuya-client PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/mbedtls/include
)

# Link against mbedtls libraries
target_link_libraries(tuya-client PRIVATE 
    mbedtls
    mbedx509
    mbedcrypto
)

# Platform-specific settings
if(WIN32)
    target_link_libraries(tuya-client PRIVATE ws2_32)
endif()

# Set output directory
set_target_properties(tuya-client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)