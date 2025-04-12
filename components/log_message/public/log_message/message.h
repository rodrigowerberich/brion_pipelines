/** 
 * @file message.h
 * Description: This file defines the basic Message class, which represents a log message
 * in a pipeline. And will be reused in the application. Other components should use this 
 * common reprentation of a log message.
 * 
 * @brief Base Log Message class
 */

#ifndef COMPONENT_LOG_MESSAGE_PUBLIC_LOG_MESSAGE_MESSAGE_H_
#define COMPONENT_LOG_MESSAGE_PUBLIC_LOG_MESSAGE_MESSAGE_H_

/******************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <ostream>
#include <string>

/******************************************************************************
 * CLASSES 
 *****************************************************************************/

namespace pipelines::log_message {

/**
 * @brief Message class representing a log message in a pipeline.
 * 
 * This class encapsulates the details of a log message, including its pipeline ID,
 * message ID, body, and next ID. 
 * 
 * The encoding is not included in this class, as it is this version of the message
 * should already contain the decoded body.
 * 
 * It provides some utility methods e.g. comparison and output formatting.
 */
class Message {
 public:
  /**
   * @brief Default constructor is deleted to prevent instantiation without parameters.
   */
  Message() = delete;

  /**
   * @brief Constructor to initialize a Message object with the given parameters.
   * 
   * @param pipeline_id The ID of the pipeline.
   * @param id The ID of the message.
   * @param body The body of the message.
   * @param next_id The ID of the next message.
   */
  Message(const std::string& pipeline_id, const std::string& id,
          const std::string& body, const std::string& next_id)
      : pipeline_id_(pipeline_id), id_(id), body_(body), next_id_(next_id) {}

  /**
   * @brief Get the pipeline ID of the message.
   * 
   * @return The pipeline ID.
   */
  const std::string& pipeline_id() const { return pipeline_id_; }
  /**
   * @brief Get the ID of the message.
   * 
   * @return The message ID.
   */
  const std::string& id() const { return id_; }
  /**
   * @brief Get the body of the message.
   * 
   * @return The message body.
   */
  const std::string& body() const { return body_; }
  /**
   * @brief Get the ID of the next message.
   * 
   * @return The next message ID.
   */
  const std::string& next_id() const { return next_id_; }

  /**
   * @brief Comparison operator to check if two Message objects are equal.
   * 
   * @param other The other Message object to compare with.
   * @return True if the two messages are equal, false otherwise.
   */
  bool operator==(const Message& other) const {
    return (pipeline_id_ == other.pipeline_id_) && (id_ == other.id_) &&
           (body_ == other.body_) && (next_id_ == other.next_id_);
  }

  /**
   * @brief Output operator to format the Message object as a string.
   * 
   * @param os The output stream.
   * @param message The Message object to format.
   * @return The output stream with the formatted message.
   */
  friend std::ostream& operator<<(std::ostream& os, const Message& message) {
    os << "(Pipeline ID: \"" << message.pipeline_id_ << "\", "
       << "ID: \"" << message.id_ << "\", "
       << "Body: \"" << message.body_ << "\", "
       << "Next ID: \"" << message.next_id_ << "\")";
    return os;
  }

 private:
  /**
   * @brief The ID of the pipeline.
   */
  std::string pipeline_id_;
  /**
   * @brief The ID of the message.
   */
  std::string id_;
  /**
   * @brief The body of the message.
   */
  std::string body_;
  /**
   * @brief The ID of the next message.
   */
  std::string next_id_;
};

}  // namespace pipelines::log_message

#endif  // COMPONENT_LOG_MESSAGE_PUBLIC_LOG_MESSAGE_MESSAGE_H_