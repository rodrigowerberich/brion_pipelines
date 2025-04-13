/**
 * @file split_by_pipeline.h
 * @brief This file defines the SplitByPipeline class, which is responsible for
 * splitting log messages by their pipeline identifiers.
 * 
 */

#ifndef COMPONENTS_LOG_MESSAGE_ORGANIZER_PUBLIC_LOG_MESSAGE_ORGANIZER_SPLIT_BY_PIPELINE_H_
#define COMPONENTS_LOG_MESSAGE_ORGANIZER_PUBLIC_LOG_MESSAGE_ORGANIZER_SPLIT_BY_PIPELINE_H_

/******************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "log_message_organizer/pipeline_log_message.h"

/******************************************************************************
 * CLASSES 
 *****************************************************************************/

namespace pipelines::log_message_organizer {

/**
 * @class SplitByPipeline
 * @brief This class is responsible for splitting log messages by their pipeline identifiers.
 * 
 * It provides a method to split a collection of log messages into separate collections
 * based on their pipeline IDs.
 */
class SplitByPipeline {
 public:
  SplitByPipeline() = delete; /**< Default constructor is deleted. */

  /**
    * @brief Constructor to initialize the SplitByPipeline class with log messages.
    * @param log_messages The collection of log messages to be split.
    */
  explicit SplitByPipeline(const LogMessages& log_messages)
      : log_messages_(log_messages) {}

  /**
    * @brief Splits the log messages by their pipeline IDs.
    * @return A map of pipeline IDs to their corresponding log messages.
    */
  PipelineLogMessagesByPipeline Split() const;

 private:
  LogMessages log_messages_; /**< Collection of log messages to be split. */
};

}  // namespace pipelines::log_message_organizer

#endif  // COMPONENTS_LOG_MESSAGE_ORGANIZER_PUBLIC_LOG_MESSAGE_ORGANIZER_SPLIT_BY_PIPELINE_H_