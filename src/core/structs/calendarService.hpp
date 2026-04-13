#pragma once

#include "utils.hpp"
#include <set>
#include <unordered_map>

struct CalendarService {
  std::string serviceId_;

  int startDate_;
  int endDate_;
  WeekDay weekDays_; // bitmask of days of the week

  std::set<int> addedDates_;
  std::set<int> removedDates_;

  [[nodiscard]] bool isTripActive(int departureTime) const {
    if (removedDates_.contains(departureTime)) {
      return false;
    }
    if (addedDates_.contains(departureTime)) {
      return true;
    }
    if (departureTime < startDate_ || departureTime > endDate_) {
      return false;
    }
    std::chrono::year_month_day ymd{std::chrono::year{departureTime / 10000},
                                    std::chrono::month((departureTime / 100) % 100),
                                    std::chrono::day(departureTime % 100)};
    return worksOnDate(ymd, weekDays_);
  }
};

struct CORE_API CalendarServiceData {
  std::unordered_map<std::string, CalendarService> services_;
};