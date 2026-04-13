#pragma once
#include <chrono>
#include <climits>
#include <cstdlib>
#include <random>
#include <string>
#include <string_view>
#include <vector>


#include "core_export.hpp"

CORE_API int parseTime(const std::string &t);

CORE_API std::string formatTime(int t);

CORE_API int parseDate(const std::string &d);

CORE_API std::string formatDate(int d);

static inline std::mt19937 rng{std::random_device{}()};

CORE_API std::vector<int> shuffleVector(std::vector<int> v);

CORE_API int getRandom(int bottom, int top);

inline constexpr std::string_view kDefaultGtfsDataPath = "google_transit";

CORE_API std::string getGtfsDataPathFromEnvOrDefault() ;

using WeekDay = std::uint8_t;

inline constexpr WeekDay kMonday = 1u << 0;
inline constexpr WeekDay kTuesday = 1u << 1;
inline constexpr WeekDay kWednesday = 1u << 2;
inline constexpr WeekDay kThursday = 1u << 3;
inline constexpr WeekDay kFriday = 1u << 4;
inline constexpr WeekDay kSaturday = 1u << 5;
inline constexpr WeekDay kSunday = 1u << 6;

inline constexpr int kDaySeconds = 86400;

inline WeekDay weekdayIndexMon0(std::chrono::year_month_day ymd) {
  std::chrono::weekday wd{std::chrono::sys_days{ymd}};
  int c = wd.c_encoding(); // 0=Sun..6=Sat
  return (c + 6) % 7;      // 0=Mon..6=Sun
}

inline bool worksOnDate(std::chrono::year_month_day ymd, WeekDay flags) {
  int idx = weekdayIndexMon0(ymd);
  return (flags & (1u << idx)) != 0;
}