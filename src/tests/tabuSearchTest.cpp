#include "algorithms/tabuSearch/tabuSearch.hpp"
#include "algorithms/dijkistra/dijkistra.hpp"
#include "dataLoader/calendarLoader.hpp"
#include "dataLoader/stationLoader.hpp"
#include "dataLoader/tripLoader.hpp"
#include "structs/calendarService.hpp"
#include "utils.hpp"
#include "gtest/gtest.h"

using ::testing::Test;

class TabuSearchTest : public Test {
protected:
  static void SetUpTestSuite() {
    const std::string dataPath = getGtfsDataPathFromEnvOrDefault();
    tripData = retrieveTrips(dataPath + "/stop_times.txt");
    stationData = retrieveStations(dataPath + "/stops.txt");
    calendarServiceData = retrieveCalendarServiceData(dataPath + "/calendar.txt",
                                                      dataPath + "/calendar_dates.txt");
  }

  static TripData tripData;
  static StationData stationData;
  static CalendarServiceData calendarServiceData;
};

TripData TabuSearchTest::tripData;
StationData TabuSearchTest::stationData;
CalendarServiceData TabuSearchTest::calendarServiceData;

TEST_F(TabuSearchTest, SimpleRoute) {
  try {
    TabuSearch tabu(std::make_unique<Dijikistra>());
    InputDataTabu inputData;
    inputData.startStation_ = "Trutnov hl. n.";
    inputData.stations_ = {"Libeč", "Głogów"};
    inputData.startTime_ = parseTime("05:36:00");
    inputData.startDate_ = 20260225;

    auto result = tabu.findPath(stationData, tripData, calendarServiceData, inputData);

    EXPECT_TRUE(result.has_value());
    result->printResult();
  } catch (const std::exception &e) {
    FAIL() << "Exception: " << e.what();
  }
}

TEST_F(TabuSearchTest, ComplexRoutes) {
  TabuSearch tabu(std::make_unique<Dijikistra>());
  InputDataTabu inputData;
  inputData.startStation_ = "Trutnov hl. n.";
  inputData.stations_ = {
      "Libeč",         "Bautzen",   "Głogów",      "Arnsdorf (b Dresden)",
      "Bischofswerda", "Zgorzelec", "Lubań Śląski"};
  inputData.startTime_ = parseTime("05:36:00");
  inputData.startDate_ = 20260225;

  auto result = tabu.findPath(stationData, tripData, calendarServiceData, inputData);

  EXPECT_TRUE(result.has_value());
  result->printResult();
}