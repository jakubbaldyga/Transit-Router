#pragma once

#include <argparse/argparse.hpp>
#include <memory>
#include <string>
#include <unordered_map>

#include "SubCommand.hpp"
#include "algorithms/aStarTime/aStarTime.hpp"
#include "algorithms/aStarTransfer/aStarTransfer.hpp"
#include "algorithms/algorithm.hpp"
#include "algorithms/dijkistra/dijkistra.hpp"
#include "algorithms/tabuSearch/tabuSearch.hpp"
#include "dataLoader/calendarLoader.hpp"
#include "dataLoader/stationLoader.hpp"
#include "dataLoader/tripLoader.hpp"
#include "fmt/base.h"
#include "spdlog/fmt/bundled/base.h"
#include "spdlog/spdlog.h"
#include "structs/calendarService.hpp"
#include <fmt/ranges.h>

class TabuSubCommand : public SubCommand {
  static constexpr std::string_view kDijkstra = "dijkstra";
  static constexpr std::string_view kAStarTime = "astar_time";
  static constexpr std::string_view kAStarTransfer = "astar_transfer";

  std::unordered_map<std::string, std::unique_ptr<Algorithm>> algorithms_;
  std::unique_ptr<TabuSearch> tabuSearch_;

public:
  TabuSubCommand()
      : SubCommand("tabu", "Find the best route between many stations, starting and "
                           "ending at the first station ") {

    algorithms_.emplace(kDijkstra, std::make_unique<Dijikistra>());
    algorithms_.emplace(kAStarTime, std::make_unique<AStarTime>());
    algorithms_.emplace(kAStarTransfer, std::make_unique<AStarTransfer>());

    argparse::Argument &argument = add_argument("algorithm")
                                       .help("Algorithm to use (dijkstra, astar_time, "
                                             "astar_transfer)")
                                       .required()
                                       .default_value(std::string("dijkstra"));
    argument.add_choice(kDijkstra);
    argument.add_choice(kAStarTime);
    argument.add_choice(kAStarTransfer);

    add_argument("--startStation").help("Name of the starting station").required();
    add_argument("--stations")
        .help("Stations")
        .nargs(argparse::nargs_pattern::at_least_one);
    add_argument("--startTime").help("Start time in HH:MM:SS format").required();
    add_argument("--startDate").help("Start date in DD.MM.YYYY format").required();
  }

  void doCommand() override {
    std::string algorithm = get("algorithm");
    std::string startStation = get("startStation");
    auto stations = get<std::vector<std::string>>("--stations");
    std::string startTimeStr = get("startTime");
    std::string startDateStr = get("startDate");

    // add log
    spdlog::info("Finding route from {} to stations{} using {} starting at {}",
                 startStation, fmt::join(stations, ", "), algorithm, startTimeStr);

    int startTime = parseTime(startTimeStr);
    int startDate = parseDate(startDateStr);
    InputDataTabu inputData(startStation, stations, startTime, startDate);

    StationData stationData = retrieveStations("google_transit/stops.txt");
    spdlog::debug("Retrieved station data");
    TripData tripData = retrieveTrips("google_transit/stop_times.txt");
    spdlog::debug("Retrieved trip data");
    CalendarServiceData calendarServiceData = retrieveCalendarServiceData(
        "google_transit/calendar.txt", "google_transit/calendar_dates.txt");
    spdlog::debug("Retrieved calendar service data");

    if (!algorithms_.contains(algorithm)) {
      spdlog::error("Unknown algorithm: {}", algorithm);
      return;
    } else {
      tabuSearch_ = std::make_unique<TabuSearch>(std::move(algorithms_[algorithm]));

      const auto start = std::chrono::steady_clock::now();

      auto result =
          tabuSearch_->findPath(stationData, tripData, calendarServiceData, inputData);

      const auto end = std::chrono::steady_clock::now();
      const auto elapsedMs =
          std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

      spdlog::info("Tabu computation finished in {} ms", elapsedMs);

      if (result) {
        result.value().printResult();
      } else {
        spdlog::info("No route found");
      }
    }
  }
};