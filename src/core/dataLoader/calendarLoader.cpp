#include "./calendarLoader.hpp"

#include "csv.hpp"
#include "utils.hpp"
#include <utility>
#include <vector>

CalendarServiceData retrieveCalendarServiceData(const std::string &calendarFileName,
                                                const std::string &exceptionsFileName) {
  CalendarServiceData data;

  csv::CSVReader reader(calendarFileName);

  for (auto &row : reader) {
    std::string serviceId = row["service_id"].get<std::string>();
    int startDate = row["start_date"].get<int>();
    int endDate = row["end_date"].get<int>();

    WeekDay weekDays = 0;

    // clang-format off
    std::vector<std::pair<std::string, int>> weekDayFlags = {
        {"monday", kMonday},
        {"tuesday", kTuesday},
        {"wednesday", kWednesday},
        {"thursday", kThursday},
        {"friday", kFriday},
        {"saturday", kSaturday},
        {"sunday", kSunday}
    };
    // clang-format on

    for (const auto &[dayName, flag] : weekDayFlags) {
      if (row[dayName].get<int>() == 1) {
        weekDays |= flag;
      }
    }

    data.services_[serviceId] = CalendarService{.serviceId_ = serviceId,
                                                .startDate_ = startDate,
                                                .endDate_ = endDate,
                                                .weekDays_ = weekDays};
  }

  loadExceptionDates(exceptionsFileName, data);

  return data;
}

static void loadExceptionDates(const std::string &fileName, CalendarServiceData &data) {
  csv::CSVReader reader(fileName);

  for (auto &row : reader) {
    std::string serviceId = row["service_id"].get<std::string>();
    int date = row["date"].get<int>();
    int exceptionType = row["exception_type"].get<int>();

    if (exceptionType == 1) {
      data.services_[serviceId].addedDates_.insert(date);
    } else if (exceptionType == 2) {
      data.services_[serviceId].removedDates_.insert(date);
    }
  }
}
