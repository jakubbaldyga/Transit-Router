#pragma once

#include <argparse/argparse.hpp>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

#include "SubCommand.hpp"
#include "algorithms/aStarTime/aStarTime.hpp"
#include "algorithms/aStarTransfer/aStarTransfer.hpp"
#include "algorithms/algorithm.hpp"
#include "algorithms/dijkistra/dijkistra.hpp"
#include "dataLoader/calendarLoader.hpp"
#include "dataLoader/stationLoader.hpp"
#include "dataLoader/tripLoader.hpp"

#include <spdlog/spdlog.h>

class RouteSubCommand : public SubCommand {
  static constexpr std::string_view kDijkstra = "dijkstra";
  static constexpr std::string_view kAStarTime = "astar_time";
  static constexpr std::string_view kAStarTransfer = "astar_transfer";

  std::unordered_map<std::string, std::unique_ptr<Algorithm>> algorithms_;

public:
  RouteSubCommand() : SubCommand("route", "Find the best route between two stations") {

    std::string defaultDataPath = "google_transit";
    if (const char *envPath = std::getenv("GTFS_DATA_PATH");
        envPath != nullptr && envPath[0] != '\0') {
      defaultDataPath = envPath;
    }

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
    add_argument("--endStation").help("Name of the destination station").required();
    add_argument("--startTime").help("Start time in HH:MM:SS format").required();
    add_argument("--startDate").help("Start date in DD.MM.YYYY format").required();
    add_argument("--dataPath")
        .help("Path to GTFS directory (stops.txt, stop_times.txt, calendar*.txt)")
        .default_value(defaultDataPath);
    add_argument("-v", "--verbose")
        .default_value(false)
        .implicit_value(true)
        .help("Enable verbose logging");
  }

  void doCommand() override {
    try {
      std::string algorithm = get("algorithm");
      std::string startStation = get("startStation");
      std::string endStation = get("endStation");
      std::string startTimeStr = get("startTime");
      std::string startDateStr = get("startDate");
      std::string dataPathStr = get("dataPath");
      bool verbose = get<bool>("verbose");

      if (verbose) {
        spdlog::set_level(spdlog::level::info);
      } else {
        spdlog::set_level(spdlog::level::err);
      }

      spdlog::info("Finding route from {} to {} using {} starting at {}", startStation,
                   endStation, algorithm, startTimeStr);

      int startTime = parseTime(startTimeStr);
      int startDate = parseDate(startDateStr);
      InputData inputData(startStation, endStation, startTime, startDate);

      std::filesystem::path dataPath(dataPathStr);
      const auto stopsPath = dataPath / "stops.txt";
      const auto stopTimesPath = dataPath / "stop_times.txt";
      const auto calendarPath = dataPath / "calendar.txt";
      const auto calendarDatesPath = dataPath / "calendar_dates.txt";

      if (!std::filesystem::exists(stopsPath) ||
          !std::filesystem::exists(stopTimesPath) ||
          !std::filesystem::exists(calendarPath) ||
          !std::filesystem::exists(calendarDatesPath)) {
        spdlog::error("Invalid GTFS data path: {}", dataPath.string());
        spdlog::error("Required files: stops.txt, stop_times.txt, calendar.txt, "
                      "calendar_dates.txt");
        return;
      }

      StationData stationData = retrieveStations(stopsPath.string());
      TripData tripData = retrieveTrips(stopTimesPath.string());
      CalendarServiceData calendarServiceData =
          retrieveCalendarServiceData(calendarPath.string(), calendarDatesPath.string());

      if (!algorithms_.contains(algorithm)) {
        spdlog::error("Unknown algorithm: {}", algorithm);
        return;
      }
      if (stationData.nameToStationId_.contains(startStation) == 0) {
        spdlog::error("Unknown start station: {}", startStation);
        return;
      }
      if (stationData.nameToStationId_.contains(endStation) == 0) {
        spdlog::error("Unknown end station: {}", endStation);
        return;
      }

      const auto start = std::chrono::steady_clock::now();

      auto result = algorithms_[algorithm]->findPath(stationData, tripData,
                                                     calendarServiceData, inputData);

      const auto end = std::chrono::steady_clock::now();
      const auto elapsedMs =
          std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

      spdlog::info("Route computation finished in {} ms", elapsedMs);

      if (result) {
        result.value().printResult();
      } else {
        spdlog::info("No route found");
      }
    } catch (const std::exception &e) {
      spdlog::error("Route command failed: {}", e.what());
    } catch (...) {
      spdlog::error("Route command failed: unknown error");
    }
  }
};