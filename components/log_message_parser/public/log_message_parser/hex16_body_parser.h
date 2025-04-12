#ifndef COMPONENT_LOG_MESSAGE_PARSER_PUBLIC_LOG_MESSAGE_PARSER_HEX16_BODY_PARSER_H_
#define COMPONENT_LOG_MESSAGE_PARSER_PUBLIC_LOG_MESSAGE_PARSER_HEX16_BODY_PARSER_H_

#include <string>
#include "log_message_parser/semantics.h"

namespace pipelines::log_message_parser::semantics {

class Hex16BodyParser : public BodyParser {
 public:
  std::string parse(const std::string& body) const override;
};

}  // namespace pipelines::log_message_parser::semantics

#endif  // COMPONENT_LOG_MESSAGE_PARSER_PUBLIC_LOG_MESSAGE_PARSER_HEX16_BODY_PARSER_H_