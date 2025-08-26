# Vendor libraries CMakeLists.txt
cmake_minimum_required(VERSION 3.5.1)

# mbedtls configuration
message(STATUS "Configuring mbedtls...")

# Set mbedtls options before adding subdirectory
set(USE_SHARED_MBEDTLS_LIBRARY OFF CACHE BOOL "" FORCE)
set(USE_STATIC_MBEDTLS_LIBRARY ON CACHE BOOL "" FORCE)
set(ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(ENABLE_PROGRAMS OFF CACHE BOOL "" FORCE)
set(MBEDTLS_FATAL_WARNINGS OFF CACHE BOOL "" FORCE)

# Add mbedtls
add_subdirectory(mbedtls)

# Export mbedtls targets for parent project
set(MBEDTLS_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/mbedtls/include PARENT_SCOPE)
set(MBEDTLS_LIBRARIES mbedtls mbedx509 mbedcrypto PARENT_SCOPE)

message(STATUS "Vendor libraries configured")