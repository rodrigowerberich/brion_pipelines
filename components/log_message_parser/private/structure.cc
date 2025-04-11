#include "log_message_parser/structure.h"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <locale>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>

namespace pipelines {

namespace log_message_parser {

namespace structure {

class StreamReadError : public std::runtime_error {
 private:
  size_t line_number_ = 0;

 public:
  StreamReadError(const std::string& message, size_t line_number)
      : std::runtime_error(message), line_number_(line_number) {}

  size_t get_line_number() const { return line_number_; }
};

class FileEndError : public StreamReadError {
 public:
  FileEndError(const std::string& message, size_t line_number)
      : StreamReadError(message, line_number) {}
};

class BadFormatError : public StreamReadError {
 public:
  BadFormatError(const std::string& message, size_t line_number)
      : StreamReadError(message, line_number) {}
};

class StreamProcessor {
 private:
  std::istreambuf_iterator<char> current_character_;
  std::istreambuf_iterator<char> end_character_;
  size_t line_number_ = 1;

  void advance_current_character() {
    if (*current_character_ == '\n') {
      ++line_number_;
    }
    ++current_character_;
  }

  void trim_string(std::string& str) {
    str.erase(std::remove_if(str.begin(), str.end(), ::isspace), str.end());
  }

  bool has_stream_ended() { return current_character_ == end_character_; }

  bool is_end_of_line() {
    return *current_character_ == '\n' || *current_character_ == '\r';
  }

  bool is_open_bracket() { return *current_character_ == '['; }

  bool is_close_bracket() { return *current_character_ == ']'; }

  void skip_whitespace() {
    while (!has_stream_ended() && std::isspace(*current_character_)) {
      advance_current_character();
    }
  }

  std::string read_until_whitespace() {
    auto result = std::ostringstream();
    while (!has_stream_ended() && !std::isspace(*current_character_)) {
      result << *current_character_;
      advance_current_character();
    }
    return result.str();
  }

  std::string attempt_to_read_continuous_string() {

    auto line_number = line_number_;
    skip_whitespace();

    auto continuous_string = read_until_whitespace();

    if (continuous_string.empty()) {
      throw FileEndError("File ended while reading continuous string",
                         line_number);
    }

    return continuous_string;
  }

 public:
  StreamProcessor(std::istream& input_stream)
      : current_character_(std::istreambuf_iterator<char>(input_stream)),
        end_character_(std::istreambuf_iterator<char>()) {}

  std::string attempt_to_read_pipeline_id() {
    try {
      return attempt_to_read_continuous_string();
    } catch (const FileEndError& e) {
      throw FileEndError("Failed to read pipeline ID", e.get_line_number());
    }
  }

  std::string attempt_to_read_id() {
    try {
      return attempt_to_read_continuous_string();
    } catch (const FileEndError& e) {
      throw FileEndError("Failed to read ID", e.get_line_number());
    }
  }

  std::string attempt_to_read_encoding() {
    try {
      return attempt_to_read_continuous_string();
    } catch (const FileEndError& e) {
      throw FileEndError("Failed to read encoding", e.get_line_number());
    }
  }

  // We read all open brackets until all close brackets are found and discard the first and last brackets
  std::string attempt_to_read_body() {
    auto open_bracket_count = 0;
    auto close_bracket_count = 0;
    auto body = std::ostringstream();
    auto line_number = line_number_;

    skip_whitespace();

    if (is_open_bracket()) {
      advance_current_character();
      ++open_bracket_count;
      while (!has_stream_ended() &&
             (open_bracket_count != close_bracket_count)) {
        if (is_open_bracket()) {
          ++open_bracket_count;
        } else if (is_close_bracket()) {
          ++close_bracket_count;
        }
        if (open_bracket_count != close_bracket_count) {
          body << *current_character_;
        }
        advance_current_character();
      }
    } else if (!has_stream_ended()) {
      line_number = line_number_;
      throw BadFormatError("Expected an opening bracket", line_number);
    } else {
      throw FileEndError("File ended while parsing body", line_number);
    }

    if (open_bracket_count != close_bracket_count) {
      throw FileEndError(
          "Expected a closing bracket, but reached the end of the file",
          line_number);
    }

    return body.str();
  }

  std::string attempt_to_read_next_id() {
    try {
      return attempt_to_read_continuous_string();
    } catch (const FileEndError& e) {
      throw FileEndError("Failed to read next ID", e.get_line_number());
    }
  }

  std::string read_until_end_of_line() {
    auto result = std::ostringstream();
    while (!has_stream_ended() && !is_end_of_line()) {
      result << *current_character_;
      advance_current_character();
    }
    return result.str();
  }

  bool is_done() {
    skip_whitespace();
    return current_character_ == end_character_;
  }

  size_t get_line_number() const { return line_number_; }
};

static std::string trim(const std::string& str) {
  auto trimmed_str = str;
  trimmed_str.erase(
      std::remove_if(trimmed_str.begin(), trimmed_str.end(), ::isspace),
      trimmed_str.end());
  return trimmed_str;
}

ParseResult Parser::parse() {
  auto raw_messages = LogMessages{};
  auto errors = ParseErrors{};

  auto stream_processor = StreamProcessor{input_stream_};

  while (!stream_processor.is_done()) {
    try {
      auto pipeline_id = stream_processor.attempt_to_read_pipeline_id();
      auto id = stream_processor.attempt_to_read_id();
      auto encoding = stream_processor.attempt_to_read_encoding();
      auto body = stream_processor.attempt_to_read_body();
      auto next_id = stream_processor.attempt_to_read_next_id();

      raw_messages.emplace_back(pipeline_id, id, encoding, body, next_id);
    } catch (const FileEndError& e) {
      auto error_message = "File ended while parsing: " + std::string(e.what());
      errors.emplace_back(error_message, e.get_line_number());
      break;
    } catch (const BadFormatError& e) {
      auto error_message = "Bad format: " + std::string(e.what());
      errors.emplace_back(error_message, e.get_line_number());
    }

    auto line_number = stream_processor.get_line_number();
    auto line = stream_processor.read_until_end_of_line();
    line = trim(line);

    if (!line.empty()) {
      auto error_message = "There is unparsed data in line " +
                           std::to_string(line_number) + ": \"" + line + "\"";
      errors.emplace_back(error_message, line_number);
    }
  }

  return {raw_messages, errors};
}

}  // namespace structure

}  // namespace log_message_parser
}  // namespace pipelines