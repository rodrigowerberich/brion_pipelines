/**
 * @file pipeline_log_message.h
 * @brief This file defines the PipelineLogMessage class, which represents a log message
 * that belongs to a singular pipeline.
 * 
 * It also has some helpful type definitions for log messages and collections of log messages.
 */
#ifndef COMPONENTS_LOG_MESSAGE_ORGANIZER_PUBLIC_LOG_MESSAGE_ORGANIZER_PIPELINE_LOG_MESSAGE_H_
#define COMPONENTS_LOG_MESSAGE_ORGANIZER_PUBLIC_LOG_MESSAGE_ORGANIZER_PIPELINE_LOG_MESSAGE_H_
#include <map>
#include <ostream>
#include <string>
#include <vector>
#include "log_message/message.h"

/******************************************************************************
 * TYPE DEFINITIONS
 *****************************************************************************/

namespace pipelines::log_message_organizer {

using LogMessage =
    pipelines::log_message::Message; /**< Type alias for log messages. */
using LogMessages = std::vector<LogMessage>; /**< Collection of log messages. */
using PipelineLogMessages = std::vector<class PipelineLogMessage>;
using PipelineLogMessagesByPipeline =
    std::map<std::string, PipelineLogMessages>;

}  // namespace pipelines::log_message_organizer

/******************************************************************************
 * CLASSES 
 *****************************************************************************/

namespace pipelines::log_message_organizer {

/**
 * @class PipelineLogMessage
 * @brief This class represents a log message that belongs to a singular pipeline.
 * 
 * It contains the message ID, body and the next ID of the message.
 */
class PipelineLogMessage {
 public:
  constexpr PipelineLogMessage() = default; /**< Default constructor. */

  /**
       * @brief Constructor to initialize a PipelineLogMessage with the given parameters.
       * @param id The ID of the log message.
       * @param body The body of the log message.
       * @param next_id The ID of the next log message.
       */
  constexpr PipelineLogMessage(const std::string& id, const std::string& body,
                               const std::string& next_id)
      : id_(id), body_(body), next_id_(next_id) {}

  // Getters for the private members
  const std::string& id() const { return id_; }
  const std::string& body() const { return body_; }
  const std::string& next_id() const { return next_id_; }

  bool operator==(const PipelineLogMessage& other) const {
    return (id_ == other.id_) && (body_ == other.body_) &&
           (next_id_ == other.next_id_);
  }

  /**
   * @brief Less than operator to compare two Message objects.
   * 
   * @param other The other Message object to compare with.
   * @return True if this message is less than the other, false otherwise.
   */
  bool operator<(const PipelineLogMessage& other) const {
    return std::tie(id_, next_id_, body_) <
           std::tie(other.id_, other.next_id_, other.body_);
  }

  friend std::ostream& operator<<(std::ostream& os,
                                  const PipelineLogMessage& message) {
    // os << "(ID: \"" << message.id_ << "\", "
    //    << "Body: \"" << message.body_ << "\", "
    //    << "Next ID: \"" << message.next_id_ << "\")";
    os << "( \"" << message.id_ << "\", "
       << "\"" << message.next_id_ << "\")";
    return os;
  }

 private:
  std::string id_;      /**< The ID of the log message. */
  std::string body_;    /**< The body of the log message. */
  std::string next_id_; /**< The ID of the next log message. */
};

}  // namespace pipelines::log_message_organizer

#endif  // COMPONENTS_LOG_MESSAGE_ORGANIZER_PUBLIC_LOG_MESSAGE_ORGANIZER_PIPELINE_LOG_MESSAGE_H_