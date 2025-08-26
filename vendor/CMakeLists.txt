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

# Add compile flags for custom implementations
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMBEDTLS_TIMING_ALT -DMBEDTLS_NO_PLATFORM_ENTROPY -I${CMAKE_CURRENT_SOURCE_DIR}/mbedtls_patch/include/mbedtls")

# Add mbedtls
add_subdirectory(mbedtls)

# After mbedtls is configured, add our custom timing source to mbedcrypto
if(TARGET mbedcrypto)
    # Add our custom timing source file directly to the library
    target_sources(mbedcrypto PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/mbedtls_patch/src/timing_alt.c
    )
    
    # Add include directory for our custom headers
    target_include_directories(mbedcrypto PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/mbedtls_patch/include
    )
    
    message(STATUS "Added custom timing implementation to mbedcrypto")
endif()

# Export mbedtls targets for parent project
set(MBEDTLS_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/mbedtls/include PARENT_SCOPE)
set(MBEDTLS_LIBRARIES mbedtls mbedx509 mbedcrypto PARENT_SCOPE)

message(STATUS "Vendor libraries configured")