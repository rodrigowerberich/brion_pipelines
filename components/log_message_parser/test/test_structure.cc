#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "log_message_parser/structure.h"

using ::testing::Eq;
using ::testing::HasSubstr;

class LogMessageParserTest : public ::testing::Test {
  // Setup and teardown methods can be added here if needed
};

TEST_F(LogMessageParserTest, EmptyInput) {
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(false));
  ASSERT_THAT(result.size(), Eq(0));
}

TEST_F(LogMessageParserTest, WhitespaceOnly) {
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("   \n");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(false));
  ASSERT_THAT(result.size(), Eq(0));
}

TEST_F(LogMessageParserTest, MissingId) {
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("This \n");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(true));
  ASSERT_THAT(parse_result.get_errors().size(), Eq(1));
  ASSERT_THAT(parse_result.get_errors()[0].get_message(),
              HasSubstr("File ended while parsing"));
  ASSERT_THAT(parse_result.get_errors()[0].get_line_number(), Eq(1));
  ASSERT_THAT(result.size(), Eq(0));
}

TEST_F(LogMessageParserTest, MissingEncoding) {
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("This is \n");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(true));
  ASSERT_THAT(parse_result.get_errors().size(), Eq(1));
  ASSERT_THAT(parse_result.get_errors()[0].get_message(),
              HasSubstr("File ended while parsing"));
  ASSERT_THAT(parse_result.get_errors()[0].get_line_number(), Eq(1));
  ASSERT_THAT(result.size(), Eq(0));
}

TEST_F(LogMessageParserTest, MissingBody) {
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("This is a \n");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(true));
  ASSERT_THAT(parse_result.get_errors().size(), Eq(1));
  ASSERT_THAT(parse_result.get_errors()[0].get_message(),
              HasSubstr("File ended while parsing"));
  ASSERT_THAT(parse_result.get_errors()[0].get_line_number(), Eq(1));
  ASSERT_THAT(result.size(), Eq(0));
}

TEST_F(LogMessageParserTest, MissingBodyNoBracket) {
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("This is a test\n");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(true));
  ASSERT_THAT(parse_result.get_errors().size(), Eq(2));

  ASSERT_THAT(parse_result.get_errors()[0].get_message(),
              HasSubstr("Expected an opening bracket"));
  ASSERT_THAT(parse_result.get_errors()[0].get_line_number(), Eq(1));

  std::cout << parse_result.get_errors()[1].get_message() << std::endl;

  ASSERT_THAT(parse_result.get_errors()[1].get_message(),
              HasSubstr("There is unparsed data in line "));
  ASSERT_THAT(parse_result.get_errors()[1].get_line_number(), Eq(1));

  ASSERT_THAT(result.size(), Eq(0));
}

TEST_F(LogMessageParserTest, MissingBodyTwoManyOpeningBrackets) {
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("This is a [tes[t] [test]\n");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(true));
  ASSERT_THAT(parse_result.get_errors().size(), Eq(1));
  ASSERT_THAT(parse_result.get_errors()[0].get_message(),
              HasSubstr("Expected a closing bracket"));
  ASSERT_THAT(parse_result.get_errors()[0].get_line_number(), Eq(1));
  ASSERT_THAT(result.size(), Eq(0));
}

TEST_F(LogMessageParserTest, MissingBodyNonNestedBrackets1) {
  using pipelines::log_message_parser::structure::LogMessage;
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("This is a [test] [test]\n");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(false));
  ASSERT_THAT(result.size(), Eq(1));
  ASSERT_THAT(result[0], Eq(LogMessage{"This", "is", "a", "test", "[test]"}));
}

TEST_F(LogMessageParserTest, MissingBodyNonNestedBrackets2) {
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("This is a [test] [test] [test]\n");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(true));
  ASSERT_THAT(parse_result.get_errors().size(), Eq(1));
  ASSERT_THAT(parse_result.get_errors()[0].get_message(),
              HasSubstr("There is unparsed data in line"));
}

