/**
 * @file app.cc
 * 
 * Main application file for the pipeline log message processing system.
 * 
 */

#include <fstream>
#include <iostream>
#include <string>
#include "log_message/message.h"
#include "log_message_organizer/organize_by_id.h"
#include "log_message_organizer/split_by_pipeline.h"
#include "log_message_parser/ascii_body_parser.h"
#include "log_message_parser/hex16_body_parser.h"
#include "log_message_parser/semantics.h"
#include "log_message_parser/structure.h"

/******************************************************************************
 * PRIVATE HELPER DECLARATIONS
 *****************************************************************************/

/******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
    return 1;
  }

  const std::string input_file = argv[1];
  std::ifstream file(input_file);
  if (!file.is_open()) {
    std::cerr << "Error opening file: " << input_file << std::endl;
    return 1;
  }

  using StructureParser = pipelines::log_message_parser::structure::Parser;
  using SemanticParser = pipelines::log_message_parser::semantics::Parser;
  using HexBodyParser =
      pipelines::log_message_parser::semantics::Hex16BodyParser;
  using AsciiBodyParser =
      pipelines::log_message_parser::semantics::AsciiBodyParser;
  using SplitByPipeline = pipelines::log_message_organizer::SplitByPipeline;
  using OrganizeById = pipelines::log_message_organizer::OrganizeById;

  auto structure_parse_result = StructureParser{file}.Parse();
  auto semantics_parser = SemanticParser{structure_parse_result.messages()};
  semantics_parser.RegisterBodyParser("0", std::make_unique<AsciiBodyParser>());
  semantics_parser.RegisterBodyParser("1", std::make_unique<HexBodyParser>());

  auto semantic_parse_result = semantics_parser.Parse();

  auto splitter = SplitByPipeline(semantic_parse_result.messages());
  auto messages_by_pipeline = splitter.Split();

  for (const auto& [pipeline_id, messages] : messages_by_pipeline) {
    auto organizer = OrganizeById(messages);
    auto organized_messages = organizer.Organize();
    std::cout << "Pipeline " << pipeline_id << std::endl;
    for (const auto& message : organized_messages) {
      std::cout << "    " << message.id() << "| " << message.body()
                << std::endl;
    }
  }

  return 0;
}