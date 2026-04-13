#include "utils.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <stdexcept>

int parseTime(const std::string &t) {
  int h = 0, m = 0, s = 0;
  if (std::sscanf(t.c_str(), "%d:%d:%d", &h, &m, &s) != 3) {
    throw std::invalid_argument("Invalid time format: '" + t + "' (expected HH:MM:SS)");
  }

  // GTFS allows hours >= 24 for next-day trips, but minutes/seconds must be valid.
  if (h < 0 || m < 0 || m >= 60 || s < 0 || s >= 60) {
    throw std::invalid_argument("Invalid time value: '" + t + "'");
  }

  return h * 3600 + m * 60 + s;
}

int parseDate(const std::string &d) {
  int day = 0, month = 0, year = 0;
  if (std::sscanf(d.c_str(), "%d.%d.%d", &day, &month, &year) != 3) {
    throw std::invalid_argument("Invalid date format: '" + d + "' (expected DD.MM.YYYY)");
  }

  if (year < 0 || month < 1 || month > 12 || day < 1 || day > 31) {
    throw std::invalid_argument("Invalid date value: '" + d + "'");
  }

  const std::chrono::year_month_day ymd{std::chrono::year{year},
                                        std::chrono::month{static_cast<unsigned>(month)},
                                        std::chrono::day{static_cast<unsigned>(day)}};
  if (!ymd.ok()) {
    throw std::invalid_argument("Invalid calendar date: '" + d + "'");
  }

  return year * 10000 + month * 100 + day;
}

std::string formatTime(int t) {
  int h = t / 3600;
  int m = (t % 3600) / 60;
  int s = t % 60;
  std::array<char, 9> buf;
  std::snprintf(buf.data(), buf.size(), "%02d:%02d:%02d", h, m, s);
  return buf.data();
}

std::string formatDate(int d) {
  int year = d / 10000;
  int month = (d % 10000) / 100;
  int day = d % 100;
  std::array<char, 11> buf;
  std::snprintf(buf.data(), buf.size(), "%02d.%02d.%04d", day, month, year);
  return buf.data();
}

int getRandom(int bottom, int top) {
  std::uniform_int_distribution<int> dist(bottom, top);
  return dist(rng);
}

std::vector<int> shuffleVector(std::vector<int> v) {
  std::shuffle(v.begin(), v.end(), rng);
  return v;
}

std::string getGtfsDataPathFromEnvOrDefault() {
  if (const char *envPath = std::getenv("GTFS_DATA_PATH");
      envPath != nullptr && envPath[0] != '\0') {
    return envPath;
  }
  return std::string(kDefaultGtfsDataPath);
}