TEST_F(LogMessageParserTest, MissingNextId) {
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("This is a [test]\n");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(true));
  ASSERT_THAT(parse_result.get_errors().size(), Eq(1));
  ASSERT_THAT(parse_result.get_errors()[0].get_message(),
              HasSubstr("File ended while parsing"));
  ASSERT_THAT(parse_result.get_errors()[0].get_line_number(), Eq(1));
  ASSERT_THAT(result.size(), Eq(0));
}

TEST_F(LogMessageParserTest, SingleLineInput) {
  using pipelines::log_message_parser::structure::LogMessage;
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("1 2 3 [4F4B] -1");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(false));
  ASSERT_THAT(result.size(), Eq(1));
  ASSERT_THAT(result[0], Eq(LogMessage{"1", "2", "3", "4F4B", "-1"}));
}

TEST_F(LogMessageParserTest, SingleLineInputWithTabs) {
  using pipelines::log_message_parser::structure::LogMessage;
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("1\t2\t3\t[4F4B]\t-1\n");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(false));
  ASSERT_THAT(result.size(), Eq(1));
  ASSERT_THAT(result[0], Eq(LogMessage{"1", "2", "3", "4F4B", "-1"}));
}

TEST_F(LogMessageParserTest, SingleLineInputWithMultipleSpaces) {
  using pipelines::log_message_parser::structure::LogMessage;
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("1    2    3    [4F4B]    -1\n");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(false));
  ASSERT_THAT(result.size(), Eq(1));
  ASSERT_THAT(result[0], Eq(LogMessage{"1", "2", "3", "4F4B", "-1"}));
}

TEST_F(LogMessageParserTest, SingleLineInputWithMixedSpacesAndTabs) {
  using pipelines::log_message_parser::structure::LogMessage;
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("1\t2    3\t[4F4B] -1\n");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(false));
  ASSERT_THAT(result.size(), Eq(1));
  ASSERT_THAT(result[0], Eq(LogMessage{"1", "2", "3", "4F4B", "-1"}));
}

TEST_F(LogMessageParserTest, SingleLineInputWithTrailingSpaces) {
  using pipelines::log_message_parser::structure::LogMessage;
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("1 2 3 [4F4B] -1   \n");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(false));

  ASSERT_THAT(result.size(), Eq(1));
  ASSERT_THAT(result[0], Eq(LogMessage{"1", "2", "3", "4F4B", "-1"}));
}

TEST_F(LogMessageParserTest, SingleLineInputWithLeadingSpaces) {
  using pipelines::log_message_parser::structure::LogMessage;
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("   1 2 3 [4F4B] -1\n");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(false));
  ASSERT_THAT(result.size(), Eq(1));
  ASSERT_THAT(result[0], Eq(LogMessage{"1", "2", "3", "4F4B", "-1"}));
}

TEST_F(LogMessageParserTest, SingleLineInputWithMultipleNewlines) {
  using pipelines::log_message_parser::structure::LogMessage;
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("1 2 3 [4F4B] -1\n\n");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(false));
  ASSERT_THAT(result.size(), Eq(1));
  ASSERT_THAT(result[0], Eq(LogMessage{"1", "2", "3", "4F4B", "-1"}));
}

TEST_F(LogMessageParserTest, SingleLineInputWithCarriageReturn) {
  using pipelines::log_message_parser::structure::LogMessage;
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("1 2 3 [4F4B] -1\r");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(false));
  ASSERT_THAT(result.size(), Eq(1));
  ASSERT_THAT(result[0], Eq(LogMessage{"1", "2", "3", "4F4B", "-1"}));
}

TEST_F(LogMessageParserTest, SingleLineInputWithCarriageReturnAndNewline) {
  using pipelines::log_message_parser::structure::LogMessage;
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("1 2 3 [4F4B] -1\r\n");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(false));
  ASSERT_THAT(result.size(), Eq(1));
  ASSERT_THAT(result[0], Eq(LogMessage{"1", "2", "3", "4F4B", "-1"}));
}

