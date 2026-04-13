#pragma once
#include <functional>
#include <map>
#include <queue>
#include <string>
#include <vector>

#include "../algorithm.hpp"
#include "core_export.hpp"
#include "structs/calendarService.hpp"

class CORE_API AStarTransfer : public Algorithm {
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

  struct Node {
    double fScore_;
    int transfers_;
    int arrivalTime_;
    int stationId_;

    bool operator>(const Node &other) const {
      if (fScore_ != other.fScore_) {
        return fScore_ > other.fScore_;
      }
      return arrivalTime_ > other.arrivalTime_;
    }
  };

  struct ParentInfo {
    int fromStation_;
    std::string tripId_;
    int departureTime_;
    int arrivalTime_;
  };

  using Graph = std::unordered_map<int, std::vector<Edge>>;
  using ArrivalMap = std::map<int, int>; // transfers -> minArrivalTime

  static constexpr double kTransferMultiplier =
      50000.0; // przesiadka jest warta 50000 m - 50 km
  static constexpr int kUnknownTime = -1;

  std::vector<AnalysedInfo> lastAnalysis_;

public:
  std::optional<PathData> findPath(const StationData &stationData,
                                   const TripData &tripData,
                                   const CalendarServiceData &calendarServiceData,
                                   const InputData &inputData) override;

  [[nodiscard]] const std::vector<AnalysedInfo> &getLastAnalysis() const override;

private:
  [[nodiscard]] double calculateHeuristic(const Station &s1, const Station &s2) const;

  // Sprawdza, czy mamy już lepszą trasę (mniej przesiadek i wcześniejszy
  // czas) do tej stacji
  bool isDominated(int stationId, int transfers, int time,
                   std::unordered_map<int, ArrivalMap> &bestArrivals) const;

  void processEdge(const Edge &edge, const Node &current, int departureTime,
                   int arrivalTime, const Station &endStation,
                   const StationData &stationData,
                   std::unordered_map<int, ArrivalMap> &bestArrivals,
                   std::unordered_map<int, ParentInfo> &parentMap,
                   std::priority_queue<Node, std::vector<Node>, std::greater<>> &queue);

  bool isBetterThanExisting(int transfers, int arrivalTime, ArrivalMap &arrivalMap) const;

  [[nodiscard]] Graph buildGraph(const TripData &tripData,
                                 const StationData &stationData) const;

  [[nodiscard]] PathData::PathDetails
  reconstructPath(const std::unordered_map<int, ParentInfo> &parentMap,
                  const StationData &stationData, int startId, int endId) const;
};