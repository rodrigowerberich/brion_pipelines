#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "log_message_parser/ascii_body_parser.h"

using ::testing::Eq;

class AsciiBodyParserTest : public ::testing::Test {
 protected:
  pipelines::log_message_parser::semantics::AsciiBodyParser parser;
};

TEST_F(AsciiBodyParserTest, EmptyString) {
  std::string input = "";
  std::string expected_output = "";
  ASSERT_THAT(parser.parse(input), Eq(expected_output));
}

TEST_F(AsciiBodyParserTest, ValidAsciiString) {
  std::string input = "Hello, World!";
  std::string expected_output = "Hello, World!";
  ASSERT_THAT(parser.parse(input), Eq(expected_output));
}

TEST_F(AsciiBodyParserTest, StringWithSpaces) {
  std::string input = "Hello,   World!";
  std::string expected_output = "Hello,   World!";
  ASSERT_THAT(parser.parse(input), Eq(expected_output));
}

TEST_F(AsciiBodyParserTest, StringWithNewlines) {
  std::string input = "Hello,\nWorld!";
  std::string expected_output = "Hello,\nWorld!";
  ASSERT_THAT(parser.parse(input), Eq(expected_output));
}

TEST_F(AsciiBodyParserTest, StringWithTabs) {
  std::string input = "Hello,\tWorld!";
  std::string expected_output = "Hello,\tWorld!";
  ASSERT_THAT(parser.parse(input), Eq(expected_output));
}