TEST_F(LogMessageParserTest, SingleLineInputWithMultipleCarriageReturns) {
  using pipelines::log_message_parser::structure::LogMessage;
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("1 2 3 [4F4B] -1\r\r");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(false));
  ASSERT_THAT(result.size(), Eq(1));
  ASSERT_THAT(result[0], Eq(LogMessage{"1", "2", "3", "4F4B", "-1"}));
}

TEST_F(LogMessageParserTest, SingleLineInputWithCarriageReturnAndSpaces) {
  using pipelines::log_message_parser::structure::LogMessage;
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("1 2 3 [4F4B] -1\r   ");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(false));
  ASSERT_THAT(result.size(), Eq(1));
  ASSERT_THAT(result[0], Eq(LogMessage{"1", "2", "3", "4F4B", "-1"}));
}

TEST_F(LogMessageParserTest, SingleLineInputWithCarriageReturnAndTabs) {
  using pipelines::log_message_parser::structure::LogMessage;
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("1 2 3 [4F4B] -1\r\t");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(false));
  ASSERT_THAT(result.size(), Eq(1));
  ASSERT_THAT(result[0], Eq(LogMessage{"1", "2", "3", "4F4B", "-1"}));
}

TEST_F(LogMessageParserTest, SingleLineInputWithBodyWithSpaces) {
  using pipelines::log_message_parser::structure::LogMessage;
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("1 2 3 [a test message] -1\n");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(false));
  ASSERT_THAT(result.size(), Eq(1));
  ASSERT_THAT(result[0], Eq(LogMessage{"1", "2", "3", "a test message", "-1"}));
}

TEST_F(LogMessageParserTest, SingleLineInputWithBodyWithTabs) {
  using pipelines::log_message_parser::structure::LogMessage;
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("1 2 3 [a\ttest\tmessage] -1\n");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(false));
  ASSERT_THAT(result.size(), Eq(1));
  ASSERT_THAT(result[0],
              Eq(LogMessage{"1", "2", "3", "a\ttest\tmessage", "-1"}));
}

TEST_F(LogMessageParserTest, SingleLineInputWithBodyWithNewlines) {
  using pipelines::log_message_parser::structure::LogMessage;
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("1 2 3 [a\ntest\nmessage] -1\n");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(false));
  ASSERT_THAT(result.size(), Eq(1));
  ASSERT_THAT(result[0],
              Eq(LogMessage{"1", "2", "3", "a\ntest\nmessage", "-1"}));
}

TEST_F(LogMessageParserTest, SingleLineInputWithBodyWithCarriageReturns) {
  using pipelines::log_message_parser::structure::LogMessage;
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("1 2 3 [a\ntest\rmessage] -1\n");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(false));
  ASSERT_THAT(result.size(), Eq(1));
  ASSERT_THAT(result[0],
              Eq(LogMessage{"1", "2", "3", "a\ntest\rmessage", "-1"}));
}

TEST_F(LogMessageParserTest, SingleLineInputWithBodyWithMixedSpacesAndTabs) {
  using pipelines::log_message_parser::structure::LogMessage;
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("1 2 3 [a test\tmessage] -1\n");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(false));
  ASSERT_THAT(result.size(), Eq(1));
  ASSERT_THAT(result[0],
              Eq(LogMessage{"1", "2", "3", "a test\tmessage", "-1"}));
}

TEST_F(LogMessageParserTest,
       SingleLineInputWithBodyWithMixedSpacesAndNewlines) {
  using pipelines::log_message_parser::structure::LogMessage;
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("1 2 3 [a test\nmessage] -1\n");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(false));
  ASSERT_THAT(result.size(), Eq(1));
  ASSERT_THAT(result[0],
              Eq(LogMessage{"1", "2", "3", "a test\nmessage", "-1"}));
}

