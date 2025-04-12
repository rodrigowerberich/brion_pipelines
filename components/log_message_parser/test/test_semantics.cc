#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "log_message_parser/semantics.h"

using ::testing::Eq;
using ::testing::HasSubstr;

namespace pipelines::log_message_parser::semantics::test {

class MockBodyParser : public BodyParser {
 public:
  MOCK_METHOD(std::string, Parse, (const std::string&), (const override));
};

}  // namespace pipelines::log_message_parser::semantics::test

class SemanticsParserTest : public ::testing::Test {
  // Setup and teardown methods can be added here if needed
};

TEST_F(SemanticsParserTest, EmptyInput) {
  using pipelines::log_message_parser::semantics::Parser;
  using pipelines::log_message_parser::structure::LogMessages;

  auto input = LogMessages{};

  auto parser = Parser{input};
  auto parse_result = parser.Parse();

  ASSERT_THAT(parse_result.HasErrors(), Eq(false));
  ASSERT_THAT(parse_result.messages().size(), Eq(0));
  ASSERT_THAT(parse_result.errors().size(), Eq(0));
}

TEST_F(SemanticsParserTest, NoBodyParserRegistered) {
  using pipelines::log_message_parser::semantics::Parser;
  using StructureLogMessages =
      pipelines::log_message_parser::structure::LogMessages;

  auto input = StructureLogMessages{{"1", "2", "3", "4F4B", "-1"}};

  auto parser = Parser{input};
  auto parse_result = parser.Parse();

  ASSERT_THAT(parse_result.HasErrors(), Eq(true));
  ASSERT_THAT(parse_result.messages().size(), Eq(0));
  ASSERT_THAT(parse_result.errors().size(), Eq(1));
  ASSERT_THAT(parse_result.errors()[0].message(),
              HasSubstr("Encoding \"3\" is not supported for log message"));
}

TEST_F(SemanticsParserTest, BodyParserThrowsError) {
  using pipelines::log_message_parser::semantics::BodyParserError;
  using pipelines::log_message_parser::semantics::Parser;
  using pipelines::log_message_parser::semantics::test::MockBodyParser;
  using StructureLogMessages =
      pipelines::log_message_parser::structure::LogMessages;

  auto input = StructureLogMessages{{"1", "2", "3", "4F4B", "-1"}};

  auto mock_body_parser = std::make_unique<MockBodyParser>();
  auto* mock_body_parser_ptr = mock_body_parser.get();

  EXPECT_CALL(*mock_body_parser_ptr, Parse("4F4B"))
      .WillOnce(testing::Throw(BodyParserError("Parsing error")));

  auto parser = Parser{input};
  parser.RegisterBodyParser("3", std::move(mock_body_parser));
  auto parse_result = parser.Parse();

  ASSERT_THAT(parse_result.HasErrors(), Eq(true));
  ASSERT_THAT(parse_result.messages().size(), Eq(0));
  ASSERT_THAT(parse_result.errors().size(), Eq(1));
  ASSERT_THAT(parse_result.errors()[0].message(),
              HasSubstr("Failed to parse body for log message"));
}

TEST_F(SemanticsParserTest, BodyParserWorksCorrectly) {
  using pipelines::log_message_parser::semantics::BodyParserError;
  using pipelines::log_message_parser::semantics::Parser;
  using pipelines::log_message_parser::semantics::test::MockBodyParser;
  using StructureLogMessages =
      pipelines::log_message_parser::structure::LogMessages;
  using SemanticLogMessage =
      pipelines::log_message_parser::semantics::LogMessage;

  auto input = StructureLogMessages{{"1", "2", "3", "4F4B", "-1"}};

  auto mock_body_parser = std::make_unique<MockBodyParser>();
  auto* mock_body_parser_ptr = mock_body_parser.get();

  EXPECT_CALL(*mock_body_parser_ptr, Parse("4F4B"))
      .WillOnce(testing::Return("Parsed body"));

  auto parser = Parser{input};
  parser.RegisterBodyParser("3", std::move(mock_body_parser));
  auto parse_result = parser.Parse();

  ASSERT_THAT(parse_result.HasErrors(), Eq(false));
  ASSERT_THAT(parse_result.messages().size(), Eq(1));

  ASSERT_THAT(parse_result.messages()[0],
              Eq(SemanticLogMessage{"1", "2", "Parsed body", "-1"}));
  ASSERT_THAT(parse_result.errors().size(), Eq(0));
}

