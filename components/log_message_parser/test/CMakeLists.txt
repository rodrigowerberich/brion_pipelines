
add_executable(test_structure_parser
    test_structure.cc
    ../private/structure.cc
)

target_link_libraries(test_structure_parser
    gtest_main
    gmock
    I_log_message_parser
)