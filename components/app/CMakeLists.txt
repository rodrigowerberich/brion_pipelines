add_executable(app
    PUBLIC
     private/app.cc
)

target_link_libraries(app
    I_log_message_organizer
    I_log_message
    I_log_message_parser
    log_message_organizer
    log_message_parser
)

# add_subdirectory(test)