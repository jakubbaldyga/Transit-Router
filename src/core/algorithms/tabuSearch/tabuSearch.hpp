#pragma once
#include <climits>
#include <memory>
#include <queue>
#include <unordered_set>

#include "../algorithm.hpp"

class CORE_API TabuSearch : public AlgorithmMany {
  static constexpr int kStepLimit = 500;
  static constexpr double kAspirationEps = 0.1;
  static constexpr int kTabuSizeMultiplier = 4;

  std::unique_ptr<Algorithm> algorithm_;

  struct TabuMemory {
    using SwapEntry = std::pair<int, int>;
    struct SwapEntryHash {
      size_t operator()(const SwapEntry &p) const;
    };

    std::unordered_set<SwapEntry, SwapEntryHash> tabuSet_;
    std::queue<SwapEntry> tabuQueue_;
    std::unordered_map<SwapEntry, int, SwapEntryHash> lastUsed_;

    int maxSize_;

    explicit TabuMemory(int size);

    bool contains(const SwapEntry &move) const;

    void insert(const SwapEntry &move, int step);

    bool isAspired(const SwapEntry &move, int step, int neighborCost, int bestCost) const;
  };

  struct CacheKey {
    int from_;
    int to_;
    int time_;
    bool operator==(const CacheKey &other) const;
  };

  struct CacheKeyHash {
    size_t operator()(const CacheKey &k) const;
  };

  std::unordered_map<CacheKey, int, CacheKeyHash> distanceCache_;

  int stationDistance(int from, int to, int time, int date,
                      const StationData &stationData, const TripData &tripData,
                      const CalendarServiceData &calendarServiceData);

  int totalCost(const std::vector<int> &stations, int startId, int startTime, int date,
                const StationData &stationData, const TripData &tripData,
                const CalendarServiceData &calendarServiceData);

  std::vector<TabuMemory::SwapEntry> sampleNeighborhood(int n);

  std::optional<PathData>
  reconstructFullPath(const std::vector<int> &perm, int startId, int startTime,
                      int startDate, const StationData &stationData,
                      const TripData &tripData,
                      const CalendarServiceData &calendarServiceData);

  std::vector<int> getShuffledStationIds(const StationData &stationData,
                                         const InputDataTabu &inputData);

public:
  explicit TabuSearch(std::unique_ptr<Algorithm> &&algorithm)
      : algorithm_(std::move(algorithm)) {}

  std::optional<PathData> findPath(const StationData &stationData,
                                   const TripData &tripData,
                                   const CalendarServiceData &calendarServiceData,
                                   const InputDataTabu &inputData) override;
};