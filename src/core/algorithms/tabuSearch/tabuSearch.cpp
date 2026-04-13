#include "tabuSearch.hpp"
#include "algorithms/algorithm.hpp"

#include <memory>
#include <utility>

TabuSearch::TabuMemory::TabuMemory(int size) : maxSize_(size) {}

bool TabuSearch::TabuMemory::contains(const SwapEntry &move) const {
  return tabuSet_.contains(move);
}

void TabuSearch::TabuMemory::insert(const SwapEntry &move, int step) {
  tabuSet_.insert(move);
  tabuQueue_.push(move);
  lastUsed_[move] = step;

  if (std::cmp_greater(tabuQueue_.size(), maxSize_)) {
    auto oldest = tabuQueue_.front();
    tabuQueue_.pop();
    tabuSet_.erase(oldest);
  }
}

bool TabuSearch::TabuMemory::isAspired(const SwapEntry &move, int step, int neighborCost,
                                       int bestCost) const {
  int last = lastUsed_.contains(move) ? lastUsed_.at(move) : 0;
  double aspirationThreshold = neighborCost + kAspirationEps * (step - last);
  return aspirationThreshold < bestCost;
}

int TabuSearch::stationDistance(int from, int to, int time, int date,
                                const StationData &stationData, const TripData &tripData,
                                const CalendarServiceData &calendarServiceData) {
  CacheKey key{.from_ = from, .to_ = to, .time_ = time};

  if (distanceCache_.contains(key)) {
    return distanceCache_[key];
  }

  auto result = algorithm_->findPath(stationData, tripData, calendarServiceData,
                                     {stationData.idToStationName_.at(from),
                                      stationData.idToStationName_.at(to), time, date});

  int cost;
  if(result.has_value()) {
    cost = result->endTime_ - result->startTime_;
  } else {
    cost = INT_MAX / 2;
  }

  distanceCache_[key] = cost;
  return cost;
}

int TabuSearch::totalCost(const std::vector<int> &stations, int startId, int startTime,
                          int date, const StationData &stationData,
                          const TripData &tripData,
                          const CalendarServiceData &calendarServiceData) {
  int cost = 0;
  int curr = startId;
  int currTime = startTime;
  for (int next : stations) {
    int c = stationDistance(curr, next, currTime, date, stationData, tripData,
                            calendarServiceData);

    if (c >= INT_MAX / 2) {
      return INT_MAX / 2;
    }
    currTime += c;
    cost += c;
    curr = next;
  }
  int c = stationDistance(curr, startId, currTime, date, stationData, tripData,
                          calendarServiceData);
  return c >= INT_MAX / 2 ? INT_MAX / 2 : cost + c;
}

std::vector<TabuSearch::TabuMemory::SwapEntry> TabuSearch::sampleNeighborhood(int n) {
  int maxPairs = (n * (n - 1)) / 2;
  if (maxPairs <= 0) {
    return {};
  }

  int sampleSize = std::max(1, n);

  sampleSize = std::min(sampleSize, maxPairs);

  std::vector<TabuMemory::SwapEntry> samples;
  std::set<std::pair<int, int>> usedPairs;

  while (std::cmp_less(samples.size(), sampleSize)) {
    int i = getRandom(0, n - 1);
    int j = getRandom(0, n - 1);
    if (i == j) {
      continue;
    }
    if (i > j) {
      std::swap(i, j);
    }

    std::pair<int, int> move = {i, j};
    if (!usedPairs.contains(move)) {
      usedPairs.insert(move);
      samples.push_back(move);
    }
  }
  return samples;
}

