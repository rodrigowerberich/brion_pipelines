/**
 * @file structure.cc
 * @brief Implementation of the structured log message parser.
 * This file contains the implementation of the structured log message parser,
 * which processes log messages from an input stream and extracts relevant
 * information such as pipeline ID, message ID, encoding, body, and next ID.
 * 
 * It uses the StreamProcessor class to read and parse the log messages,
 * handling errors and formatting details. The parser also provides error
 * handling for cases where the input format is invalid or the end of the
 * stream is reached unexpectedly.
 * 
 */

/******************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "log_message_parser/structure.h"
#include <algorithm>
#include <iterator>
#include <locale>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>

/******************************************************************************
 * PRIVATE HELPER DECLARATIONS
 *****************************************************************************/

namespace pipelines::log_message_parser::structure {

// Forward declaration of the StreamProcessor class
class StreamProcessor;

// Private helper function declarations
/**
 * @brief Trims whitespace on the right from a string.
 * @param str The string to trim.
 * @return A new string with whitespace removed.
 */
static std::string TrimRight(std::string str);

/**
 * @brief Trims whitespace on the left from a string.
 * @param str The string to trim.
 * @return A new string with whitespace removed.
 * 
 */
static std::string TrimLeft(std::string str);

/**
 * @brief Trims whitespace from a string.
 * @param str The string to trim.
 * @return A new string with whitespace removed.
 */
static std::string Trim(std::string str);

/**
 * @brief Attempts to read a structured log message from the stream.
 * @param stream_processor The StreamProcessor instance to read from.
 * @param structure_messages The collection of parsed log message where the message will be stored.
 * @param error The collection of parsing errors where any errors will be stored.
 */
static void AttemptToReadStructureLogMessage(StreamProcessor& stream_processor,
                                             LogMessages& structure_messages,
                                             ParseErrors& error);

/**
  * @brief Advances the stream until the end of the line. If anything other than whitespace is found, it will
  * add an error to the error collection.
  * @param stream_processor The StreamProcessor instance to read from.
  * @param error The collection of parsing errors where any errors will be stored.
  */
static void AdvanceUntilEndOfLine(StreamProcessor& stream_processor,
                                  ParseErrors& error);

}  // namespace pipelines::log_message_parser::structure

/******************************************************************************
 * PRIVATE CLASSES
 *****************************************************************************/

namespace pipelines::log_message_parser::structure {
/**
 * @class StreamReadError
 * @brief Represents an error encountered while reading from a stream.
 *
 * This class encapsulates information about a stream reading error, including
 * the error message and the line number where the error occurred.
 */
class StreamReadError : public std::runtime_error {
 public:
  /**
    * @brief Constructs a StreamReadError with the given message and line number.
    * @param message The error message.
    * @param line_number The line number where the error occurred.
    */
  StreamReadError(const std::string& message, size_t line_number)
      : std::runtime_error(message), line_number_(line_number) {}

  /**
    * @brief Retrieves the line number where the error occurred.
    * @return The line number as a size_t.
    */
  size_t line_number() const { return line_number_; }

 private:
  size_t line_number_ = 0; /**< The line number where the error occurred. */
};

/**
 * @class FileEndError
 * @brief Represents an error caused by reaching the end of a file unexpectedly.
 *
 * This class extends StreamReadError to provide specific error handling for
 * cases where the end of a file is reached while reading.
 */
class FileEndError : public StreamReadError {
 public:
  /**
    * @brief Constructs a FileEndError with the given message and line number.
    * @param message The error message.
    * @param line_number The line number where the error occurred.
    */
  FileEndError(const std::string& message, size_t line_number)
      : StreamReadError(message, line_number) {}
};

/**
 * @class BadFormatError
 * @brief Represents an error caused by encountering a bad format in the input.
 *
 * This class extends StreamReadError to provide specific error handling for
 * cases where the input format is invalid.
 */
class BadFormatError : public StreamReadError {
 public:
  /**
    * @brief Constructs a BadFormatError with the given message and line number.
    * @param message The error message.
    * @param line_number The line number where the error occurred.
    */
  BadFormatError(const std::string& message, size_t line_number)
      : StreamReadError(message, line_number) {}
};

/**
 * @class StreamProcessor
 * @brief Processes structured log messages from an input stream.
 *
 * This class provides methods for reading and parsing structured log messages
 * from an input stream. It handles whitespace, brackets, and other formatting
 * details to extract the components of a log message.
 */
class StreamProcessor {
 public:
  /**
    * @brief Constructs a StreamProcessor with the given input stream.
    * @param input_stream The input stream to process.
    */
  explicit StreamProcessor(std::istream& input_stream)
      : current_character_(std::istreambuf_iterator<char>(input_stream)),
        end_character_(std::istreambuf_iterator<char>()) {}

  /**
    * @brief Attempts to read the pipeline ID from the stream.
    * @return The pipeline ID as a string.
    * @throws FileEndError if the pipeline ID cannot be read.
    */
  std::string AttemptToReadPipelineId();