TEST_F(LogMessageParserTest, SingleLineInputWithBodyWithMultipleBrackets) {
  using pipelines::log_message_parser::structure::LogMessage;
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("1 2 3 [a test [message]] -1\n");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(false));
  ASSERT_THAT(result.size(), Eq(1));
  ASSERT_THAT(result[0],
              Eq(LogMessage{"1", "2", "3", "a test [message]", "-1"}));
}

TEST_F(LogMessageParserTest,
       SingleLineInputWithBodyWithMultipleBracketsAndNewlines) {
  using pipelines::log_message_parser::structure::LogMessage;
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input("1 2 3 [a test\n[message]] -1\n");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(false));
  ASSERT_THAT(result.size(), Eq(1));
  ASSERT_THAT(result[0],
              Eq(LogMessage{"1", "2", "3", "a test\n[message]", "-1"}));
}

TEST_F(LogMessageParserTest, MultipleLinesInput) {
  using pipelines::log_message_parser::structure::LogMessage;
  using pipelines::log_message_parser::structure::Parser;

  std::istringstream input(
      "2 3 1 [4F4B] -1\n"
      "1 0 0 [some text] 1\n"
      "1 1 0 [another text] 2\n"
      "2 99 1 [4F4B] 3\n"
      "1 2 1 [626F6479] -1\n");

  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(false));
  ASSERT_THAT(result.size(), Eq(5));

  ASSERT_THAT(result[0], Eq(LogMessage{"2", "3", "1", "4F4B", "-1"}));
  ASSERT_THAT(result[1], Eq(LogMessage{"1", "0", "0", "some text", "1"}));
  ASSERT_THAT(result[2], Eq(LogMessage{"1", "1", "0", "another text", "2"}));
  ASSERT_THAT(result[3], Eq(LogMessage{"2", "99", "1", "4F4B", "3"}));
  ASSERT_THAT(result[4], Eq(LogMessage{"1", "2", "1", "626F6479", "-1"}));
}

TEST_F(LogMessageParserTest, ActualLog1WithoutBreaklines) {
  using pipelines::log_message_parser::structure::LogMessage;
  using pipelines::log_message_parser::structure::Parser;

  auto expected_result = std::vector<LogMessage>{
      {"legacy-hex", "2", "1",
       "4d6f726269206c6f626f72746973206d6178696d757320766976657272612e20416c697"
       "175616d2065742068656e647265726974206e756c6c61",
       "-1"},
      {"2", "12", "0",
       "nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt "
       "in culpa qui officia deserunt mollit anim id est laborum.",
       "1"},
      {"1", "0", "0", "Lorem ipsum dolor sit amet, consectetur adipiscing elit",
       "-1"},
      {"2", "10", "0",
       "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris "
       "nisi ut aliquip ex ea",
       "30"},
      {"3", "1", "0",
       "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua",
       "-1"},
      {"2", "30", "0",
       "commodo consequat. duis aute irure dolor in reprehenderit in voluptate "
       "velit esse cillum dolore eu fugiat",
       "12"},
      {"legacy-hex", "1", "1",
       "566976616d75732072757472756d2069642065726174206e6563207665686963756c612"
       "e20446f6e6563206672696e67696c6c61206c6163696e696120656c656966656e642e",
       "2"}};

  auto input = std::istringstream(
      R"(legacy-hex 2 1 [4d6f726269206c6f626f72746973206d6178696d757320766976657272612e20416c697175616d2065742068656e647265726974206e756c6c61] -1 
2 12 0 [nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.] 1 
1 0 0 [Lorem ipsum dolor sit amet, consectetur adipiscing elit] -1 
2 10 0 [Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea] 30 
3 1 0 [sed do eiusmod tempor incididunt ut labore et dolore magna aliqua] -1 
2 30 0 [commodo consequat. duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat] 12 
legacy-hex 1 1 [566976616d75732072757472756d2069642065726174206e6563207665686963756c612e20446f6e6563206672696e67696c6c61206c6163696e696120656c656966656e642e] 2)");

  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(false));
  ASSERT_THAT(result.size(), Eq(expected_result.size()));

  for (size_t i = 0; i < expected_result.size(); ++i) {
    ASSERT_THAT(result[i], Eq(expected_result[i]));
  }
}

