add_library(I_log_message_organizer INTERFACE)
target_include_directories(I_log_message_organizer INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/public
)

add_library(log_message_organizer STATIC
    private/organize_by_id.cc
    private/split_by_pipeline.cc
)

target_include_directories(log_message_organizer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/private
)

target_link_libraries(log_message_organizer
    I_log_message_organizer 
    I_log_message
)

add_subdirectory(test)