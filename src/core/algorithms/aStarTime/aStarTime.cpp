#include "aStarTime.hpp"

#include <algorithm>
#include <optional>

double AStarTime::calculateHeuristicCost(const Station &s1, const Station &s2) {
  // we need to somehow convert it into the time cost
  // so we need to divide by the speed - I assumed the avg_sped in m/s
  return s1.distTo(s2) / kAvgTransferSpeedSq;
}

std::optional<PathData>
AStarTime::findPath(const StationData &stationData, const TripData &tripData,
                    const CalendarServiceData &calendarServiceData,
                    const InputData &inputData) {
  int startId = stationData.nameToStationId_.at(inputData.startStation_);
  int endId = stationData.nameToStationId_.at(inputData.endStation_);

  AStarTime::Graph graph = buildGraph(tripData, stationData);

  const Station &endStation = stationData.stations_.at(endId);

  using State = std::pair<double, int>; // (f cost, stationId)
  std::priority_queue<State, std::vector<State>, std::greater<>> stations;
  std::unordered_map<int, int> bestTime;
  std::unordered_map<int, ParentInfo> parent;

  bestTime[startId] = inputData.startTime_;
  stations.emplace(0, startId);

  while (!stations.empty()) {
    auto [f, station] = stations.top();
    stations.pop();

    if (station == endId) {
      break;
    }
    if (!graph.contains(station)) {
      continue;
    }

    for (const auto &edge : graph.at(station)) {
      auto [dep, arr] = edge.nextDeparture(bestTime[station]);

      if (bestTime.contains(edge.to_) && arr >= bestTime[edge.to_]) {
        continue;
      }

      if (!calendarServiceData.services_.at(tripData.trips_.at(edge.tripId_).serviceId_)
               .isTripActive(inputData.startDate_ + dep / kDaySeconds)) {
        continue;
      }

      if (!bestTime.contains(edge.to_) || arr < bestTime[edge.to_]) {
        lastAnalysis_.push_back({.from_ = station, .to_ = edge.to_});

        bestTime[edge.to_] = arr;
        parent[edge.to_] = {.fromStation_ = station,
                            .tripId_ = edge.tripId_,
                            .departureTime_ = dep,
                            .arrivalTime_ = arr};
        double hCost =
            calculateHeuristicCost(stationData.stations_.at(edge.to_), endStation);
        stations.emplace(arr - inputData.startTime_ + hCost, edge.to_);
      }
    }
  }

  if (!bestTime.contains(endId)) {
    return std::nullopt;
  }

  auto path = reconstructPath(parent, stationData, startId, endId);
  return {{inputData.startDate_, inputData.startTime_, bestTime[endId], path}};
}

const std::vector<AnalysedInfo> &AStarTime::getLastAnalysis() const {
  return lastAnalysis_;
}

AStarTime::Graph AStarTime::buildGraph(const TripData &tripData,
                                       const StationData &stationData) {
  AStarTime::Graph graph;
  for (const auto &[lineId, trip] : tripData.trips_) {
    if (trip.stations_.size() < 2) {
      continue;
    }
    for (auto from = trip.stations_.begin(); from != std::prev(trip.stations_.end());
         ++from) {
      for (auto to = std::next(from); to != trip.stations_.end(); ++to) {
        graph[stationData.parentId_.at(from->stationId_)].push_back(
            {.to_ = stationData.parentId_.at(to->stationId_),
             .departureTime_ = from->departure_,
             .arrivalTime_ = to->arrival_,
             .tripId_ = lineId});
      }
    }
  }
  return graph;
}

PathData::PathDetails
AStarTime::reconstructPath(const std::unordered_map<int, ParentInfo> &parent,
                           const StationData &stationData, int startId, int endId) {
  PathData::PathDetails path;
  int curr = endId;
  while (curr != startId) {
    auto [prev, line, dep, arr] = parent.at(curr);
    path.emplace_back(line, dep, arr, stationData.idToStationName_.at(prev),
                      stationData.idToStationName_.at(curr));
    curr = prev;
  }
  std::ranges::reverse(path);
  return path;
}
