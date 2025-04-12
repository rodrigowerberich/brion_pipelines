

# Tests for the structure parser
add_executable(test_structure_parser
    test_structure.cc
    ../private/structure.cc
)
target_link_libraries(test_structure_parser
    gtest_main
    gmock
    I_log_message_parser
)

# Tests for the semantics parser
add_executable(test_semantics_parser
    test_semantics.cc
    ../private/semantics.cc
)
target_link_libraries(test_semantics_parser
    gtest_main
    gmock
    I_log_message_parser
)

# Tests for the hex16 body parser
add_executable(test_hex16_body_parser
    test_hex16_body_parser.cc
    ../private/hex16_body_parser.cc
)
target_link_libraries(test_hex16_body_parser
    gtest_main
    gmock
    I_log_message_parser
)

# Tests for the ascii body parser
add_executable(test_ascii_body_parser
    test_ascii_body_parser.cc
    ../private/ascii_body_parser.cc
)
target_link_libraries(test_ascii_body_parser
    gtest_main
    gmock
    I_log_message_parser
)