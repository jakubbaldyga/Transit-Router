#pragma once

#include "core_export.hpp"
#include "string"
#include "structs/calendarService.hpp"

static void loadExceptionDates(const std::string &fileName, CalendarServiceData &data);

CalendarServiceData CORE_API retrieveCalendarServiceData(
    const std::string &calendarFileName, const std::string &exceptionsFileName);