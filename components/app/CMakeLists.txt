add_executable(app
     private/app.cc
)

target_link_libraries(app
    I_log_message_organizer
    I_log_message
    I_log_message_parser
    log_message_organizer
    log_message_parser
    clipp
)

# add_subdirectory(test)