# Test websocket executable configuration

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

# Set output directory
set_target_properties(test_websocket PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)