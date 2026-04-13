#include "Parser.hpp"

#include <utility>

Parser::Parser(std::string programName, std::string programVersion)
    : argparse::ArgumentParser(std::move(programName), std::move(programVersion)) {
  subCommands_ = std::vector<std::unique_ptr<SubCommand>>();

  for (auto &subCommand : subCommands_) {
    add_subparser(*subCommand);
  }
}

void Parser::parseArgs(const std::vector<std::string> &arguments) { // NOLINT(modernize-avoid-c-arrays)
  try {
    ArgumentParser::parse_args(arguments);
  } catch (const std::exception &err) {
    std::cerr << err.what() << "\n";
    std::cerr << *this;
    return;
  }
  for (auto &subCommand : subCommands_) {
    if (is_subcommand_used(*subCommand)) {
      subCommand->doCommand();
    }
  }
}
