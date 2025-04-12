#ifndef COMPONENT_LOG_MESSAGE_PARSER_PUBLIC_LOG_MESSAGE_PARSER_SEMANTICS_H_
#define COMPONENT_LOG_MESSAGE_PARSER_PUBLIC_LOG_MESSAGE_PARSER_SEMANTICS_H_

#include <map>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>

#include "log_message_parser/structure.h"

namespace pipelines::log_message_parser::semantics {

class LogMessage {
 public:
  LogMessage() = default;
  LogMessage(const std::string& pipeline_id, const std::string& id,
             const std::string& body, const std::string& next_id)
      : pipeline_id_(pipeline_id), id_(id), body_(body), next_id_(next_id) {}

  const std::string& get_pipeline_id() const { return pipeline_id_; }
  const std::string& get_id() const { return id_; }
  const std::string& get_body() const { return body_; }
  const std::string& get_next_id() const { return next_id_; }
  bool operator==(const LogMessage& other) const {
    return pipeline_id_ == other.pipeline_id_ && id_ == other.id_ &&
           body_ == other.body_ && next_id_ == other.next_id_;
  }
  friend std::ostream& operator<<(std::ostream& os, const LogMessage& message) {
    os << "(Pipeline ID: \"" << message.pipeline_id_ << "\", "
       << "ID: \"" << message.id_ << "\", "
       << "Body: \"" << message.body_ << "\", "
       << "Next ID: \"" << message.next_id_ << "\")";
    return os;
  }

 private:
  std::string pipeline_id_;
  std::string id_;
  std::string body_;
  std::string next_id_;
};

class ParseError {
 public:
  ParseError(const std::string& message) : message_(message) {}

  const std::string& get_message() const { return message_; }

 private:
  std::string message_;
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

class BodyParserError : public std::runtime_error {
 public:
  explicit BodyParserError(const std::string& message)
      : std::runtime_error(message) {}
};

class BodyParser {
 public:
  virtual ~BodyParser() = default;
  virtual std::string parse(const std::string& body) const = 0;
};

class Parser {
 public:
  explicit Parser(const structure::LogMessages& structure_log_messages)
      : structure_log_messages_(structure_log_messages) {}

  void register_body_parser(const std::string& encoding,
                            std::unique_ptr<BodyParser> body_parser);

  ParseResult parse();

 private:
  structure::LogMessages structure_log_messages_;
  std::map<std::string, std::unique_ptr<BodyParser>> body_parsers_;
};

}  // namespace pipelines::log_message_parser::semantics

#endif  // COMPONENT_LOG_MESSAGE_PARSER_PUBLIC_LOG_MESSAGE_PARSER_SEMANTICS_H_
