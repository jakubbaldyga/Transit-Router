#include "aStarTransfer.hpp"

std::optional<PathData>
AStarTransfer::findPath(const StationData &stationData, const TripData &tripData,
                        const CalendarServiceData &calendarServiceData,
                        const InputData &inputData) {
  const int startId = stationData.nameToStationId_.at(inputData.startStation_);
  const int endId = stationData.nameToStationId_.at(inputData.endStation_);
  const Station &endStation = stationData.stations_.at(endId);

  const Graph graph = buildGraph(tripData, stationData);

  std::priority_queue<Node, std::vector<Node>, std::greater<>> queue;
  std::unordered_map<int, ArrivalMap> bestArrivals;
  std::unordered_map<int, ParentInfo> parentMap;

  bestArrivals[startId][0] = inputData.startTime_;
  queue.push(
      {.fScore_ = calculateHeuristic(stationData.stations_.at(startId), endStation),
       .transfers_ = 0,
       .arrivalTime_ = inputData.startTime_,
       .stationId_ = startId});

  int finalArrival = kUnknownTime;

  while (!queue.empty()) {
    Node current = queue.top();
    queue.pop();

    if (current.stationId_ == endId) {
      finalArrival = current.arrivalTime_;
      break;
    }

    if (isDominated(current.stationId_, current.transfers_, current.arrivalTime_,
                    bestArrivals)) {
      continue;
    }

    if (!graph.contains(current.stationId_)) {
      continue;
    }

    for (const auto &edge : graph.at(current.stationId_)) {
      auto [departureTime, arrivalTime] = edge.nextDeparture(current.arrivalTime_);

      if (bestArrivals.contains(edge.to_) &&
          arrivalTime >= bestArrivals[edge.to_].begin()->second) {
        continue;
      }

      if (!calendarServiceData.services_.at(tripData.trips_.at(edge.tripId_).serviceId_)
               .isTripActive(inputData.startDate_ + departureTime / kDaySeconds)) {
        continue;
      }
      lastAnalysis_.push_back({.from_ = current.stationId_, .to_ = edge.to_});

      processEdge(edge, current, departureTime, arrivalTime, endStation, stationData,
                  bestArrivals, parentMap, queue);
    }
  }

  if (finalArrival == kUnknownTime) {
    return std::nullopt;
  }

  return {{inputData.startDate_, inputData.startTime_, finalArrival,
           reconstructPath(parentMap, stationData, startId, endId)}};
}

const std::vector<AnalysedInfo> &AStarTransfer::getLastAnalysis() const {
  return lastAnalysis_;
}

double AStarTransfer::calculateHeuristic(const Station &s1, const Station &s2) const {
  return s1.distTo(s2);
}

bool AStarTransfer::isDominated(int stationId, int transfers, int time,
                                std::unordered_map<int, ArrivalMap> &bestArrivals) const {
  if (!bestArrivals.contains(stationId)) {
    return false;
  }

  for (const auto &[prevTransfers, prevTime] : bestArrivals[stationId]) {
    if (prevTransfers <= transfers && prevTime < time) {
      return true;
    }
  }
  return false;
}

void AStarTransfer::processEdge(
    const Edge &edge, const Node &current, int departureTime, int arrivalTime,
    const Station &endStation, const StationData &stationData,
    std::unordered_map<int, ArrivalMap> &bestArrivals,
    std::unordered_map<int, ParentInfo> &parentMap,
    std::priority_queue<Node, std::vector<Node>, std::greater<>> &queue) {
  int newTransfers = current.transfers_ + 1;
  auto &arrivalMap = bestArrivals[edge.to_];

  if (isBetterThanExisting(newTransfers, arrivalTime, arrivalMap)) {
    arrivalMap[newTransfers] = arrivalTime;
    parentMap[edge.to_] = {.fromStation_ = current.stationId_,
                           .tripId_ = edge.tripId_,
                           .departureTime_ = departureTime,
                           .arrivalTime_ = arrivalTime};

    double h = calculateHeuristic(stationData.stations_.at(edge.to_), endStation);
    queue.push({.fScore_ = newTransfers * kTransferMultiplier + h,
                .transfers_ = newTransfers,
                .arrivalTime_ = arrivalTime,
                .stationId_ = edge.to_});
  }
}

bool AStarTransfer::isBetterThanExisting(int transfers, int arrivalTime,
                                         ArrivalMap &arrivalMap) const {
  for (auto it = arrivalMap.begin();
       it != arrivalMap.end();) { // usuwamy wszystkie trasy, które są gorsze od nowej
                                  // (Pareto-frontier)
    if (it->first <= transfers && it->second <= arrivalTime) {
      return false;
    }

    if (it->first >= transfers && it->second >= arrivalTime) {
      it = arrivalMap.erase(it);
    } else {
      ++it;
    }
  }
  return true;
}

AStarTransfer::Graph AStarTransfer::buildGraph(const TripData &tripData,
                                               const StationData &stationData) const {
  Graph graph;
  for (const auto &[lineId, trip] : tripData.trips_) {
    if (trip.stations_.size() < 2) {
      continue;
    }

    for (auto fromIt = trip.stations_.begin(); fromIt != std::prev(trip.stations_.end());
         ++fromIt) {
      int fromParent = stationData.parentId_.at(fromIt->stationId_);
      for (auto toIt = std::next(fromIt); toIt != trip.stations_.end(); ++toIt) {
        graph[fromParent].push_back({.to_ = stationData.parentId_.at(toIt->stationId_),
                                     .departure_ = fromIt->departure_,
                                     .arrival_ = toIt->arrival_,
                                     .tripId_ = lineId});
      }
    }
  }
  return graph;
}

auto AStarTransfer::reconstructPath(const std::unordered_map<int, ParentInfo> &parentMap,
                                    const StationData &stationData, int startId,
                                    int endId) const -> PathData::PathDetails {
  PathData::PathDetails path;
  int curr = endId;

  while (curr != startId) {
    const auto &info = parentMap.at(curr);
    path.emplace_back(info.tripId_, info.departureTime_, info.arrivalTime_,
                      stationData.idToStationName_.at(info.fromStation_),
                      stationData.idToStationName_.at(curr));
    curr = info.fromStation_;
  }

  std::ranges::reverse(path);
  return path;
}
