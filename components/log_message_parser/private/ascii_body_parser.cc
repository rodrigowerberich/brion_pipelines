#include "log_message_parser/ascii_body_parser.h"
#include "log_message_parser/semantics.h"

namespace pipelines::log_message_parser::semantics {

std::string AsciiBodyParser::parse(const std::string& body) const {
  return body;
}

}  // namespace pipelines::log_message_parser::semantics