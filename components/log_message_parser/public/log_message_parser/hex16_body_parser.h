/**
 * @file hex16_body_parser.h
 * @brief Defines the Hex16BodyParser class for parsing hexadecimal 16-bit body messages.
 *
 * This file declares the Hex16BodyParser class, which is a specialized implementation
 * of the BodyParser interface. It provides functionality to parse hexadecimal 16-bit
 * formatted body messages.
 */

#ifndef COMPONENT_LOG_MESSAGE_PARSER_PUBLIC_LOG_MESSAGE_PARSER_HEX16_BODY_PARSER_H_
#define COMPONENT_LOG_MESSAGE_PARSER_PUBLIC_LOG_MESSAGE_PARSER_HEX16_BODY_PARSER_H_

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
 * @class Hex16BodyParser
 * @brief A parser for hexadecimal 16-bit body messages.
 *
 * The Hex16BodyParser class is responsible for parsing body messages that are
 * formatted as hexadecimal 16-bit values. 
 */
class Hex16BodyParser : public BodyParser {
 public:
  /**
   * @brief Parses a hexadecimal 16-bit formatted body message.
   *
   * This method takes a string representation of a body message and parses it
   * as a hexadecimal 16-bit value. The parsed result is returned as a string.
   * Any whitespace characters in the input string are ignored.
   * If the input string has an odd length or contains non-hexadecimal characters,
   * a BodyParserError exception is thrown.
   *
   * @param body The body message to be parsed, represented as a string.
   * @return A string containing the parsed result.
   */
  std::string Parse(const std::string& body) const override;
};

}  // namespace pipelines::log_message_parser::semantics

#endif  // COMPONENT_LOG_MESSAGE_PARSER_PUBLIC_LOG_MESSAGE_PARSER_HEX16_BODY_PARSER_H_