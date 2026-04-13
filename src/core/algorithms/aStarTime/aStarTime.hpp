#pragma once
#include "../algorithm.hpp"
#include "core_export.hpp"

class CORE_API AStarTime : public Algorithm {
  struct Edge {
    int to_;
    int departureTime_;
    int arrivalTime_;
    std::string tripId_;

    [[nodiscard]] std::pair<int, int> nextDeparture(int currentTime) const {
      int dayOffset = currentTime / kDaySeconds;
      int dep = departureTime_ + dayOffset * kDaySeconds;
      int arr = arrivalTime_ + dayOffset * kDaySeconds;

      if (dep < currentTime) { // jeśli kurs już odjechał, przesuwamy o kolejny dzień
        dep += kDaySeconds;
        arr += kDaySeconds;
      }

      return {dep, arr};
    }
  };
  using Graph = std::unordered_map<int, std::vector<Edge>>;

  struct ParentInfo {
    int fromStation_;
    std::string tripId_;
    int departureTime_;
    int arrivalTime_;
  };

  std::vector<AnalysedInfo> lastAnalysis_;

  static constexpr double kAvgTransferSpeedSq = (52.0 / 3.6);

  double calculateHeuristicCost(const Station &s1, const Station &s2);

public:
  std::optional<PathData> findPath(const StationData &stationData,
                                   const TripData &tripData,
                                   const CalendarServiceData &calendarServiceData,
                                   const InputData &inputData) override;

  [[nodiscard]] const std::vector<AnalysedInfo> &getLastAnalysis() const override;

private:
  Graph buildGraph(const TripData &tripData, const StationData &stationData);

  PathData::PathDetails reconstructPath(const std::unordered_map<int, ParentInfo> &parent,
                                        const StationData &stationData, int startId,
                                        int endId);
};