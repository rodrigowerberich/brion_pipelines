/**
 * @file semantics.h
 * @brief Defines core classes and types for log message parsing semantics.
 *
 * This file contains the declarations of key classes and types used in the
 * log message parsing process, including error handling, parsing results,
 * and the base interface for body parsers.
 */

#ifndef COMPONENT_LOG_MESSAGE_PARSER_PUBLIC_LOG_MESSAGE_PARSER_SEMANTICS_H_
#define COMPONENT_LOG_MESSAGE_PARSER_PUBLIC_LOG_MESSAGE_PARSER_SEMANTICS_H_

/******************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <map>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "log_message/message.h"
#include "log_message_parser/structure.h"

/******************************************************************************
 * TYPE DEFINITIONS
 *****************************************************************************/

namespace pipelines::log_message_parser::semantics {

using LogMessage =
    ::pipelines::log_message::Message; /**< Alias for log message type. */
using LogMessages =
    ::std::vector<LogMessage>; /**< Collection of log messages. */
using ParseErrors =
    ::std::vector<class ParseError>; /**< Collection of parsing errors. */

}  // namespace pipelines::log_message_parser::semantics

/******************************************************************************
 * CLASSES
 *****************************************************************************/

namespace pipelines::log_message_parser::semantics {

/**
 * @class ParseError
 * @brief Represents an error encountered during parsing.
 *
 * This class encapsulates information about a parsing error, including
 * an error message describing the issue.
 */
class ParseError {
 public:
  /**
   * @brief Constructs a ParseError with the given message.
   * @param message The error message.
   */
  ParseError(const std::string& message) : message_(message) {}

  /**
   * @brief Retrieves the error message.
   * @return The error message as a string.
   */
  const std::string& message() const { return message_; }

 private:
  std::string message_; /**< The error message. */
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
 * @class BodyParserError
 * @brief Represents an error specific to body parsing.
 *
 * This class extends `std::runtime_error` to provide detailed error
 * information for issues encountered during body parsing.
 */
class BodyParserError : public std::runtime_error {
 public:
  /**
   * @brief Constructs a BodyParserError with the given message.
   * @param message The error message.
   */
  explicit BodyParserError(const std::string& message)
      : std::runtime_error(message) {}
};

/**
 * @class BodyParser
 * @brief Abstract base class for body parsers.
 *
 * This class defines the interface for body parsers, which are responsible
 * for parsing the body of a log message based on a specific encoding.
 */
class BodyParser {
 public:
  /**
   * @brief Virtual destructor for the BodyParser class.
   */
  virtual ~BodyParser() = default;

  /**
   * @brief Parses the given body message.
   * @param body The body message to parse.
   * @return The parsed body as a string.
   * @throws BodyParserError if parsing fails.
   */
  virtual std::string Parse(const std::string& body) const = 0;
};

/**
 * @class Parser
 * @brief Main parser class for the semantics of log messages.
 *
 * This class manages the parsing of log messages using registered body
 * parsers for specific encodings. It processes a collection of structured
 * log messages and produces a ParseResult containing the parsed messages
 * and any errors encountered.
 */
class Parser {
 private:
  using BodyParserPtr =
      std::unique_ptr<BodyParser>; /**< Pointer to a body parser. */
  using BodyParserMap =
      std::map<std::string, BodyParserPtr>; /** Map of body parsers. */

 public:
  /**
   * @brief Default constructor for the Parser class.
   */
  Parser() = default;

  /**
   * @brief Registers a body parser for a specific encoding.
   * @param encoding The encoding type can be any string, for future use. Currently only "0" (ascii) and "1"(hex16) are used.
   * @param body_parser A unique pointer to the body parser.
   */
  void RegisterBodyParser(const std::string& encoding,
                          BodyParserPtr body_parser) {
    body_parsers_[encoding] = std::move(body_parser);
  }

  /**
   * @brief Parses the structured log messages.
   * @param structure_log_messages The structured log messages to parse.
   * @return A ParseResult containing the parsed messages and errors.
   */
  ParseResult Parse(const structure::LogMessages& structure_log_messages);

 private:
  BodyParserMap body_parsers_; /**< Registered body parsers. */
};

}  // namespace pipelines::log_message_parser::semantics

#endif  // COMPONENT_LOG_MESSAGE_PARSER_PUBLIC_LOG_MESSAGE_PARSER_SEMANTICS_H_