  /**
    * @brief Attempts to read the ID from the stream.
    * @return The ID as a string.
    * @throws FileEndError if the ID cannot be read.
    */
  std::string AttemptToReadId();

  /**
    * @brief Attempts to read the encoding from the stream.
    * @return The encoding as a string.
    * @throws FileEndError if the encoding cannot be read.
    */
  std::string AttemptToReadEncoding();

  /**
    * @brief Attempts to read the body from the stream.
    * @return The body as a string.
    * @throws FileEndError if the body cannot be read.
    * @throws BadFormatError if the body format is invalid.
    */
  std::string AttemptToReadBody();

  /**
    * @brief Attempts to read the next ID from the stream.
    * @return The next ID as a string.
    * @throws FileEndError if the next ID cannot be read.
    */
  std::string AttemptToReadNextId();

  /**
    * @brief Reads characters from the stream until the end of the line.
    * @return A string containing the characters read.
    */
  std::string ReadUntilEndOfLine();

  /**
    * @brief Checks if the stream processing is complete.
    * @return true if the stream has been fully processed, false otherwise.
    */
  bool IsDone();

  /**
    * @brief Retrieves the current line number in the stream.
    * @return The current line number as a size_t.
    */
  size_t line_number() const { return line_number_; }

 private:
  std::istreambuf_iterator<char>
      current_character_; /**< Iterator for the current character in the stream. */
  std::istreambuf_iterator<char>
      end_character_;      /**< Iterator for the end of the stream. */
  size_t line_number_ = 1; /**< The current line number in the stream. */

  /**
    * @brief Advances the current character iterator.
    * 
    * This method increments the current character iterator and updates the
    * line number if a newline character is encountered.
    * It is used to move through the input stream while processing characters.
    */
  void AdvanceCurrentCharacter() {
    if (*current_character_ == '\n') {
      ++line_number_;
    }
    ++current_character_;
  }

  /**
    * @brief Checks if the stream has ended.
    * @return true if the stream has ended, false otherwise.
    */
  bool HasStreamEnded() { return current_character_ == end_character_; }

  /**
    * @brief Checks if the current character is the end of a line.
    * @return true if the current character is a newline or carriage return, false otherwise.
    */
  bool IsEndOfLine() {
    return *current_character_ == '\n' || *current_character_ == '\r';
  }

  /**
    * @brief Checks if the current character is an opening bracket.
    * @return true if the current character is '[', false otherwise.
    */
  bool IsOpenBracket() { return *current_character_ == '['; }

  /**
    * @brief Checks if the current character is a closing bracket.
    * @return true if the current character is ']', false otherwise.
    */
  bool IsCloseBracket() { return *current_character_ == ']'; }

  /**
    * @brief Skips whitespace characters in the stream.
    */
  void SkipWhitespace() {
    while (!HasStreamEnded() && std::isspace(*current_character_)) {
      AdvanceCurrentCharacter();
    }
  }

  /**
    * @brief Reads characters from the stream until whitespace is encountered.
    * @return A string containing the characters read.
    */
  std::string ReadUntilWhitespace();

  /**
    * @brief Attempts to read a continuous string from the stream.
    * @return The continuous string read from the stream.
    * @throws FileEndError if the end of the file is reached unexpectedly.
    */
  std::string AttemptToReadContinuousString();

  /**
   * @brief Attempt to read a continuous string or report given error.
   * @param error_message The error message to report if reading fails.
   * @return The continuous string read from the stream.
   * @throws FileEndError if the end of the file is reached unexpectedly.
   */
  std::string AttemptToReadContinuousStringOrThrow(
      const std::string& error_message);

  /**
   * @brief Read the stream until it find the a matching number of open and close brackets.
   * @param line_number The line number where the search started.
   * @return The body read from the stream.
   * @throws FileEndError if the end of the file is reached unexpectedly.
   * @pre The stream must be positioned at an opening bracket '['.
   */
  std::string SearchForMatchingBrackets(size_t line_number);
};

}  // namespace pipelines::log_message_parser::structure

/******************************************************************************
 * PRIVATE HELPER IMPLEMENTATIONS
 *****************************************************************************/