TEST_F(LogMessageParserTest, ActualLog1WithBreaklines) {
  using pipelines::log_message_parser::structure::LogMessage;
  using pipelines::log_message_parser::structure::Parser;

  auto expected_result = std::vector<LogMessage>{
      {"legacy-hex", "2", "1",
       "4d6f726269206c6f626f72746973206d6178696d757320766976657272612e20416c697"
       "175616d2065742068656e647265726974206e756c6c61",
       "-1"},
      {"2", "12", "0",
       "nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt "
       "in culpa qui officia deserunt mollit anim id est laborum.",
       "1"},
      {"1", "0", "0", "Lorem ipsum dolor sit amet, consectetur adipiscing elit",
       "-1"},
      {"2", "10", "0",
       "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris "
       "nisi ut aliquip ex ea",
       "30"},
      {"3", "1", "0",
       "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua",
       "-1"},
      {"2", "30", "0",
       "commodo consequat. duis aute irure dolor in reprehenderit in voluptate "
       "velit esse cillum dolore eu fugiat",
       "12"},
      {"legacy-hex", "1", "1",
       "566976616d75732072757472756d2069642065726174206e6563207665686963756c612"
       "e20446f6e6563206672696e67696c6c61206c6163696e696120656c656966656e\n642"
       "e",
       "2"}};

  auto input = std::istringstream(
      R"(legacy-hex 2 1 [4d6f726269206c6f626f72746973206d6178696d757320766976657272612e20416c697175616d2065742068656e647265726974206e756c6c61] -1
2 12 0 [nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.]
1
1 0 0 [Lorem ipsum dolor sit amet, consectetur adipiscing elit] -1
2 10 0 [Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea] 30
3 1 0 [sed do eiusmod tempor incididunt ut labore et dolore magna aliqua] -1
2 30 0 [commodo consequat. duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat] 12
legacy-hex 1 1
[566976616d75732072757472756d2069642065726174206e6563207665686963756c612e20446f6e6563206672696e67696c6c61206c6163696e696120656c656966656e
642e] 2)");

  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(false));
  ASSERT_THAT(result.size(), Eq(expected_result.size()));

  for (size_t i = 0; i < expected_result.size(); ++i) {
    ASSERT_THAT(result[i], Eq(expected_result[i]));
  }
}

TEST_F(LogMessageParserTest, ActualLog2WithoutBreaklines) {
  using pipelines::log_message_parser::structure::LogMessage;
  using pipelines::log_message_parser::structure::Parser;

  auto expected_result = std::vector<LogMessage>{
      {"legacy-hex", "legacy-2", "1",
       "4d6f726269206c6f626f72746973206d6178696d757320766976657272612e20416c697"
       "175616d2065742068656e647265726974206e756c6c61",
       "-1"},
      {"2", "12", "0",
       "nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt "
       "in culpa qui officia deserunt mollit anim id est laborum.",
       "1"},
      {"1", "0", "0", "Lorem ipsum dolor sit amet, consectetur adipiscing elit",
       "-1"},
      {"2", "10", "0",
       "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris "
       "nisi ut aliquip ex ea",
       "30"},
      {"3", "1", "0",
       "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua",
       "-1"},
      {"2", "30", "0",
       "commodo consequat. duis aute irure dolor in reprehenderit in voluptate "
       "velit esse cillum dolore eu fugiat",
       "12"},
      {"legacy-hex", "legacy-1", "1",
       "566976616d75732072757472756d2069642065726174206e6563207665686963756c612"
       "e20446f6e6563206672696e67696c6c61206c6163696e696120656c656966656e642e",
       "legacy-2"}};

  auto input = std::istringstream(
      R"(legacy-hex legacy-2 1 [4d6f726269206c6f626f72746973206d6178696d757320766976657272612e20416c697175616d2065742068656e647265726974206e756c6c61] -1 
2 12 0 [nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.] 1 
1 0 0 [Lorem ipsum dolor sit amet, consectetur adipiscing elit] -1 
2 10 0 [Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea] 30 
3 1 0 [sed do eiusmod tempor incididunt ut labore et dolore magna aliqua] -1 
2 30 0 [commodo consequat. duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat] 12 
legacy-hex legacy-1 1 [566976616d75732072757472756d2069642065726174206e6563207665686963756c612e20446f6e6563206672696e67696c6c61206c6163696e696120656c656966656e642e] legacy-2 
111 99 0 [......)");

  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(true));
  ASSERT_THAT(result.size(), Eq(expected_result.size()));

  // Check for errors
  ASSERT_THAT(parse_result.get_errors().size(), Eq(1));
  ASSERT_THAT(
      parse_result.get_errors()[0].get_message(),
      HasSubstr("File ended while parsing: Expected a closing bracket"));
  ASSERT_THAT(parse_result.get_errors()[0].get_line_number(), Eq(8));

  // Check for messages
  for (size_t i = 0; i < expected_result.size(); ++i) {
    ASSERT_THAT(result[i], Eq(expected_result[i]));
  }
}

