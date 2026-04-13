#include "algorithms/aStarTime/aStarTime.hpp"
#include "dataLoader/calendarLoader.hpp"
#include "dataLoader/stationLoader.hpp"
#include "dataLoader/tripLoader.hpp"
#include "utils.hpp"
#include "gtest/gtest.h"

using ::testing::Test;

class AStarTest : public Test {
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

TripData AStarTest::tripData;
StationData AStarTest::stationData;
CalendarServiceData AStarTest::calendarServiceData;

TEST_F(AStarTest, SimpleTest) {
  AStarTime aStar;

  InputData inputData;
  inputData.startStation_ = "Trutnov hl. n.";
  inputData.endStation_ = "Libeč";
  inputData.startTime_ = parseTime("05:36:00");
  inputData.startDate_ = 20260225;

  auto result = aStar.findPath(stationData, tripData, calendarServiceData, inputData);
  if (!result.has_value()) {
    FAIL() << "No path found";
  }

  EXPECT_EQ(result->endTime_, parseTime("05:48:00"));

  inputData.startStation_ = "Trutnov hl. n.";
  inputData.endStation_ = "Głogów";
  result = aStar.findPath(stationData, tripData, calendarServiceData, inputData);
  EXPECT_EQ(result->endTime_, parseTime("10:08:00"));
  EXPECT_EQ(result->stations_.size(), 3);
  result->printResult();
}

TEST_F(AStarTest, MultipleDaysTest) {
  AStarTime aStar;

  InputData inputData;
  inputData.startStation_ = "Wałbrzych Główny";
  inputData.endStation_ = "Jelenia Góra";
  inputData.startTime_ = parseTime("23:30:00");
  inputData.startDate_ = 20260227;

  auto result = aStar.findPath(stationData, tripData, calendarServiceData, inputData);
  if (!result.has_value()) {
    FAIL() << "No path found";
  }
  EXPECT_EQ(result->endTime_, parseTime("01:13:00") + kDaySeconds);
  result->printResult();
}

TEST_F(AStarTest, ComplexTest) {
  AStarTime aStar;

  InputData inputData;
  inputData.startStation_ = "Vratislavice nad Nisou";
  inputData.endStation_ = "Bautzen";
  inputData.startTime_ = parseTime("05:36:00");
  inputData.startDate_ = 20260225;

  auto result = aStar.findPath(stationData, tripData, calendarServiceData, inputData);
  if (!result.has_value()) {
    FAIL() << "No path found";
  }
  EXPECT_EQ(result->endTime_, parseTime("12:42:00"));
  EXPECT_EQ(result->stations_.size(), 4);
}