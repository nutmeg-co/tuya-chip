# WebSocket Parser library configuration
cmake_minimum_required(VERSION 3.5.1)

message(STATUS "Configuring websocket-parser...")

# Create websocket-parser library
add_library(websocket-parser STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/websocket-parser/websocket_parser.c
)

# Set include directories for the library
target_include_directories(websocket-parser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/websocket-parser
)

# Set compile options if needed
target_compile_options(websocket-parser PRIVATE
    -Wall
    -Wextra
)

# Export variables for parent project
set(WEBSOCKET_PARSER_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/websocket-parser PARENT_SCOPE)
set(WEBSOCKET_PARSER_LIBRARIES websocket-parser PARENT_SCOPE)

message(STATUS "WebSocket Parser library configured")