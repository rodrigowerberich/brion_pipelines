/**
 * @file ascii_body_parser.h
 * @brief This file defines the AsciiBodyParser class, which is responsible for
 * parsing ASCII-encoded log message bodies.
 * 
 * This class implements the BodyParser interface and provides a method to
 * parse the body of a log message encoded in ASCII format.
 */
#ifndef COMPONENT_LOG_MESSAGE_PARSER_PUBLIC_LOG_MESSAGE_PARSER_ASCII_BODY_PARSER_H_
#define COMPONENT_LOG_MESSAGE_PARSER_PUBLIC_LOG_MESSAGE_PARSER_ASCII_BODY_PARSER_H_

/******************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <string>
#include "log_message_parser/semantics.h"

/******************************************************************************
 * CLASSES
 *****************************************************************************/

namespace pipelines::log_message_parser::semantics {
/**
 * @brief The AsciiBodyParser class is responsible for parsing ASCII-encoded
 * log message bodies.
 *
 * This class implements the BodyParser interface and provides a method to
 * parse the body of a log message encoded in ASCII format.
 * 
 */
class AsciiBodyParser : public BodyParser {
 public:
  /**
     * @brief Parses the given ASCII-encoded body.
     *
     * The current version does nothing, as the body is already in ASCII
     *
     * @param body The ASCII-encoded body to parse.
     * @return The parsed body as a string.
     */
  std::string Parse(const std::string& body) const override;
};

}  // namespace pipelines::log_message_parser::semantics

#endif  // COMPONENT_LOG_MESSAGE_PARSER_PUBLIC_LOG_MESSAGE_PARSER_ASCII_BODY_PARSER_H_