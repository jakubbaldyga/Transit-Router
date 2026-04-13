#include "tripLoader.hpp"
#include "internal/csv_reader.hpp"

TripData retrieveTrips(const std::string &fileName) {
  TripData tripData;
  csv::CSVReader reader("google_transit/stop_times.txt");

  for (auto &row : reader) {
    std::string tripId = row["trip_id"].get<>();
    int stopId = row["stop_id"].get<int>();
    int seqNum = row["stop_sequence"].get<int>();
    int arrivalTime = parseTime(row["arrival_time"].get<std::string>());
    int departureTime = parseTime(row["departure_time"].get<std::string>());

    if (!tripData.trips_.contains(tripId)) {
      tripData.trips_[tripId].tripId_ = tripId;
    }

    tripData.trips_[tripId].addStation(seqNum, stopId, arrivalTime, departureTime);
  }

  csv::CSVReader reader2("google_transit/trips.txt");
  for (auto &row : reader2) {
    std::string tripId = row["trip_id"].get<std::string>();
    std::string serviceId = row["service_id"].get<std::string>();

    tripData.trips_[tripId].serviceId_ = serviceId;
  }

  return tripData;
}
