/**
 * @file organize_by_id.h
 * @brief This file defines the OrganizeById class, which is responsible for organizing
 * log messages based on their indentifiers.
 * 
 */

#ifndef COMPONENTS_LOG_MESSAGE_ORGANIZER_PUBLIC_LOG_MESSAGE_ORGANIZER_ORGANIZE_BY_ID_H_
#define COMPONENTS_LOG_MESSAGE_ORGANIZER_PUBLIC_LOG_MESSAGE_ORGANIZER_ORGANIZE_BY_ID_H_

/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "log_message_organizer/pipeline_log_message.h"

/******************************************************************************
 * CLASSES 
 *****************************************************************************/

namespace pipelines::log_message_organizer {
/**
 * @class OrganizeById
 * @brief This class is responsible for organizing log messages based on their identifiers.
 *
 * It provides a method to organize a collection of log messages by their IDs.
 */
class OrganizeById {
 public:
  OrganizeById() = delete; /**< Default constructor is deleted. */

  /**
     * @brief Constructor to initialize the OrganizeById class with log messages.
     * @param log_messages The collection of log messages to be organized.
     */
  OrganizeById(const PipelineLogMessages& log_messages)
      : log_messages_(log_messages) {}

  /**
     * @brief Organizes the log messages by their IDs.
     * @return A collection of organized log messages.
     */
  PipelineLogMessages Organize() const;

 private:
  PipelineLogMessages
      log_messages_; /**< Collection of log messages to be organized. */
};

}  // namespace pipelines::log_message_organizer
#endif  // COMPONENTS_LOG_MESSAGE_ORGANIZER_PUBLIC_LOG_MESSAGE_ORGANIZER_ORGANIZE_BY_ID_H_