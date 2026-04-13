#pragma once
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "core_export.hpp"
#include "structs/calendarService.hpp"
#include "structs/station.hpp"
#include "structs/trip.hpp"

// return data of all algorithms
struct CORE_API PathData {
  int startDate_;
  int startTime_;
  int endTime_;

  struct PathEntry {
    std::string lineId_;

    int timeFrom_;
    int timeTo_;

    std::string stationFrom_;

    std::string stationTo_;
  };

  using PathDetails = std::vector<PathEntry>;
  PathDetails stations_; // <line name, station name>

  PathData(int startDate, int startTime, int endTime, PathDetails stations);

  void printResult() const;
};

struct AnalysedInfo {
  int from_;
  int to_;
};

struct CORE_API InputData {
  std::string startStation_;
  std::string endStation_;

  int startTime_;
  int startDate_;

  InputData() = default;
  InputData(const std::string &startStation, const std::string &endStation, int startTime,
            int startDate = 0);
};

// since we cant do a virtual function, we will wrap it in the class
class CORE_API Algorithm {
public:
  virtual std::optional<PathData> findPath(const StationData &stationData,
                                           const TripData &tripData,
                                           const CalendarServiceData &calendarServiceData,
                                           const InputData &inputData) = 0;

  [[nodiscard]] virtual const std::vector<AnalysedInfo> &getLastAnalysis() const = 0;
  virtual ~Algorithm() = default;
};

struct CORE_API InputDataTabu {
  std::string startStation_;
  std::vector<std::string> stations_;

  int startTime_;
  int startDate_;
  std::unique_ptr<Algorithm> algorithm_;
};

class CORE_API AlgorithmMany {
public:
  virtual std::optional<PathData> findPath(const StationData &stationData,
                                           const TripData &tripData,
                                           const CalendarServiceData &calendarServiceData,
                                           const InputDataTabu &inputData) = 0;
};