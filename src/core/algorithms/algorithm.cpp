#include "algorithm.hpp"

#include "utils.hpp"
#include <utility>

PathData::PathData(int startDate, int startTime, int endTime, PathDetails stations) {
  this->startDate_ = startDate;
  this->startTime_ = startTime;
  this->endTime_ = endTime;
  this->stations_ = std::move(stations);
}

void PathData::printResult() const {
  std::cout << "\nStart: " << formatTime(startTime_) << "\n";
  std::cout << "Koniec: " << formatTime(endTime_) << "\n";
  std::cout << "Trasa:\n";
  for (const auto &entry : stations_) {
    std::cout << entry.lineId_ << " | "
              << formatDate(startDate_ + entry.timeFrom_ / kDaySeconds) << " | "
              << formatTime(entry.timeFrom_ % kDaySeconds) << " - " << entry.stationFrom_
              << "\t->\t" << entry.stationTo_ << " - "
              << formatTime(entry.timeTo_ % kDaySeconds) << "\n";
  }
  std::cout << "\n";
}

InputData::InputData(const std::string &startStation, const std::string &endStation,
                     int startTime, int startDate) {
  this->startStation_ = startStation;
  this->endStation_ = endStation;
  this->startTime_ = startTime;
  this->startDate_ = startDate;
}
