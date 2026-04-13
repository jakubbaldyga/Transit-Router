#pragma once

#include <argparse/argparse.hpp>
#include <memory>
#include <string>
#include <vector>

#include "../SubCommand/SubCommand.hpp"

class Parser : public argparse::ArgumentParser {
  std::vector<std::unique_ptr<SubCommand>> subCommands_;

public:
  Parser(std::string programName, std::string programVersion);

  template <typename T>
    requires std::is_base_of_v<SubCommand, T>
  void addSubCommand() {
    subCommands_.push_back(std::make_unique<T>());
    add_subparser(*subCommands_.back());
  }

  void parseArgs(const std::vector<std::string> &arguments); 
};