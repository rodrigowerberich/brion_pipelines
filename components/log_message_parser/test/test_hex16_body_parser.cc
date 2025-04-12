#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "log_message_parser/hex16_body_parser.h"

using ::testing::Eq;

class Hex16BodyParserTest : public ::testing::Test {
 protected:
  pipelines::log_message_parser::semantics::Hex16BodyParser parser;
};

TEST_F(Hex16BodyParserTest, EmptyString) {
  std::string input = "";
  std::string expected_output = "";
  ASSERT_THAT(parser.Parse(input), Eq(expected_output));
}

TEST_F(Hex16BodyParserTest, ValidHexadecimalString) {
  std::string input = "626F6479";
  std::string expected_output = "body";

  ASSERT_THAT(parser.Parse(input), Eq(expected_output));
}

TEST_F(Hex16BodyParserTest, ValidHexadecimalStringWithSpaces) {
  std::string input = "4F 4B";
  std::string expected_output = "OK";

  ASSERT_THAT(parser.Parse(input), Eq(expected_output));
}

TEST_F(Hex16BodyParserTest, ValidHexadecimalStringWithNewlines) {
  std::string input = "4f\n4B";
  std::string expected_output = "OK";

  ASSERT_THAT(parser.Parse(input), Eq(expected_output));
}

TEST_F(Hex16BodyParserTest, BigText) {
  std::string input =
      "566976616d75732072757472756d2069642065726174206e6563207665686963756c612e"
      "20446f6e6563206672696e67696c6c61206c6163696e696120656c656966656e\n642e";
  std::string expected_output =
      "Vivamus rutrum id erat nec vehicula. Donec fringilla lacinia eleifend.";

  ASSERT_THAT(parser.Parse(input), Eq(expected_output));
}

TEST_F(Hex16BodyParserTest, OddLengthHexadecimalString) {
  std::string input = "4F4B1";
  ASSERT_THROW(parser.Parse(input),
               pipelines::log_message_parser::semantics::BodyParserError);
}

TEST_F(Hex16BodyParserTest, NonHexadecimalCharacters) {
  std::string input = "4G4B";
  ASSERT_THROW(parser.Parse(input),
               pipelines::log_message_parser::semantics::BodyParserError);
}
