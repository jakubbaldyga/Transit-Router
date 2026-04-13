#pragma once

#include <ctime>
#include <iomanip>
#include <iostream>
#include <queue>
#include <set>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "core_export.hpp"
#include "csv.hpp"
#include "utils.hpp"

class CORE_API TripStation {
public:
  int stationId_;
  int seq_;

  int arrival_;
  int departure_;

  TripStation(int stationId, int seq, int arrival, int departure);
};

struct CORE_API UniqueSequenceStations {
  bool operator()(const TripStation &a, const TripStation &b) const;
};

class CORE_API Trip {
public:
  std::string tripId_;
  std::string serviceId_;
  std::set<TripStation, UniqueSequenceStations> stations_;

  void addStation(int seq, int stationId, int arrival, int departure);
};

struct TripData {
  std::unordered_map<std::string, Trip> trips_;
};