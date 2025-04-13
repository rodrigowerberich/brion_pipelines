/**
 * @file app.cc
 * 
 * Main application file for the pipeline log message processing system.
 * 
 */

#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include "clipp.h"
#include "log_message/message.h"
#include "log_message_organizer/organize_by_id.h"
#include "log_message_organizer/split_by_pipeline.h"
#include "log_message_parser/ascii_body_parser.h"
#include "log_message_parser/hex16_body_parser.h"
#include "log_message_parser/semantics.h"
#include "log_message_parser/structure.h"

/******************************************************************************
 * TYPEDEFS AND ALIASES
 *****************************************************************************/
namespace pipelines::app {

/// Type alias for the structure parser results
using StructureParseResult = log_message_parser::structure::ParseResult;

/// Type alias for the semantics parser results
using SemanticsParseResult = log_message_parser::semantics::ParseResult;

/// Type alias for the log messages
using SemanticsLogMessages = log_message_parser::semantics::LogMessages;

/// Type alias for the log message errors separated by pipeline
using MessagesByPipeline = log_message_organizer::PipelineLogMessagesByPipeline;

}  // namespace pipelines::app

/******************************************************************************
 * CLASSES
 *****************************************************************************/
namespace pipelines::app {

/**
 * @class CommandLineArguments
 * @brief The command line arguments of the application
 * 
 * This class has all the parsed command line arguments
 */
struct CommandLineArguments {
  /// The name of the input file where the data is
  std::string input_file{};
  /// If output_to_file is true, this will contain the name of the file where we will write the output
  std::string output_file{};
  /// Set if the user wants the output written to a file
  bool output_to_file = false;
  /// When set, will print out any warning/errors
  bool verbose = false;
  /// If set will print out the help message
  bool help = false;
  /// When set will make any error cause a failure
  bool strict = false;
};

/**
 * @class ApplicationRuntimeError
 * @brief All runtime application errors should inherit from this class
 */
class ApplicationRuntimeError : public std::runtime_error {
 public:
  /**
   * @brief Constructor
   * @param message Error message
   */
  explicit ApplicationRuntimeError(const std::string& message)
      : std::runtime_error(message) {}
};

}  // namespace pipelines::app

/******************************************************************************
 * PRIVATE HELPER DECLARATIONS
 *****************************************************************************/

namespace pipelines::app {

/**
 * @brief Parses command line arguments.
 * @param argc The number of command line arguments.
 * @param argv The command line arguments.
 * @return A pair containing a boolean indicating success and the parsed command line arguments.
 */
static std::pair<bool, CommandLineArguments> ParseCommandLineArguments(
    int argc, char* argv[]);

static void RunApplication(const CommandLineArguments& cli_args);

/**
 * @brief Parses the structure of the log messages from the input file.
 * @param input_file The input file containing log messages.
 * @return The parsed structure of the log messages.
 */
static StructureParseResult ParseStructure(const std::string& input_file);
/**
 * @brief Parses the semantics of the log messages from the structure parse result.
 * @param structure_parse_result The structure parse result.
 * @return The parsed semantics of the log messages.
 */
static SemanticsParseResult ParseSemantics(
    const StructureParseResult& structure_parse_result);
/**
 * @brief Parses the input file and returns the log messages.
 * @param input_file The input file containing log messages.
 * @param cli_args The command line arguments.
 * @return The parsed log messages.
 */
static SemanticsLogMessages ParseInputFile(
    const std::string& input_file, const CommandLineArguments& cli_args);
/**
 * @brief Prints the log messages for a specific pipeline.
 * @param oss The output stream to print to.
 * @param pipeline_id The ID of the pipeline.
 * @param messages The log messages for the pipeline.
 */
static void PrintPipelineLogMessages(
    std::ostream& oss, const std::string& pipeline_id,
    const log_message_organizer::PipelineLogMessages& messages);
/**
 * @brief Prints the log messages for all pipelines.
 * @param oss The output stream to print to.
 * @param messages The log messages for all pipelines.
 */
static void PrintPipeline(std::ostream& oss,
                          const MessagesByPipeline& messages);
/**
 * @brief Outputs the log messages to the specified output stream or file (decided by the cli).
 * @param messages The log messages to output.
 * @param cli_args The command line arguments.
 */
static void OutputMessages(const MessagesByPipeline& messages,
                           const CommandLineArguments& cli_args);
/**
 * @brief Runs the application with the specified command line arguments.
 * @param cli_args The command line arguments.
 */
static void RunApplication(const CommandLineArguments& cli_args);

}  // namespace pipelines::app

/******************************************************************************
 * PRIVATE HELPER IMPLEMENTATIONS
 *****************************************************************************/

