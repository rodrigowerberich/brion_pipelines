/**
 * @file hex16_body_parser.cc
 * @brief Implementation of the Hex16BodyParser class.
 */

/******************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "log_message_parser/hex16_body_parser.h"
#include "log_message_parser/semantics.h"

#include <algorithm>
#include <iterator>
#include <locale>
#include <ranges>
#include <sstream>
#include <string>

/******************************************************************************
 * PRIVATE HELPER DECLARATIONS
 *****************************************************************************/
namespace pipelines::log_message_parser::semantics {

/**
 * @brief Removes all whitespace characters from the given string.
 *
 * @param str The input string.
 * @return A new string with all whitespace characters removed.
 */
static std::string RemoveWhitespace(const std::string& str);

/**
 * @brief Checks if all characters in the given string are hexadecimal digits.
 *
 * @param str The input string.
 * @return true if all characters are hexadecimal digits, false otherwise.
 */
static bool AreAllCharactersHex(const std::string& str);

/**
 * @brief Converts a hexadecimal string to its corresponding byte representation.
 *
 * @param str The input hexadecimal string.
 * @return A new string containing the byte representation of the input string.
 */
static std::string FromHexStr(const std::string& str);

/**
 * @brief Creates an error message for an odd-length hexadecimal string.
 *
 * @param body The original string.
 * @param length The length of the string.
 * @return A formatted error message.
 */
static std::string CreateOddLengthErrorMessage(const std::string& body,
                                               size_t length);

/**
 * @brief Creates an error message for a non-hexadecimal string.
 * 
 * @param body The original string.
 * @return A formatted error message.
 */
static std::string CreateNonHexErrorMessage(const std::string& body);

}  // namespace pipelines::log_message_parser::semantics

/******************************************************************************
 * PRIVATE HELPER IMPLEMENTATION
 *****************************************************************************/

namespace pipelines::log_message_parser::semantics {

static std::string RemoveWhitespace(const std::string& str) {
  std::string result;
  std::ranges::copy_if(str, std::back_inserter(result),
                       [](char c) { return !std::isspace(c); });
  return result;
}

static bool AreAllCharactersHex(const std::string& str) {
  return std::all_of(str.begin(), str.end(),
                     [](char c) { return std::isxdigit(c); });
}

static std::string FromHexStr(const std::string& str) {
  std::string parsed_body;
  parsed_body.reserve(str.size() / 2);
  for (size_t i = 0; i < str.size(); i += 2) {
    auto byte_str = str.substr(i, 2);
    char byte = static_cast<char>(std::stoi(byte_str, nullptr, 16));
    parsed_body.push_back(byte);
  }
  return parsed_body;
}

static std::string CreateOddLengthErrorMessage(const std::string& body,
                                               size_t length) {
  std::ostringstream oss;
  oss << "Hexadecimal string has an odd number of characters: " << length
      << "\n"
      << "Original string: " << body;
  return oss.str();
}

static std::string CreateNonHexErrorMessage(const std::string& body) {
  std::ostringstream oss;
  oss << "Hexadecimal string contains non-hexadecimal characters\n"
      << "Original string: " << body;
  return oss.str();
}

}  // namespace pipelines::log_message_parser::semantics

/******************************************************************************
 * PUBLIC CLASS METHODS IMPLEMENTATION
 *****************************************************************************/

namespace pipelines::log_message_parser::semantics {

std::string Hex16BodyParser::Parse(const std::string& body) const {
  auto no_whitespace_body = RemoveWhitespace(body);
  auto hex_length = no_whitespace_body.size();
  if ((hex_length % 2) != 0) {
    throw BodyParserError(CreateOddLengthErrorMessage(body, hex_length));
  }
  if (!AreAllCharactersHex(no_whitespace_body)) {
    throw BodyParserError(CreateNonHexErrorMessage(body));
  }

  return FromHexStr(no_whitespace_body);
}

}  // namespace pipelines::log_message_parser::semantics