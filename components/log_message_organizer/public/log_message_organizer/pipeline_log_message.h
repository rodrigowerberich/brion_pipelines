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

/**
 * @brief Type alias for a log message with a pipeline ID.
 */
using LogMessage = pipelines::log_message::Message;
/** 
 * @brief Type alias for a collection of log messages with their pipeline IDs.
 */
using LogMessages = std::vector<LogMessage>;
/** 
 * @brief Type alias for a collection of log messages that belong to a pipeline.
 */
using PipelineLogMessages = std::vector<class PipelineLogMessage>;
/**
 * @brief Type alias for a collection of log messages organized by pipeline ID.
 * 
 * This is a map where the key is the pipeline ID and the value is a vector of
 * PipelineLogMessages.
 */
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

  /**
   * @brief Getter for the ID of the log message.
   * 
   * @return The ID of the log message.
   */
  const std::string& id() const { return id_; }
  /**
   * @brief Getter for the body of the log message.
   * 
   * @return The body of the log message.
   * @note The body is expected to be a decoded string.
   */
  const std::string& body() const { return body_; }
  /**
   * @brief Getter for the ID of the next log message.
   * 
   * @return The ID of the next log message.
   */
  const std::string& next_id() const { return next_id_; }

  /**
   * @brief Equality operator to compare two Message objects.
   * 
   * @param other The other Message object to compare with.
   * @return True if the two messages are equal, false otherwise.
   */
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

  /**
   * @brief Output operator to format the PipelineLogMessage object as a string.
   * 
   * @param os The output stream.
   * @param message The PipelineLogMessage object to format.
   * @return The output stream with the formatted message.
   */
  friend std::ostream& operator<<(std::ostream& os,
                                  const PipelineLogMessage& message) {
    os << "(ID: \"" << message.id_ << "\", "
       << "Body: \"" << message.body_ << "\", "
       << "Next ID: \"" << message.next_id_ << "\")";
    return os;
  }

 private:
  std::string id_;      /**< The ID of the log message. */
  std::string body_;    /**< The body of the log message. */
  std::string next_id_; /**< The ID of the next log message. */
};

}  // namespace pipelines::log_message_organizer

#endif  // COMPONENTS_LOG_MESSAGE_ORGANIZER_PUBLIC_LOG_MESSAGE_ORGANIZER_PIPELINE_LOG_MESSAGE_H_