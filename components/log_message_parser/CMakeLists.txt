add_library(I_log_message_parser INTERFACE)
target_include_directories(I_log_message_parser INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/public
)

add_library(log_message_parser STATIC
    private/structure.cc
    private/semantics.cc
    private/hex16_body_parser.cc
    private/ascii_body_parser.cc
)

target_include_directories(log_message_parser PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/private
)

target_link_libraries(log_message_parser
    I_log_message_parser
    I_log_message
)

add_subdirectory(test)