TEST_F(LogMessageParserTest, ActualLog2WithBreaklines) {
  using pipelines::log_message_parser::structure::LogMessage;
  using pipelines::log_message_parser::structure::Parser;
  auto expected_result = std::vector<LogMessage>{
      {"legacy-hex", "legacy-2", "1",
       "4d6f726269206c6f626f72746973206d6178696d757320766976657272612e20416c697"
       "175616d2065742068656e647265726974206e756c6c61",
       "-1"},
      {"2", "12", "0",
       "nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt "
       "in culpa qui officia deserunt mollit anim id est laborum.",
       "1"},
      {"1", "0", "0", "Lorem ipsum dolor sit amet, consectetur adipiscing elit",
       "-1"},
      {"2", "10", "0",
       "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris "
       "nisi ut aliquip ex ea",
       "30"},
      {"3", "1", "0",
       "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua",
       "-1"},
      {"2", "30", "0",
       "commodo consequat. duis aute irure dolor in reprehenderit in voluptate "
       "velit esse cillum dolore eu fugiat",
       "12"},
      {"legacy-hex", "legacy-1", "1",
       "566976616d75732072757472756d2069642065726174206e6563207665686963756c612"
       "e20446f6e6563206672696e67696c6c61206c6163696e696120656c656966656e\n642"
       "e",
       "legacy-2"}};

  auto input = std::istringstream(
      R"(
legacy-hex legacy-2 1
[4d6f726269206c6f626f72746973206d6178696d757320766976657272612e20416c697175616d2065742068656e647265726974206e756c6c61] -1
2 12 0 [nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.]
1
1 0 0 [Lorem ipsum dolor sit amet, consectetur adipiscing elit] -1
2 10 0 [Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea] 30
3 1 0 [sed do eiusmod tempor incididunt ut labore et dolore magna aliqua] -1
2 30 0 [commodo consequat. duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat] 12
legacy-hex legacy-1 1
[566976616d75732072757472756d2069642065726174206e6563207665686963756c612e20446f6e6563206672696e67696c6c61206c6163696e696120656c656966656e
642e] legacy-2
111 99 0 [......)");
  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(true));
  ASSERT_THAT(result.size(), Eq(expected_result.size()));

  // Check for errors
  ASSERT_THAT(parse_result.get_errors().size(), Eq(1));
  ASSERT_THAT(
      parse_result.get_errors()[0].get_message(),
      HasSubstr("File ended while parsing: Expected a closing bracket"));
  ASSERT_THAT(parse_result.get_errors()[0].get_line_number(), Eq(13));

  // Check for messages
  for (size_t i = 0; i < expected_result.size(); ++i) {
    ASSERT_THAT(result[i], Eq(expected_result[i]));
  }
}

