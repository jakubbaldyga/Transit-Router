#include <benchmark/benchmark.h>

#include <filesystem>
#include <iostream>

#include "algorithms/aStarTime/aStarTime.hpp"
#include "algorithms/aStarTransfer/aStarTransfer.hpp"
#include "algorithms/dijkistra/dijkistra.hpp"
#include "dataLoader/calendarLoader.hpp"
#include "dataLoader/stationLoader.hpp"
#include "dataLoader/tripLoader.hpp"
#include "utils.hpp"

static TripData tripData;
static StationData stationData;
static InputData inputData;
static CalendarServiceData calendarServiceData;

static void BM_Dijikistra(benchmark::State &state) {
  for (auto _ : state) {
    Dijikistra d;
    auto result = d.findPath(stationData, tripData, calendarServiceData, inputData);
    benchmark::DoNotOptimize(result);
  }
}

static void BM_AStarTime(benchmark::State &state) {
  for (auto _ : state) {
    AStarTime d;
    auto result = d.findPath(stationData, tripData, calendarServiceData, inputData);
    benchmark::DoNotOptimize(result);
  }
}

static void BM_AStarTransfers(benchmark::State &state) {
  for (auto _ : state) {
    AStarTransfer d;
    auto result = d.findPath(stationData, tripData, calendarServiceData, inputData);
    benchmark::DoNotOptimize(result);
  }
}

static void globalSetup() {
  std::filesystem::path dataPath(getGtfsDataPathFromEnvOrDefault());
  const auto stopsPath = dataPath / "stops.txt";
  const auto stopTimesPath = dataPath / "stop_times.txt";
  const auto calendarPath = dataPath / "calendar.txt";
  const auto calendarDatesPath = dataPath / "calendar_dates.txt";

  if (!std::filesystem::exists(stopsPath) || !std::filesystem::exists(stopTimesPath) ||
      !std::filesystem::exists(calendarPath) ||
      !std::filesystem::exists(calendarDatesPath)) {
    std::cerr << "Invalid GTFS data path: " << dataPath.string() << "\n"
              << "Required files: stops.txt, stop_times.txt, calendar.txt, "
              << "calendar_dates.txt\n";
    std::exit(1);
  }

  tripData = retrieveTrips(stopTimesPath.string());
  stationData = retrieveStations(stopsPath.string());
  calendarServiceData =
      retrieveCalendarServiceData(calendarPath.string(), calendarDatesPath.string());

  inputData.startStation_ = "Vratislavice nad Nisou";
  inputData.endStation_ = "Bautzen";
  inputData.startTime_ = parseTime("05:36:00");
}

int main(int argc, char **argv) {
  globalSetup();

  benchmark::Initialize(&argc, argv);

  BENCHMARK(BM_Dijikistra);
  BENCHMARK(BM_AStarTime);

  BENCHMARK(BM_AStarTransfers);
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return 0;
}