namespace pipelines::log_message_parser::structure {

static std::string TrimRight(std::string str) {
  str.erase(std::ranges::find_if(str | std::views::reverse,
                                 [](char c) { return !std::isspace(c); })
                .base(),
            end(str));
  return str;
}

static std::string TrimLeft(std::string str) {
  str.erase(begin(str),
            std::ranges::find_if(str, [](char c) { return !std::isspace(c); }));
  return str;
}

static std::string Trim(std::string str) {
  return TrimLeft(TrimRight(str));
}

static void AttemptToReadStructureLogMessage(StreamProcessor& stream_processor,
                                             LogMessages& structure_messages,
                                             ParseErrors& errors) {
  try {
    auto pipeline_id = stream_processor.AttemptToReadPipelineId();
    auto id = stream_processor.AttemptToReadId();
    auto encoding = stream_processor.AttemptToReadEncoding();
    auto body = stream_processor.AttemptToReadBody();
    auto next_id = stream_processor.AttemptToReadNextId();

    structure_messages.emplace_back(pipeline_id, id, encoding, body, next_id);
  } catch (const FileEndError& e) {
    auto error_message = "File ended while parsing: " + std::string(e.what());
    errors.emplace_back(error_message, e.line_number());
  } catch (const BadFormatError& e) {
    auto error_message = "Bad format: " + std::string(e.what());
    errors.emplace_back(error_message, e.line_number());
  }
}

static void AdvanceUntilEndOfLine(StreamProcessor& stream_processor,
                                  ParseErrors& errors) {
  auto line_number = stream_processor.line_number();
  auto line = stream_processor.ReadUntilEndOfLine();

  line = Trim(line);

  if (!line.empty()) {
    auto error_message = "There is unparsed data in line " +
                         std::to_string(line_number) + ": \"" + line + "\"";
    errors.emplace_back(error_message, line_number);
  }
}

}  // namespace pipelines::log_message_parser::structure

/******************************************************************************
 * PRIVATE CLASS METHODS IMPLEMENTATION
 *****************************************************************************/

namespace pipelines::log_message_parser::structure {

std::string StreamProcessor::ReadUntilWhitespace() {
  auto result = std::ostringstream();
  while (!HasStreamEnded() && !std::isspace(*current_character_)) {
    result << *current_character_;
    AdvanceCurrentCharacter();
  }
  return result.str();
}

std::string StreamProcessor::AttemptToReadContinuousString() {
  auto line_number = line_number_;
  SkipWhitespace();

  auto continuous_string = ReadUntilWhitespace();

  if (continuous_string.empty()) {
    throw FileEndError("File ended while reading continuous string",
                       line_number);
  }

  return continuous_string;
}

std::string StreamProcessor::AttemptToReadContinuousStringOrThrow(
    const std::string& error_message) {
  try {
    return AttemptToReadContinuousString();
  } catch (const FileEndError& e) {
    throw FileEndError(error_message, e.line_number());
  }
}

std::string StreamProcessor::AttemptToReadPipelineId() {
  return AttemptToReadContinuousStringOrThrow("Failed to read pipeline ID");
}

std::string StreamProcessor::AttemptToReadId() {
  return AttemptToReadContinuousStringOrThrow("Failed to read ID");
}

std::string StreamProcessor::AttemptToReadEncoding() {
  return AttemptToReadContinuousStringOrThrow("Failed to read encoding");
}

std::string StreamProcessor::AttemptToReadBody() {
  auto body = std::string{};
  auto line_number = line_number_;

  SkipWhitespace();

  if (IsOpenBracket()) {
    body = SearchForMatchingBrackets(line_number);
  } else if (!HasStreamEnded()) {
    line_number = line_number_;
    throw BadFormatError("Expected an opening bracket", line_number);
  } else {
    throw FileEndError("File ended while parsing body", line_number);
  }

  return body;
}

std::string StreamProcessor::AttemptToReadNextId() {
  return AttemptToReadContinuousStringOrThrow("Failed to read next ID");
}

std::string StreamProcessor::ReadUntilEndOfLine() {
  auto result = std::ostringstream();
  while (!HasStreamEnded() && !IsEndOfLine()) {
    result << *current_character_;
    AdvanceCurrentCharacter();
  }
  return result.str();
}

bool StreamProcessor::IsDone() {
  SkipWhitespace();
  return HasStreamEnded();
}

std::string StreamProcessor::SearchForMatchingBrackets(size_t line_number) {
  auto open_bracket_count = 1;
  auto close_bracket_count = 0;
  auto body = std::ostringstream();

  auto bracket_count_does_not_match = [&open_bracket_count,
                                       &close_bracket_count]() {
    return open_bracket_count != close_bracket_count;
  };

  // Skip the opening bracket
  AdvanceCurrentCharacter();

  while (!HasStreamEnded() && bracket_count_does_not_match()) {
    if (IsOpenBracket()) {
      ++open_bracket_count;
    } else if (IsCloseBracket()) {
      ++close_bracket_count;
    }
    if (bracket_count_does_not_match()) {
      body << *current_character_;
    }
    AdvanceCurrentCharacter();
  }
  if (bracket_count_does_not_match()) {
    throw FileEndError(
        "Expected a closing bracket, but reached the end of the file",
        line_number);
  }
  return body.str();
}

}  // namespace pipelines::log_message_parser::structure

/******************************************************************************
 * PUBLIC CLASS METHODS IMPLEMENTATION
 *****************************************************************************/

namespace pipelines::log_message_parser::structure {

ParseResult Parser::Parse() {
  auto structure_messages = LogMessages{};
  auto errors = ParseErrors{};

  auto stream_processor = StreamProcessor{input_stream_};

  while (!stream_processor.IsDone()) {
    AttemptToReadStructureLogMessage(stream_processor, structure_messages,
                                     errors);

    AdvanceUntilEndOfLine(stream_processor, errors);
  }

  return {structure_messages, errors};
}

}  // namespace pipelines::log_message_parser::structure