/**
 * @file semantics.cc
 * @brief Implementation of the Parser class and related functionality.
 *
 * This file contains the implementation of the Parser class, which is responsible
 * for managing the parsing of log messages using registered body parsers for
 * specific encodings. It processes structured log messages and produces a
 * ParseResult containing the parsed messages and any errors encountered.
 */

/******************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "log_message_parser/semantics.h"
#include "log_message_parser/structure.h"

#include <sstream>

/******************************************************************************
 * PRIVATE HELPER DECLARATIONS
 *****************************************************************************/

namespace pipelines::log_message_parser::semantics {
/**
   * @brief Create an error message when a body is not successfully parsed.
   * 
   * @param structure_message The structure message that failed to parse.
   * @param encoding The encoding used for parsing.
   * @param error The error encountered during parsing.
   * @return A formatted error message.
   */
static std::string CreateBodyParseErrorMessage(
    const structure::LogMessage& structure_message, const std::string& encoding,
    const BodyParserError& error);

/**
 * @brief Create an error message when an encoding is not supported.
 *
 * @param structure_message The structure message that failed to parse.
 * @param encoding The encoding used for parsing.
 * @return A formatted error message.
 */
static std::string CreateUnsupportedEncodingErrorMessage(
    const structure::LogMessage& structure_message,
    const std::string& encoding);

}  // namespace pipelines::log_message_parser::semantics

/******************************************************************************
 * PRIVATE HELPER IMPLEMENTATION
 *****************************************************************************/

namespace pipelines::log_message_parser::semantics {

static std::string CreateBodyParseErrorMessage(
    const structure::LogMessage& structure_message, const std::string& encoding,
    const BodyParserError& error) {
  std::ostringstream oss;
  oss << "Failed to parse body for log message: \"" << structure_message
      << "\" with encoding \"" << encoding << "\": " << error.what();
  return oss.str();
}

static std::string CreateUnsupportedEncodingErrorMessage(
    const structure::LogMessage& structure_message,
    const std::string& encoding) {
  std::ostringstream oss;
  oss << "Encoding \"" << encoding << "\" is not supported for log message: \""
      << structure_message << "\"";
  return oss.str();
}

}  // namespace pipelines::log_message_parser::semantics

/******************************************************************************
 * PUBLIC CLASS METHODS IMPLEMENTATION
 *****************************************************************************/

namespace pipelines::log_message_parser::semantics {

ParseResult Parser::Parse(
    const structure::LogMessages& structure_log_messages) {
  auto parsed_messages = LogMessages{};
  auto errors = ParseErrors{};

  for (const auto& structure_message : structure_log_messages) {
    const auto& pipeline_id = structure_message.pipeline_id();
    const auto& id = structure_message.id();
    const auto& encoding = structure_message.encoding();
    const auto& body = structure_message.body();
    const auto& next_id = structure_message.next_id();

    // Check if a body parser is registered for the given encoding.
    if (auto it = body_parsers_.find(encoding); it != end(body_parsers_)) {
      try {
        // Parse the body using the registered parser.
        auto parsed_body = it->second->Parse(body);
        parsed_messages.emplace_back(pipeline_id, id, parsed_body, next_id);

      } catch (const BodyParserError& e) {
        // Handle parsing errors and record them.
        auto error_message =
            CreateBodyParseErrorMessage(structure_message, encoding, e);
        errors.emplace_back(error_message);
      }
    } else {
      // Handle unsupported encoding errors.
      auto error_message =
          CreateUnsupportedEncodingErrorMessage(structure_message, encoding);
      errors.emplace_back(error_message);
    }
  }

  // Return the result containing parsed messages and errors.
  return {parsed_messages, errors};
}

}  // namespace pipelines::log_message_parser::semantics
