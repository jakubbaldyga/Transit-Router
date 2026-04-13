#include "stationLoader.hpp"

#include <string>

#include "structs/station.hpp"

StationData retrieveStations(const std::string &fileName) {
  StationData stationData;
  csv::CSVReader reader(fileName);

  const int kIsParent = 1;

  for (auto &row : reader) {
    int id = row["stop_id"].get<int>();
    std::string stationName = row["stop_name"].get<std::string>();

    stationData.idToStationName_.insert({id, stationName});

    if (row["location_type"].get<int>() == kIsParent) {
      // we can add it as a "new" station
      auto lat = row["stop_lat"].get<float>();
      auto lng = row["stop_lon"].get<float>();

      stationData.nameToStationId_.insert({stationName, id});

      stationData.stations_.insert({id, {id, stationName, lat, lng}});

      stationData.parentId_[id] = id;
    } else {
      int id = row["stop_id"].get<int>();
      int parentId = row["parent_station"].get<int>();
      stationData.parentId_[id] = parentId;
    }
  }
  return stationData;
}
