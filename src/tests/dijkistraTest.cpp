#include "algorithms/dijkistra/dijkistra.hpp"
#include "dataLoader/calendarLoader.hpp"
#include "dataLoader/stationLoader.hpp"
#include "dataLoader/tripLoader.hpp"
#include "utils.hpp"
#include "gtest/gtest.h"

using ::testing::Test;

class DijkstraTest : public Test {
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

TripData DijkstraTest::tripData;
StationData DijkstraTest::stationData;
CalendarServiceData DijkstraTest::calendarServiceData;
/*
Start: 05:36:00
Koniec: 09:57:00
Trasa:
37148498_396994 | 05:36:00 - Trutnov hl. n.     ->      Sędzisław - 06:33:00
37150534_399030 | 06:40:00 - Sędzisław  ->      Wrocław Główny - 08:26:00
37148766_397262 | 08:43:00 - Wrocław Główny     ->      Głogów - 09:57:00
*/
TEST_F(DijkstraTest, SimpleTest) {
  Dijikistra dijkstra;

  InputData inputData;
  inputData.startStation_ = "Trutnov hl. n.";
  inputData.endStation_ = "Libeč";
  inputData.startTime_ = parseTime("05:36:00");
  inputData.startDate_ = 20260225;

  auto result = dijkstra.findPath(stationData, tripData, calendarServiceData, inputData);
  if (!result.has_value()) {
    FAIL() << "No path found";
  }
  EXPECT_EQ(result->endTime_, parseTime("05:48:00"));
  result->printResult();

  inputData.startStation_ = "Trutnov hl. n.";
  inputData.endStation_ = "Głogów";
  result = dijkstra.findPath(stationData, tripData, calendarServiceData, inputData);
  if (!result.has_value()) {
    FAIL() << "No path found";
  }
  result->printResult();
  EXPECT_EQ(result->endTime_, parseTime("10:08:00"));
  EXPECT_EQ(result->stations_.size(), 3);
}

TEST_F(DijkstraTest, MultipleDaysTest) {
  Dijikistra dijkstra;

  InputData inputData;
  inputData.startStation_ = "Wałbrzych Główny";
  inputData.endStation_ = "Jelenia Góra";
  inputData.startTime_ = parseTime("23:30:00");
  inputData.startDate_ = 20260227;

  auto result = dijkstra.findPath(stationData, tripData, calendarServiceData, inputData);
  if (!result.has_value()) {
    FAIL() << "No path found";
  }
  EXPECT_EQ(result->endTime_, parseTime("01:13:00") + kDaySeconds);
  result->printResult();
}

TEST_F(DijkstraTest, ComplexTest) {
  Dijikistra dijkstra;

  InputData inputData;
  inputData.startStation_ = "Vratislavice nad Nisou";
  inputData.endStation_ = "Bautzen";
  inputData.startTime_ = parseTime("05:36:00");
  inputData.startDate_ = 20260225;

  auto result = dijkstra.findPath(stationData, tripData, calendarServiceData, inputData);
  if (!result.has_value()) {
    FAIL() << "No path found";
  }
  EXPECT_EQ(result->endTime_, parseTime("12:42:00"));
  EXPECT_EQ(result->stations_.size(), 4);
}
