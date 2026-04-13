#include <iostream>
#include <string>
#include <vector>

// define this before including windows.h to avoid conflicts with min/max macros
#ifndef NOMINMAX
#define NOMINMAX
#endif
// makes program lighter and faster, since we dont need all the windows features
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// clang-format off
//order matters here, we need windows.h before anything else
#include <windows.h> 
#include <shellapi.h>
// clang-format on

#include "Parser/Parser.hpp"
#include "SubCommand/routeSubCommand.hpp"
#include "SubCommand/tabuSubCommand.hpp"

#include <spdlog/spdlog.h>

std::vector<std::string> getUtf8Args() {
  std::vector<std::string> args;
  int wargc;

  LPWSTR *wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);

  if (wargv != nullptr) {
    for (int i = 0; i < wargc; ++i) {
      int sizeNeeded =
          WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, nullptr, 0, nullptr, nullptr);
      std::string utf8Arg(sizeNeeded, 0);

      WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, &utf8Arg[0], sizeNeeded, nullptr,
                          nullptr);

      utf8Arg.pop_back(); // remove trailing \0
      args.push_back(utf8Arg);
    }
    LocalFree(wargv);
  }

  return args;
}

int main() {
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);

  Parser program("transit", "Program to find the best routes between stations");
  program.addSubCommand<RouteSubCommand>();
  program.addSubCommand<TabuSubCommand>();

  try {
    // Przekazujemy argumenty wprost z naszej windowsowej funkcji
    program.parseArgs(getUtf8Args());
  } catch (const std::exception &err) {
    std::cerr << err.what() << "\n";
    return 1;
  }

  return 0;
}