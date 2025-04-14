add_executable(pipeline_parser
     private/app.cc
)

target_link_libraries(pipeline_parser
    I_log_message_organizer
    I_log_message
    I_log_message_parser
    log_message_organizer
    log_message_parser
    clipp
)

install(TARGETS pipeline_parser
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# add_subdirectory(test)