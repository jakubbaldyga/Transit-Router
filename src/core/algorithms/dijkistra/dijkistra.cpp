#include "dijkistra.hpp"

#include <algorithm>

std::optional<PathData>
Dijikistra::findPath(const StationData &stationData, const TripData &tripData,
                     const CalendarServiceData &calendarServiceData,
                     const InputData &inputData) {
  int startId = stationData.nameToStationId_.at(inputData.startStation_);
  int endId = stationData.nameToStationId_.at(inputData.endStation_);

  Graph graph = buildGraph(tripData, stationData);

  auto [bestTime, parent] =
      runDijkstra(graph, startId, inputData.startTime_, endId, inputData.startDate_,
                  calendarServiceData, tripData);

  if (!bestTime.count(endId)) {
    return std::nullopt;
  }

  auto path = reconstructPath(parent, stationData, startId, endId);

  return std::make_optional(
      PathData(inputData.startDate_, inputData.startTime_, bestTime[endId], path));
}

const std::vector<AnalysedInfo> &Dijikistra::getLastAnalysis() const {
  return lastAnalysis_;
}

Dijikistra::Graph Dijikistra::buildGraph(const TripData &tripData,
                                         const StationData &stationData) {
  Dijikistra::Graph graph;
  for (const auto &[tripdId, trip] : tripData.trips_) {
    if (trip.stations_.size() < 2) {
      continue;
    }

    for (auto from = trip.stations_.begin(); from != std::prev(trip.stations_.end());
         ++from) {
      for (auto to = std::next(from); to != trip.stations_.end(); ++to) {
        graph[stationData.parentId_.at(from->stationId_)].push_back(
            Edge{.to_ = stationData.parentId_.at(to->stationId_),
                 .departure_ = from->departure_,
                 .arrival_ = to->arrival_,
                 .tripId_ = tripdId});
      }
    }
  }
  return graph;
}

std::pair<std::unordered_map<int, int>, std::unordered_map<int, Dijikistra::ParentInfo>>
Dijikistra::runDijkstra(const Graph &graph, int startId, int startTime, int endId,
                        int queryDate, const CalendarServiceData &calendarServiceData,
                        const TripData &tripData) {
  using State = std::pair<int, int>; // (time, stationId)

  std::priority_queue<State, std::vector<State>, std::greater<>> stations;

  std::unordered_map<int, int> bestTime;      // stationId - time
  std::unordered_map<int, ParentInfo> parent; // stationId - (stationId from, lineId)

  lastAnalysis_.clear();

  stations.emplace(startTime, startId);
  bestTime[startId] = startTime;

  while (!stations.empty()) {
    auto [currentTime, station] = stations.top();
    stations.pop();

    if (station == endId) {
      break;
    }
    if (currentTime > bestTime[station]) {
      continue;
    }
    if (!graph.contains(station)) {
      continue;
    }

    for (const auto &edge : graph.at(station)) {
      auto [dep, arr] = edge.nextDeparture(currentTime);

      if (bestTime.contains(edge.to_) && arr >= bestTime[edge.to_]) {
        continue;
      }

      if (!calendarServiceData.services_.at(tripData.trips_.at(edge.tripId_).serviceId_)
               .isTripActive(queryDate + dep / kDaySeconds)) {
        continue;
      }

      if (!bestTime.contains(edge.to_) || arr < bestTime[edge.to_]) {
        lastAnalysis_.push_back({.from_ = station, .to_ = edge.to_});

        bestTime[edge.to_] = arr;
        parent[edge.to_] = {.from_ = station,
                            .departure_ = dep,
                            .arrival_ = arr,
                            .tripId_ = edge.tripId_};
        stations.emplace(arr, edge.to_);
      }
    }
  }

  return {bestTime, parent};
}

PathData::PathDetails
Dijikistra::reconstructPath(const std::unordered_map<int, ParentInfo> &parent,
                            const StationData &stationData, int startId, int endId) {
  PathData::PathDetails path;
  int curr = endId;
  while (curr != startId) {
    auto edge = parent.at(curr);
    path.emplace_back(edge.tripId_, edge.departure_, edge.arrival_,
                      stationData.idToStationName_.at(edge.from_),
                      stationData.idToStationName_.at(curr));
    curr = edge.from_;
  }
  std::ranges::reverse(path);
  return path;
}
