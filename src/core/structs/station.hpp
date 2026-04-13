#pragma once

#include <csv.hpp>
#include <ctime>
#include <iostream>
#include <unordered_map>

#include "core_export.hpp"

class CORE_API Station {
  // parentID is a station id and is unique
  // if station doesnt have parentID - it is a self-parent station
  int parentID_;

  std::string stationName_;

  float lat_;
  float lng_;

public:
  Station(int parentID, std::string stationName, float lat, float lng);

  friend std::ostream &operator<<(std::ostream &os, const Station &obj);

  [[nodiscard]] double distTo(const Station &other) const;
  [[nodiscard]] float lat() const;
  [[nodiscard]] float lng() const;
};

struct CORE_API StationData {
  std::unordered_map<int, Station> stations_;
  std::unordered_map<std::string, int> nameToStationId_;
  std::unordered_map<int, std::string> idToStationName_;
  std::unordered_map<int, int> parentId_;
};