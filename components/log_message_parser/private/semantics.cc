#include "log_message_parser/semantics.h"
#include "log_message_parser/structure.h"

#include <sstream>

namespace pipelines {
namespace log_message_parser {
namespace semantics {

void Parser::register_body_parser(const std::string& encoding,
                                  std::unique_ptr<BodyParser> body_parser) {
  body_parsers_[encoding] = std::move(body_parser);
}

ParseResult Parser::parse() {
  auto parsed_messages = LogMessages{};
  auto errors = ParseErrors{};

  for (const auto& structure_message : structure_log_messages_) {
    auto pipeline_id = structure_message.get_pipeline_id();
    auto id = structure_message.get_id();
    auto encoding = structure_message.get_encoding();
    auto body = structure_message.get_body();
    auto next_id = structure_message.get_next_id();

    if (auto it = body_parsers_.find(encoding); it != body_parsers_.end()) {
      try {
        body = it->second->parse(body);
        parsed_messages.emplace_back(pipeline_id, id, body, next_id);

      } catch (const BodyParserError& e) {
        std::stringstream error_message;
        error_message << "Failed to parse body for log message : \""
                      << structure_message << "\" with encoding \"" << encoding
                      << "\": " << e.what();
        errors.emplace_back(error_message.str());
      }
    } else {
      std::stringstream error_message;
      error_message << "Encoding \"" << encoding
                    << "\" is not supported for log message : \""
                    << structure_message << "\"";
      errors.emplace_back(error_message.str());
    }
  }

  return {parsed_messages, errors};
}

}  //namespace semantics
}  //namespace log_message_parser
}  //namespace pipelines