std::optional<PathData>
TabuSearch::reconstructFullPath(const std::vector<int> &perm, int startId, int startTime,
                                int startDate, const StationData &stationData,
                                const TripData &tripData,
                                const CalendarServiceData &calendarServiceData) {
  PathData::PathDetails details;
  int curr = startId;
  int currTime = startTime;
  bool hasError = false;

  auto addLeg = [&](int from, int to) {
    InputData input;
    input.startStation_ = stationData.idToStationName_.at(from);
    input.endStation_ = stationData.idToStationName_.at(to);
    input.startTime_ = currTime;
    input.startDate_ = startDate;
    auto result = algorithm_->findPath(stationData, tripData, calendarServiceData, input);
    if (!result.has_value()) {
      hasError = true;
      return;
    }
    for (const auto &entry : result->stations_) {
      details.push_back(entry);
    }
    currTime = result->endTime_;
  };

  for (int next : perm) {
    addLeg(curr, next);
    if (hasError) {
      return std::nullopt;
    }
    curr = next;
  }
  addLeg(curr, startId);
  if (hasError) {
    return std::nullopt;
  }

  return std::make_optional(PathData(startDate, startTime, currTime, details));
}

std::vector<int> TabuSearch::getShuffledStationIds(const StationData &stationData,
                                                   const InputDataTabu &inputData) {
  std::vector<int> stationIds(inputData.stations_.size());
  for (int i = 0; std::cmp_less(i, inputData.stations_.size()); i++) {
    stationIds[i] = stationData.nameToStationId_.at(inputData.stations_[i]);
  }

  return shuffleVector(stationIds);
}

std::optional<PathData>
TabuSearch::findPath(const StationData &stationData, const TripData &tripData,
                     const CalendarServiceData &calendarServiceData,
                     const InputDataTabu &inputData) {
  distanceCache_.clear();

  int startId = stationData.nameToStationId_.at(inputData.startStation_);
  std::vector<int> current = getShuffledStationIds(stationData, inputData);

  int currentCost =
      totalCost(current, startId, inputData.startTime_, inputData.startDate_, stationData,
                tripData, calendarServiceData);

  std::vector<int> bestOrder = current;
  int bestCost = currentCost;

  TabuMemory tabuMemory(static_cast<int>(current.size()) * kTabuSizeMultiplier);

  for (int step = 0; step < kStepLimit; step++) {
    std::vector<int> bestNeighbors;
    std::pair<int, int> bestMove = {-1, -1};
    int bestNeighborCost = INT_MAX;

    for (const auto &[i, j] : sampleNeighborhood(static_cast<int>(current.size()))) {
      auto neighbors = current;
      std::swap(neighbors[i], neighbors[j]);

      TabuMemory::SwapEntry move = std::make_pair(i, j);
      int neighborCost =
          totalCost(neighbors, startId, inputData.startTime_, inputData.startDate_,
                    stationData, tripData, calendarServiceData);

      if ((!tabuMemory.contains(move) ||
           tabuMemory.isAspired(move, step, neighborCost, bestCost)) &&
          neighborCost < bestNeighborCost) {
        bestNeighbors = neighbors;
        bestMove = move;
        bestNeighborCost = neighborCost;
      }
    }

    if (bestMove.first == -1) {
      break;
    }

    current = bestNeighbors;
    currentCost = bestNeighborCost;

    tabuMemory.insert(bestMove, step);

    if (currentCost < bestCost) {
      bestOrder = current;
      bestCost = currentCost;
    }
  }

  return reconstructFullPath(bestOrder, startId, inputData.startTime_,
                             inputData.startDate_, stationData, tripData,
                             calendarServiceData);
}

size_t TabuSearch::CacheKeyHash::operator()(const CacheKey &k) const {
  return std::hash<int>()(k.from_) ^ (std::hash<int>()(k.to_) << 1) ^
         (std::hash<int>()(k.time_) << 2);
}

bool TabuSearch::CacheKey::operator==(const CacheKey &other) const {
  return from_ == other.from_ && to_ == other.to_ && time_ == other.time_;
}

size_t TabuSearch::TabuMemory::SwapEntryHash::operator()(const SwapEntry &p) const {
  return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
}
