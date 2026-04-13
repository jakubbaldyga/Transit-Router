#include "dataLoader/calendarLoader.hpp"
#include "dataLoader/stationLoader.hpp"
#include "dataLoader/tripLoader.hpp"
#include "utils.hpp"
#include "gtest/gtest.h"

TEST(CalendarLoader, Loader) {
  const std::string dataPath = getGtfsDataPathFromEnvOrDefault();
  auto calendarData = retrieveCalendarServiceData(dataPath + "/calendar.txt",
                                                  dataPath + "/calendar_dates.txt");
  EXPECT_EQ(calendarData.services_["1004_398133"].serviceId_, "1004_398133");
  EXPECT_EQ(calendarData.services_["1004_398133"].startDate_, 20260225);
  EXPECT_EQ(calendarData.services_["1004_398133"].endDate_, 20260306);
  EXPECT_EQ(calendarData.services_["1004_398133"].weekDays_,
            kMonday | kTuesday | kWednesday | kThursday | kFriday);
};

TEST(StationLoader, Loader) {
  const std::string dataPath = getGtfsDataPathFromEnvOrDefault();
  auto stationData = retrieveStations(dataPath + "/stops.txt");
  EXPECT_EQ(stationData.idToStationName_[1474824], "Trutnov hl. n.");
  EXPECT_EQ(stationData.idToStationName_[1413352], "Trutnov hl. n.");

  EXPECT_EQ(stationData.nameToStationId_["Trutnov hl. n."], 1413352);

  EXPECT_EQ(stationData.nameToStationId_["Křenov"], 1413195);
}

TEST(TripLoader, Loader) {
  const std::string dataPath = getGtfsDataPathFromEnvOrDefault();
  auto tripsData = retrieveTrips(dataPath + "/stop_times.txt");

  EXPECT_EQ(tripsData.trips_["37148498_396994"].tripId_, "37148498_396994");
  EXPECT_EQ(tripsData.trips_["37148498_396994"].serviceId_, "1_396994");
  EXPECT_EQ(tripsData.trips_["37148498_396994"].stations_.begin()->seq_, 0);
  EXPECT_EQ(tripsData.trips_["37148498_396994"].stations_.begin()->stationId_, 1474824);
  EXPECT_EQ(tripsData.trips_["37148498_396994"].stations_.rbegin()->seq_, 9);
  EXPECT_EQ(tripsData.trips_["37148498_396994"].stations_.rbegin()->stationId_, 1474758);
}