TEST_F(LogMessageParserTest, ActualLog3WithoutBreaklines) {
  using pipelines::log_message_parser::structure::LogMessage;
  using pipelines::log_message_parser::structure::Parser;

  auto expected_result = std::vector<LogMessage>{
      {"legacy-hex", "2", "1",
       "4d6f726269206c6f626f72746973206d6178696d757320766976657272612e20416c697"
       "175616d2065742068656e647265726974206e756c6c61",
       "-1"},
      {"2", "37620c47-da9b-4218-9c35-fdb5961d4239", "0",
       "nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt "
       "in culpa qui officia deserunt mollit anim id est laborum.",
       "-1"},
      {"1", "0", "0", "Lorem ipsum dolor sit amet, consectetur adipiscing elit",
       "-1"},
      {"2", "04e28d3b-d945-4051-8eeb-6f049f391234", "0",
       "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris "
       "nisi ut aliquip ex ea",
       "5352ab80-7b0a-421f-8ab4-5c840ae882ee"},
      {"3", "1", "0",
       "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua",
       "-1"},
      {"2", "5352ab80-7b0a-421f-8ab4-5c840ae882ee", "0",
       "commodo consequat. duis aute irure dolor in reprehenderit in voluptate "
       "velit esse cillum dolore eu fugiat",
       "37620c47-da9b-4218-9c35-fdb5961d4239"},
      {"legacy-hex", "1", "1",
       "566976616d75732072757472756d2069642065726174206e6563207665686963756c612"
       "e20446f6e6563206672696e67696c6c61206c6163696e696120656c656966656e642e",
       "2"}};

  auto input = std::istringstream(
      R"(legacy-hex 2 1 [4d6f726269206c6f626f72746973206d6178696d757320766976657272612e20416c697175616d2065742068656e647265726974206e756c6c61] -1 
2 37620c47-da9b-4218-9c35-fdb5961d4239 0 [nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.] -1 
1 0 0 [Lorem ipsum dolor sit amet, consectetur adipiscing elit] -1 
2 04e28d3b-d945-4051-8eeb-6f049f391234 0 [Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea] 5352ab80-7b0a-421f-8ab4-5c840ae882ee 
3 1 0 [sed do eiusmod tempor incididunt ut labore et dolore magna aliqua] -1 
2 5352ab80-7b0a-421f-8ab4-5c840ae882ee 0 [commodo consequat. duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat] 37620c47-da9b-4218-9c35-fdb5961d4239 
legacy-hex 1 1 [566976616d75732072757472756d2069642065726174206e6563207665686963756c612e20446f6e6563206672696e67696c6c61206c6163696e696120656c656966656e642e] 2 )");

  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(false));
  ASSERT_THAT(result.size(), Eq(expected_result.size()));

  for (size_t i = 0; i < expected_result.size(); ++i) {
    ASSERT_THAT(result[i], Eq(expected_result[i]));
  }
}

