add_library(I_log_message_parser INTERFACE)
target_include_directories(I_log_message_parser INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/public
)

add_library(log_message_parser STATIC
    private/structure.cc
)

target_include_directories(log_message_parser PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/private
)

target_link_libraries(log_message_parser
    I_log_message_parser
)

add_subdirectory(test)