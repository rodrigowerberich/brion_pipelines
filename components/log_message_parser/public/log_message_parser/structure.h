#ifndef COMPONENT_LOG_MESSAGE_PARSER_PUBLIC_LOG_MESSAGE_PARSER_STRUCTURE_H_
#define COMPONENT_LOG_MESSAGE_PARSER_PUBLIC_LOG_MESSAGE_PARSER_STRUCTURE_H_

#include <istream>
#include <string>
#include <vector>

namespace pipelines::log_message_parser::structure {

class LogMessage {
 public:
  LogMessage() = default;
  LogMessage(const std::string& pipeline_id, const std::string& id,
             const std::string& encoding, const std::string& body,
             const std::string& next_id)
      : pipeline_id_(pipeline_id),
        id_(id),
        encoding_(encoding),
        body_(body),
        next_id_(next_id) {}

  const std::string& pipeline_id() const { return pipeline_id_; }
  const std::string& id() const { return id_; }
  const std::string& body() const { return body_; }
  const std::string& next_id() const { return next_id_; }
  const std::string& get_encoding() const { return encoding_; }

  bool operator==(const LogMessage& other) const {
    return pipeline_id_ == other.pipeline_id_ && id_ == other.id_ &&
           encoding_ == other.encoding_ && body_ == other.body_ &&
           next_id_ == other.next_id_;
  }

  friend std::ostream& operator<<(std::ostream& os, const LogMessage& message) {
    os << "(Pipeline ID: \"" << message.pipeline_id_ << "\", "
       << "ID: \"" << message.id_ << "\", "
       << "Encoding: \"" << message.encoding_ << "\", "
       << "Body: \"" << message.body_ << "\", "
       << "Next ID: \"" << message.next_id_ << "\")";
    return os;
  }

 private:
  std::string pipeline_id_;
  std::string id_;
  std::string encoding_;
  std::string body_;
  std::string next_id_;
};

class ParseError {
 public:
  ParseError(const std::string& message, size_t line_number)
      : message_(message), line_number_(line_number) {}

  const std::string& get_message() const { return message_; }
  size_t get_line_number() const { return line_number_; }

 private:
  std::string message_;
  size_t line_number_;
};

using LogMessages = std::vector<LogMessage>;
using ParseErrors = std::vector<ParseError>;

class ParseResult {
 public:
  ParseResult(const LogMessages& messages, const ParseErrors& errors)
      : messages_(messages), errors_(errors) {}

  const LogMessages& get_messages() const { return messages_; }
  const ParseErrors& get_errors() const { return errors_; }
  bool has_errors() const { return !errors_.empty(); }

 private:
  LogMessages messages_;
  ParseErrors errors_;
};

class Parser {
 public:
  explicit Parser(std::istream& input_stream) : input_stream_(input_stream) {}

  ParseResult parse();

 private:
  std::istream& input_stream_;
};

}  // namespace pipelines::log_message_parser::structure

#endif  // COMPONENT_LOG_MESSAGE_PARSER_PUBLIC_LOG_MESSAGE_PARSER_STRUCTURE_H_