TEST_F(LogMessageParserTest, ActualLog3WithBreaklines) {
  using pipelines::log_message_parser::structure::LogMessage;
  using pipelines::log_message_parser::structure::Parser;

  auto expected_result = std::vector<LogMessage>{
      {"legacy-hex", "2", "1",
       "4d6f726269206c6f626f72746973206d6178696d757320766976657272612e20416c697"
       "175616d2065742068656e647265726974206e756c6c61",
       "-1"},
      {"2", "37620c47-da9b-4218-9c35-fdb5961d4239", "0",
       "nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt "
       "in culpa qui officia\ndeserunt mollit anim id est laborum.",
       "-1"},
      {"1", "0", "0", "Lorem ipsum dolor sit amet, consectetur adipiscing elit",
       "-1"},
      {"2", "04e28d3b-d945-4051-8eeb-6f049f391234", "0",
       "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris "
       "nisi ut aliquip ex ea",
       "5352ab80-7b0a-421f-8ab4-5c840ae882ee"},
      {"3", "1", "0",
       "sed do eiusmod tempor incididunt ut labore et dolore\nmagna aliqua",
       "-1"},
      {"2", "5352ab80-7b0a-421f-8ab4-5c840ae882ee", "0",
       "commodo consequat. duis aute irure dolor in reprehenderit in voluptate "
       "velit esse cillum dolore\nneu fugiat",
       "37620c47-da9b-4218-9c35-fdb5961d4239"},
      {"legacy-hex", "1", "1",
       "566976616d75732072757472756d2069642065726174206e6563207665686963756c612"
       "e20446f6e6563206672696e67696c6c61206c6163696e696120656c656966656e\n642"
       "e",
       "2"}};

  auto input = std::istringstream(
      R"(legacy-hex 2 1 [4d6f726269206c6f626f72746973206d6178696d757320766976657272612e20416c697175616d2065742068656e647265726974206e756c6c61] -1
2 37620c47-da9b-4218-9c35-fdb5961d4239 0 [nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia
deserunt mollit anim id est laborum.] -1
1 0 0 [Lorem ipsum dolor sit amet, consectetur adipiscing elit] -1
2 04e28d3b-d945-4051-8eeb-6f049f391234 0 [Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea]
5352ab80-7b0a-421f-8ab4-5c840ae882ee
3 1 0 [sed do eiusmod tempor incididunt ut labore et dolore magna aliqua] -1
2 5352ab80-7b0a-421f-8ab4-5c840ae882ee 0 [commodo consequat. duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore
eu fugiat] 37620c47-da9b-4218-9c35-fdb5961d4239
legacy-hex 1 1
[566976616d75732072757472756d2069642065726174206e6563207665686963756c612e20446f6e6563206672696e67696c6c61206c6163696e696120656c656966656e
642e] 2 )");

  auto parser = Parser{input};
  auto parse_result = parser.parse();
  auto result = parse_result.get_messages();

  ASSERT_THAT(parse_result.has_errors(), Eq(false));
  ASSERT_THAT(result.size(), Eq(7));

  ASSERT_THAT(result[0],
              Eq(LogMessage{
                  "legacy-hex", "2", "1",
                  "4d6f726269206c6f626f72746973206d6178696d75732076697665727261"
                  "2e20416c697175616d2065742068656e647265726974206e756c6c61",
                  "-1"}));
  ASSERT_THAT(
      result[1],
      Eq(LogMessage{
          "2", "37620c47-da9b-4218-9c35-fdb5961d4239", "0",
          "nulla pariatur. Excepteur sint occaecat cupidatat non proident, "
          "sunt in culpa qui officia\ndeserunt mollit anim id est laborum.",
          "-1"}));
  ASSERT_THAT(
      result[2],
      Eq(LogMessage{"1", "0", "0",
                    "Lorem ipsum dolor sit amet, consectetur adipiscing elit",
                    "-1"}));
  ASSERT_THAT(
      result[3],
      Eq(LogMessage{"2", "04e28d3b-d945-4051-8eeb-6f049f391234", "0",
                    "Ut enim ad minim veniam, quis nostrud exercitation "
                    "ullamco laboris nisi ut aliquip ex ea",
                    "5352ab80-7b0a-421f-8ab4-5c840ae882ee"}));
  ASSERT_THAT(
      result[4],
      Eq(LogMessage{
          "3", "1", "0",
          "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua",
          "-1"}));
  ASSERT_THAT(
      result[5],
      Eq(LogMessage{"2", "5352ab80-7b0a-421f-8ab4-5c840ae882ee", "0",
                    "commodo consequat. duis aute irure dolor in reprehenderit "
                    "in voluptate velit esse cillum dolore\neu fugiat",
                    "37620c47-da9b-4218-9c35-fdb5961d4239"}));
  ASSERT_THAT(result[6],
              Eq(LogMessage{"legacy-hex", "1", "1",
                            "566976616d75732072757472756d2069642065726174206"
                            "e6563207665686963756c612e20446f6e6563206672696e"
                            "67696c6c61206c6163696e696120656c656966656e\n642e",
                            "2"}));
}