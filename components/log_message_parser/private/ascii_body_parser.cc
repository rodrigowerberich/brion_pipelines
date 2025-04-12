/**
 * @file ascii_body_parser.cc
 * @brief Implementation of the AsciiBodyParser class.
 */

/******************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "log_message_parser/ascii_body_parser.h"
#include "log_message_parser/semantics.h"

/******************************************************************************
 * PUBLIC CLASS METHODS IMPLEMENTATION
 *****************************************************************************/

namespace pipelines::log_message_parser::semantics {

std::string AsciiBodyParser::Parse(const std::string& body) const {
  return body;
}

}  // namespace pipelines::log_message_parser::semantics