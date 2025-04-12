/**
 * @file structure.h
 * @brief Parses a stream of data into a structured log message format.
 *
 * This file contains the declarations of the LogMessage, ParseError, and
 * ParseResult classes, as well as the Parser class for processing structured
 * log messages. These classes provide the foundation for handling log message
 * parsing and error reporting.
 */

#ifndef COMPONENT_LOG_MESSAGE_PARSER_PUBLIC_LOG_MESSAGE_PARSER_STRUCTURE_H_
#define COMPONENT_LOG_MESSAGE_PARSER_PUBLIC_LOG_MESSAGE_PARSER_STRUCTURE_H_

/******************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <istream>
#include <string>
#include <vector>

/******************************************************************************
 * TYPE DEFINITIONS
 *****************************************************************************/

namespace pipelines::log_message_parser::structure {

using LogMessages =
    std::vector<class LogMessage>; /**< Collection of log messages. */
using ParseErrors =
    std::vector<class ParseError>; /**< Collection of parsing errors. */

}  // namespace pipelines::log_message_parser::structure

/******************************************************************************
 * CLASSES
 *****************************************************************************/

namespace pipelines::log_message_parser::structure {

/**
 * @class LogMessage
 * @brief Represents a structured log message.
 *
 * This class encapsulates the details of a log message, including its pipeline ID,
 * message ID, encoding type, body content, and the ID of the next message in the sequence.
 */
class LogMessage {
 public:
  /**
   * @brief Constructs a LogMessage with the given parameters.
   * @param pipeline_id The ID of the pipeline.
   * @param id The ID of the log message.
   * @param encoding The encoding type of the log message body.
   * @param body The body content of the log message.
   * @param next_id The ID of the next log message in the sequence.
   */
  LogMessage(const std::string& pipeline_id, const std::string& id,
             const std::string& encoding, const std::string& body,
             const std::string& next_id)
      : pipeline_id_(pipeline_id),
        id_(id),
        encoding_(encoding),
        body_(body),
        next_id_(next_id) {}

  /**
   * @brief Retrieves the pipeline ID.
   * @return The pipeline ID as a string.
   */
  const std::string& pipeline_id() const { return pipeline_id_; }

  /**
   * @brief Retrieves the log message ID.
   * @return The log message ID as a string.
   */
  const std::string& id() const { return id_; }

  /**
   * @brief Retrieves the body content of the log message.
   * @return The body content as a string.
   */
  const std::string& body() const { return body_; }

  /**
   * @brief Retrieves the ID of the next log message in the sequence.
   * @return The next log message ID as a string.
   */
  const std::string& next_id() const { return next_id_; }

  /**
   * @brief Retrieves the encoding type of the log message body.
   * @return The encoding type as a string.
   */
  const std::string& encoding() const { return encoding_; }

  /**
   * @brief Compares two LogMessage objects for equality.
   * @param other The other LogMessage object to compare.
   * @return true if the two LogMessage objects are equal, false otherwise.
   */
  bool operator==(const LogMessage& other) const {
    return pipeline_id_ == other.pipeline_id_ && id_ == other.id_ &&
           encoding_ == other.encoding_ && body_ == other.body_ &&
           next_id_ == other.next_id_;
  }

  /**
   * @brief Outputs the LogMessage details to a stream.
   * @param os The output stream.
   * @param message The LogMessage object to output.
   * @return The output stream.
   */
  friend std::ostream& operator<<(std::ostream& os, const LogMessage& message) {
    os << "(Pipeline ID: \"" << message.pipeline_id_ << "\", "
       << "ID: \"" << message.id_ << "\", "
       << "Encoding: \"" << message.encoding_ << "\", "
       << "Body: \"" << message.body_ << "\", "
       << "Next ID: \"" << message.next_id_ << "\")";
    return os;
  }

 private:
  std::string pipeline_id_; /**< The ID of the pipeline. */
  std::string id_;          /**< The ID of the log message. */
  std::string encoding_;    /**< The encoding type of the log message body. */
  std::string body_;        /**< The body content of the log message. */
  std::string next_id_; /**< The ID of the next log message in the sequence. */
};

/**
 * @class ParseError
 * @brief Represents an error encountered during parsing.
 *
 * This class encapsulates information about a parsing error, including
 * an error message and the line number where the error occurred.
 */
class ParseError {
 public:
  /**
   * @brief Constructs a ParseError with the given message and line number.
   * @param message The error message.
   * @param line_number The line number where the error occurred.
   */
  ParseError(const std::string& message, size_t line_number)
      : message_(message), line_number_(line_number) {}

  /**
   * @brief Retrieves the error message.
   * @return The error message as a string.
   */
  const std::string& message() const { return message_; }

  /**
   * @brief Retrieves the line number where the error occurred.
   * @return The line number as a size_t.
   */
  size_t line_number() const { return line_number_; }

 private:
  std::string message_; /**< The error message. */
  size_t line_number_;  /**< The line number where the error occurred. */
};

/**
 * @class ParseResult
 * @brief Represents the result of a parsing operation.
 *
 * This class encapsulates the results of a parsing operation, including
 * successfully parsed log messages and any errors encountered during parsing.
 */
class ParseResult {
 public:
  /**
   * @brief Constructs a ParseResult with the given messages and errors.
   * @param messages The successfully parsed log messages.
   * @param errors The errors encountered during parsing.
   */
  ParseResult(const LogMessages& messages, const ParseErrors& errors)
      : messages_(messages), errors_(errors) {}

  /**
   * @brief Retrieves the parsed log messages.
   * @return A reference to the collection of parsed log messages.
   */
  const LogMessages& messages() const { return messages_; }

  /**
   * @brief Retrieves the parsing errors.
   * @return A reference to the collection of parsing errors.
   */
  const ParseErrors& errors() const { return errors_; }

  /**
   * @brief Checks if any errors were encountered during parsing.
   * @return true if there are errors, false otherwise.
   */
  bool HasErrors() const { return !errors_.empty(); }

 private:
  LogMessages messages_; /**< The successfully parsed log messages. */
  ParseErrors errors_;   /**< The errors encountered during parsing. */
};

/**
 * @class Parser
 * @brief Parses structured log messages from an input stream.
 *
 * This class reads structured log messages from an input stream and
 * produces a ParseResult containing the parsed messages and any errors
 * encountered during parsing.
 */
class Parser {
 public:
  /**
   * @brief Constructs a Parser with the given input stream.
   * @param input_stream The input stream containing structured log messages.
   */
  explicit Parser(std::istream& input_stream) : input_stream_(input_stream) {}

  /**
   * @brief Parses the structured log messages from the input stream.
   * @return A ParseResult containing the parsed messages and errors.
   */
  ParseResult Parse();

 private:
  std::istream& input_stream_; /**< The input stream containing log messages. */
};

}  // namespace pipelines::log_message_parser::structure

#endif  // COMPONENT_LOG_MESSAGE_PARSER_PUBLIC_LOG_MESSAGE_PARSER_STRUCTURE_H_