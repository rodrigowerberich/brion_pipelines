#include "log_message_parser/hex16_body_parser.h"
#include "log_message_parser/semantics.h"

#include <algorithm>
#include <iterator>
#include <locale>
#include <ranges>
#include <string>

namespace pipelines::log_message_parser::semantics {

static std::string remove_whitespace(const std::string& str) {
  std::string result;
  std::ranges::copy_if(str, std::back_inserter(result),
                       [](char c) { return !std::isspace(c); });
  return result;
}

static bool all_characters_are_hex(const std::string& str) {
  return std::all_of(str.begin(), str.end(),
                     [](char c) { return std::isxdigit(c); });
}

static std::string from_hex_str(const std::string& str) {
  std::string parsed_body;
  parsed_body.reserve(str.size() / 2);
  for (size_t i = 0; i < str.size(); i += 2) {
    auto byte_str = str.substr(i, 2);
    char byte = static_cast<char>(std::stoi(byte_str, nullptr, 16));
    parsed_body.push_back(byte);
  }
  return parsed_body;
}

std::string Hex16BodyParser::parse(const std::string& body) const {
  auto no_white_space_body = remove_whitespace(body);
  if (no_white_space_body.size() % 2 != 0) {
    throw BodyParserError(
        "Hexadecimal string has an odd number of characters: " +
        std::to_string(no_white_space_body.size()) +
        "\n Original string: " + body);
  }
  if (!all_characters_are_hex(no_white_space_body)) {
    throw BodyParserError(
        std::string{
            "Hexadecimal string contains non-hexadecimal characters\n"} +
        "Original string: " + body);
  }

  return from_hex_str(no_white_space_body);
}

}  // namespace pipelines::log_message_parser::semantics