TEST_F(SemanticsParserTest, MultipleMessages) {
  using pipelines::log_message_parser::semantics::BodyParserError;
  using pipelines::log_message_parser::semantics::Parser;
  using pipelines::log_message_parser::semantics::test::MockBodyParser;
  using StructureLogMessages =
      pipelines::log_message_parser::structure::LogMessages;
  using SemanticLogMessage =
      pipelines::log_message_parser::semantics::LogMessage;

  auto input = StructureLogMessages{
      {"1", "2", "3", "4F4B", "-1"},
      {"5", "6", "3", "8F8B", "-2"},
  };

  auto mock_body_parser = std::make_unique<MockBodyParser>();
  auto* mock_body_parser_ptr = mock_body_parser.get();

  EXPECT_CALL(*mock_body_parser_ptr, Parse("4F4B"))
      .WillOnce(testing::Return("Parsed body 1"));
  EXPECT_CALL(*mock_body_parser_ptr, Parse("8F8B"))
      .WillOnce(testing::Return("Parsed body 2"));

  auto parser = Parser{input};
  parser.RegisterBodyParser("3", std::move(mock_body_parser));
  auto parse_result = parser.Parse();

  ASSERT_THAT(parse_result.HasErrors(), Eq(false));
  ASSERT_THAT(parse_result.messages().size(), Eq(2));

  ASSERT_THAT(parse_result.messages()[0],
              Eq(SemanticLogMessage{"1", "2", "Parsed body 1", "-1"}));
  ASSERT_THAT(parse_result.messages()[1],
              Eq(SemanticLogMessage{"5", "6", "Parsed body 2", "-2"}));
}

TEST_F(SemanticsParserTest, MultipleMessagesWithErrors) {
  using pipelines::log_message_parser::semantics::BodyParserError;
  using pipelines::log_message_parser::semantics::Parser;
  using pipelines::log_message_parser::semantics::test::MockBodyParser;
  using StructureLogMessages =
      pipelines::log_message_parser::structure::LogMessages;
  using SemanticLogMessage =
      pipelines::log_message_parser::semantics::LogMessage;

  auto input = StructureLogMessages{
      {"1", "2", "3", "4F4B", "-1"},
      {"5", "6", "7", "8F8B", "-2"},
  };

  auto mock_body_parser = std::make_unique<MockBodyParser>();
  auto* mock_body_parser_ptr = mock_body_parser.get();

  EXPECT_CALL(*mock_body_parser_ptr, Parse("4F4B"))
      .WillOnce(testing::Return("Parsed body"));
  EXPECT_CALL(*mock_body_parser_ptr, Parse("8F8B")).Times(0);

  auto parser = Parser{input};
  parser.RegisterBodyParser("3", std::move(mock_body_parser));
  auto parse_result = parser.Parse();

  ASSERT_THAT(parse_result.HasErrors(), Eq(true));
  ASSERT_THAT(parse_result.messages().size(), Eq(1));
  ASSERT_THAT(parse_result.errors().size(), Eq(1));

  ASSERT_THAT(parse_result.messages()[0],
              Eq(SemanticLogMessage{"1", "2", "Parsed body", "-1"}));

  ASSERT_THAT(parse_result.errors()[0].message(),
              HasSubstr("Encoding \"7\" is not supported for log message"));
}