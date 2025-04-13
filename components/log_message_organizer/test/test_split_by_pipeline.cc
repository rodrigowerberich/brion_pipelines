#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "log_message/message.h"
#include "log_message_organizer/split_by_pipeline.h"

using ::testing::Eq;

class SplitByPipelineTest : public ::testing::Test {
  // Setup and teardown methods can be added here if needed
};

TEST_F(SplitByPipelineTest, EmptyInput) {
  using pipelines::log_message_organizer::LogMessages;
  using pipelines::log_message_organizer::PipelineLogMessagesByPipeline;
  using pipelines::log_message_organizer::SplitByPipeline;

  auto input = LogMessages{};
  auto expected_output = PipelineLogMessagesByPipeline{};

  auto splitter = SplitByPipeline{input};
  ASSERT_THAT(splitter.Split(), Eq(expected_output));
}

TEST_F(SplitByPipelineTest, SinglePipeline) {
  using pipelines::log_message_organizer::LogMessages;
  using pipelines::log_message_organizer::PipelineLogMessagesByPipeline;
  using pipelines::log_message_organizer::SplitByPipeline;

  auto input = LogMessages{
      {"pipeline1", "1", "Hello, World!", "2"},
      {"pipeline1", "2", "Goodbye, World!", "-1"},
  };
  auto expected_output = PipelineLogMessagesByPipeline{
      {"pipeline1",
       {{"1", "Hello, World!", "2"}, {"2", "Goodbye, World!", "-1"}}},
  };

  auto splitter = SplitByPipeline{input};
  ASSERT_THAT(splitter.Split(), Eq(expected_output));
}

TEST_F(SplitByPipelineTest, MultiplePipelines) {
  using pipelines::log_message_organizer::LogMessages;
  using pipelines::log_message_organizer::PipelineLogMessagesByPipeline;
  using pipelines::log_message_organizer::SplitByPipeline;

  auto input = LogMessages{
      {"pipeline1", "1", "Hello, World!", "2"},
      {"pipeline1", "2", "Goodbye, World!", "-1"},
      {"pipeline2", "3", "Hello, Universe!", "4"},
      {"pipeline2", "4", "Goodbye, Universe!", "-1"},
  };
  auto expected_output = PipelineLogMessagesByPipeline{
      {"pipeline1",
       {{"1", "Hello, World!", "2"}, {"2", "Goodbye, World!", "-1"}}},
      {"pipeline2",
       {{"3", "Hello, Universe!", "4"}, {"4", "Goodbye, Universe!", "-1"}}},
  };

  auto splitter = SplitByPipeline{input};
  ASSERT_THAT(splitter.Split(), Eq(expected_output));
}