namespace pipelines::app {

static std::pair<bool, CommandLineArguments> ParseCommandLineArguments(
    int argc, char* argv[]) {
  using namespace clipp;

  auto cli_args = CommandLineArguments{};

  auto cli =
      (option("-h", "--help").set(cli_args.help) % "show this help message",
       value("infile", cli_args.input_file) % "input filename",
       option("-v", "--verbose").set(cli_args.verbose) %
           "verbose output, will show all warnings",
       option("-s", "--strict").set(cli_args.strict) %
           "strict mode, will throw an error if any warnings are found",
       option("-o", "--output").set(cli_args.output_to_file) %
               "output to file" &
           value("outfile", cli_args.output_file));

  auto success = parse(argc, argv, cli);
  if (!success || cli_args.help) {
    std::cout << make_man_page(cli, argv[0]) << '\n';
  }

  return {static_cast<bool>(success), cli_args};
}

static StructureParseResult ParseStructure(const std::string& input_file) {
  using StructureParser = log_message_parser::structure::Parser;

  std::ifstream file(input_file);
  if (!file.is_open()) {
    throw ApplicationRuntimeError("Error opening file: " + input_file);
  }

  auto structure_parser = StructureParser{file};
  return structure_parser.Parse();
}

static SemanticsParseResult ParseSemantics(
    const StructureParseResult& structure_parse_result) {
  using SemanticParser = log_message_parser::semantics::Parser;
  using HexBodyParser = log_message_parser::semantics::Hex16BodyParser;
  using AsciiBodyParser = log_message_parser::semantics::AsciiBodyParser;

  auto semantics_parser = SemanticParser{};
  semantics_parser.RegisterBodyParser("0", std::make_unique<AsciiBodyParser>());
  semantics_parser.RegisterBodyParser("1", std::make_unique<HexBodyParser>());

  auto semantic_parse_result =
      semantics_parser.Parse(structure_parse_result.messages());

  return semantic_parse_result;
}

static SemanticsLogMessages ParseInputFile(
    const std::string& input_file, const CommandLineArguments& cli_args) {
  auto show_warnings = cli_args.verbose;
  auto strict = cli_args.strict;

  auto structure_results = ParseStructure(input_file);
  auto semantic_parse_result = ParseSemantics(structure_results);

  auto has_errors =
      structure_results.HasErrors() || semantic_parse_result.HasErrors();

  if (show_warnings && has_errors) {
    std::cerr << "Some problems were found while parsing: " << input_file
              << " the output may be incomplete or incorrect." << std::endl;
    for (const auto& error : structure_results.errors()) {
      std::cerr << "Structure error: " << error.message() << std::endl;
    }
    for (const auto& error : semantic_parse_result.errors()) {
      std::cerr << "Semantic error: " << error.message() << std::endl;
    }
  }
  if (has_errors && strict) {
    throw ApplicationRuntimeError("Strict mode enabled, errors found.");
  }

  return semantic_parse_result.messages();
}

static void PrintPipelineLogMessages(
    std::ostream& oss, const std::string& pipeline_id,
    const log_message_organizer::PipelineLogMessages& messages) {
  oss << "Pipeline " << pipeline_id << std::endl;
  for (const auto& message : messages) {
    oss << "    " << message.id() << "| " << message.body() << std::endl;
  }
}

static void PrintPipeline(std::ostream& oss,
                          const MessagesByPipeline& messages) {
  for (const auto& [pipeline_id, messages] : messages) {
    PrintPipelineLogMessages(oss, pipeline_id, messages);
  }
}

static void OutputMessages(const MessagesByPipeline& messages,
                           const CommandLineArguments& cli_args) {

  if (cli_args.output_to_file) {
    std::ofstream output_file(cli_args.output_file);
    if (!output_file.is_open()) {
      throw ApplicationRuntimeError("Error opening output file: " +
                                    cli_args.output_file);
    }
    PrintPipeline(output_file, messages);
  } else {
    PrintPipeline(std::cout, messages);
  }
}

static void RunApplication(const CommandLineArguments& cli_args) {
  const std::string input_file = cli_args.input_file;

  using SplitByPipeline = pipelines::log_message_organizer::SplitByPipeline;
  using OrganizeById = pipelines::log_message_organizer::OrganizeById;

  auto structure_messages = ParseInputFile(input_file, cli_args);

  if (structure_messages.empty()) {
    std::cerr << "No messages found in the input file." << std::endl;
    std::cerr << "Please check if the file is empty or try running the program "
                 "with the -v option."
              << std::endl;
    throw ApplicationRuntimeError("No messages found in the input file.");
  }

  auto messages_by_pipeline = SplitByPipeline(structure_messages).Split();

  for (const auto& [pipeline_id, messages] : messages_by_pipeline) {
    auto organized_messages = OrganizeById(messages).Organize();
    messages_by_pipeline[pipeline_id] = organized_messages;
  }

  OutputMessages(messages_by_pipeline, cli_args);
}

}  // namespace pipelines::app
/******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

int main(int argc, char* argv[]) {
  using namespace pipelines::app;

  auto [success, cli_args] = ParseCommandLineArguments(argc, argv);
  auto app_return_code = 0;

  if (success && !cli_args.help) {
    try {
      RunApplication(cli_args);
    } catch (const ApplicationRuntimeError& e) {
      std::cerr << "Application ended because: " << e.what() << std::endl;
      app_return_code = 1;
    } catch (const std::exception& e) {
      std::cerr << "Unexpected error: " << e.what() << std::endl;
      app_return_code = 1;
    } catch (...) {
      std::cerr << "Unknown error occurred." << std::endl;
      app_return_code = 1;
    }
  } else {
    if (!cli_args.help) {
      app_return_code = 1;
    }
  }

  return app_return_code;
}