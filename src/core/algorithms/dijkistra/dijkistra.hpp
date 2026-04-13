#pragma once
#include "../algorithm.hpp"

class CORE_API Dijikistra : public Algorithm {
public:
  struct ParentInfo {
    int from_;
    int departure_;
    int arrival_;
    std::string tripId_;
  };

private:
  struct Edge {
    int to_;
    int departure_;
    int arrival_;
    std::string tripId_;

    [[nodiscard]] std::pair<int, int> nextDeparture(int currentTime) const {
      int dayOffset = currentTime / kDaySeconds;
      int dep = departure_ + dayOffset * kDaySeconds;
      int arr = arrival_ + dayOffset * kDaySeconds;

      if (dep < currentTime) { // jeśli kurs już odjechał, przesuwamy o kolejny dzień
        dep += kDaySeconds;
        arr += kDaySeconds;
      }

      return {dep, arr};
    }
  };

  using Graph = std::unordered_map<int, std::vector<Edge>>;

public:
  std::optional<PathData> findPath(const StationData &stationData,
                                   const TripData &tripData,
                                   const CalendarServiceData &calendarServiceData,
                                   const InputData &inputData) override;

  [[nodiscard]] const std::vector<AnalysedInfo> &getLastAnalysis() const override;

private:
  Graph buildGraph(const TripData &tripData, const StationData &stationData);

  std::pair<std::unordered_map<int, int>, std::unordered_map<int, ParentInfo>>
  runDijkstra(const Graph &graph, int startId, int startTime, int endId, int queryDate,
              const CalendarServiceData &calendarServiceData, const TripData &tripData);

  PathData::PathDetails reconstructPath(const std::unordered_map<int, ParentInfo> &parent,
                                        const StationData &stationData, int startId,
                                        int endId);

  std::vector<AnalysedInfo> lastAnalysis_;
};