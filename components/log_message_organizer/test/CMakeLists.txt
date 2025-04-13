

# Tests for the organizer by id
add_executable(test_organize_by_id
    test_organize_by_id.cc
    ../private/organize_by_id.cc
)
target_link_libraries(test_organize_by_id
    gtest_main
    gmock
    I_log_message_organizer
    I_log_message
)
gtest_discover_tests(test_organize_by_id)

# Tests for the split by pipeline
add_executable(test_split_by_pipeline
    test_split_by_pipeline.cc
    ../private/split_by_pipeline.cc
)
target_link_libraries(test_split_by_pipeline
    gtest_main
    gmock
    I_log_message_organizer
    I_log_message
)
gtest_discover_tests(test_split_by_pipeline)