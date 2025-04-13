/**
 * @file split_by_pipeline.cc
 * @brief Implementation of the SplitByPipeline class.
 * 
 * This file contains the implementation of the SplitByPipeline class, which is responsible
 * for splitting log messages by their pipeline identifiers.
 */

/******************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "log_message_organizer/split_by_pipeline.h"

/******************************************************************************
 * PRIVATE HELPER DECLARATIONS
 *****************************************************************************/

/******************************************************************************
 * PRIVATE CLASS METHODS IMPLEMENTATION
 *****************************************************************************/

/******************************************************************************
  * PUBLIC CLASS METHODS IMPLEMENTATION
  * *****************************************************************************/

namespace pipelines::log_message_organizer {

PipelineLogMessagesByPipeline SplitByPipeline::Split() const {
  auto messages_by_pipeline = PipelineLogMessagesByPipeline{};
  for (const auto& message : log_messages_) {
    const auto& pipeline_id = message.pipeline_id();
    auto& messages = messages_by_pipeline[pipeline_id];
    messages.emplace_back(message.id(), message.body(), message.next_id());
  }
  return messages_by_pipeline;
}

}  // namespace pipelines::log_message_organizer