#include <SFML/Graphics.hpp>

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <unordered_set>
#include <vector>

#include "algorithms/aStarTransfer/aStarTransfer.hpp"
#include "algorithms/algorithm.hpp"
#include "dataLoader/calendarLoader.hpp"
#include "dataLoader/stationLoader.hpp"
#include "dataLoader/tripLoader.hpp"
#include "mapRenderer.hpp"
#include "utils.hpp"

const int kInitialZoom = 7;

struct VisualEdge {
  int from_;
  int to_;
};

std::uint64_t edgeKey(int from, int to) {
  return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(from)) << 32) |
         static_cast<std::uint32_t>(to);
}

std::vector<VisualEdge> buildWholeGraphEdges(const TripData &tripData,
                                             const StationData &stationData) {
  std::vector<VisualEdge> edges;
  std::unordered_set<std::uint64_t> seen;

  for (const auto &[tripId, trip] : tripData.trips_) {
    (void)tripId;
    if (trip.stations_.size() < 2) {
      continue;
    }

    auto from = trip.stations_.begin();
    auto to = std::next(from);
    while (to != trip.stations_.end()) {
      const int fromStation = stationData.parentId_.at(from->stationId_);
      const int toStation = stationData.parentId_.at(to->stationId_);

      if (fromStation != toStation) {
        const auto key = edgeKey(fromStation, toStation);
        if (!seen.contains(key)) {
          seen.insert(key);
          edges.push_back({.from_ = fromStation, .to_ = toStation});
        }
      }

      ++from;
      ++to;
    }
  }

  return edges;
}

std::vector<VisualEdge> buildAnalyzedEdges(const std::vector<AnalysedInfo> &analysis) {
  std::vector<VisualEdge> edges;
  edges.reserve(analysis.size());
  for (const auto &step : analysis) {
    edges.push_back({.from_ = step.from_, .to_ = step.to_});
  }
  return edges;
}

std::vector<VisualEdge> buildShortestPathEdges(const StationData &stationData,
                                               const PathData &route) {
  std::vector<VisualEdge> edges;

  edges.reserve(route.stations_.size());
  for (const auto &entry : route.stations_) {
    if (!stationData.nameToStationId_.contains(entry.stationFrom_) ||
        !stationData.nameToStationId_.contains(entry.stationTo_)) {
      continue;
    }

    edges.push_back({.from_ = stationData.nameToStationId_.at(entry.stationFrom_),
                     .to_ = stationData.nameToStationId_.at(entry.stationTo_)});
  }

  return edges;
}

void drawEdgeLayer(sf::RenderWindow &window, MapRenderer &mapRenderer,
                   const std::vector<VisualEdge> &edges, const StationData &stationData,
                   float thickness, const sf::Color &color) {
  for (const auto &edge : edges) {
    if (!stationData.stations_.contains(edge.from_) ||
        !stationData.stations_.contains(edge.to_)) {
      continue;
    }

    const Station &fromStation = stationData.stations_.at(edge.from_);
    const Station &toStation = stationData.stations_.at(edge.to_);

    MapRenderer::Polyline polyline;
    polyline.points_ = {{.lat_ = fromStation.lat(), .lon_ = fromStation.lng()},
                        {.lat_ = toStation.lat(), .lon_ = toStation.lng()}};
    polyline.color_ = color;
    polyline.thickness_ = thickness;

    mapRenderer.drawPolyline(window, polyline);
  }
}

void centerMapOnRoute(MapRenderer &mapRenderer, const StationData &stationData,
                      const std::vector<VisualEdge> &shortestPathEdges,
                      const std::string &fallbackStartStation) {
  if (!shortestPathEdges.empty()) {
    std::unordered_set<int> routeStations;
    for (const auto &edge : shortestPathEdges) {
      routeStations.insert(edge.from_);
      routeStations.insert(edge.to_);
    }

    float sumLat = 0.0f;
    float sumLng = 0.0f;
    int count = 0;
    for (const int stationId : routeStations) {

      const Station &station = stationData.stations_.at(stationId);
      sumLat += station.lat();
      sumLng += station.lng();
      ++count;
    }

    mapRenderer.setCenter(sumLat / static_cast<float>(count),
                          sumLng / static_cast<float>(count));
  } else {
    // center on start station
    const Station &station =
        stationData.stations_.at(stationData.nameToStationId_.at(fallbackStartStation));
    mapRenderer.setCenter(station.lat(), station.lng());
  }
}

int main() {
  std::filesystem::path dataPath(getGtfsDataPathFromEnvOrDefault());
  const auto stopsPath = dataPath / "stops.txt";
  const auto stopTimesPath = dataPath / "stop_times.txt";
  const auto calendarPath = dataPath / "calendar.txt";
  const auto calendarDatesPath = dataPath / "calendar_dates.txt";

  if (!std::filesystem::exists(stopsPath) || !std::filesystem::exists(stopTimesPath) ||
      !std::filesystem::exists(calendarPath) ||
      !std::filesystem::exists(calendarDatesPath)) {
    std::cerr << "Invalid GTFS data path: " << dataPath.string() << "\n"
              << "Required files: stops.txt, stop_times.txt, calendar.txt, "
              << "calendar_dates.txt\n";
    return -1;
  }

  StationData stationData = retrieveStations(stopsPath.string());
  TripData tripData = retrieveTrips(stopTimesPath.string());
  CalendarServiceData calendarServiceData =
      retrieveCalendarServiceData(calendarPath.string(), calendarDatesPath.string());

  InputData demoInput;
  demoInput.startStation_ = "Vratislavice nad Nisou";
  demoInput.endStation_ = "Bautzen";
  demoInput.startTime_ = parseTime("05:36:00");
  demoInput.startDate_ = 20260225;

  std::unique_ptr<Algorithm> algorithm = std::make_unique<AStarTransfer>();
  std::optional<PathData> route =
      algorithm->findPath(stationData, tripData, calendarServiceData, demoInput);

  if (!route.has_value()) {
    std::cerr << "No route found \n";
    return -1;
  }

  const auto &analysis = algorithm->getLastAnalysis();

  const std::vector<VisualEdge> analyzedEdges = buildAnalyzedEdges(analysis);
  const std::vector<VisualEdge> wholeGraphEdges =
      buildWholeGraphEdges(tripData, stationData);
  const std::vector<VisualEdge> shortestPathEdges =
      buildShortestPathEdges(stationData, *route);

  sf::RenderWindow window(sf::VideoMode({1000, 800}), "OSM Viewer");

  MapRenderer mapRenderer(kInitialZoom);
  mapRenderer.setZoom(kInitialZoom);
  centerMapOnRoute(mapRenderer, stationData, shortestPathEdges, demoInput.startStation_);

  bool showWholeGraph = true;
  bool showAnalyzed = true;
  bool showResult = true;
  while (window.isOpen()) {
    while (const std::optional event = window.pollEvent()) {
      if (event->is<sf::Event::Closed>()) {
        window.close();
      }

      mapRenderer.handleEvent(*event, window);

      if (const auto *key = event->getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::G) {
          showWholeGraph = !showWholeGraph;
        }
        if (key->code == sf::Keyboard::Key::A) {
          showAnalyzed = !showAnalyzed;
        }
        if (key->code == sf::Keyboard::Key::R) {
          showResult = !showResult;
        }
      }
    }

    window.clear();
    mapRenderer.drawBaseMap(window);

    if (showAnalyzed) {
      drawEdgeLayer(window, mapRenderer, analyzedEdges, stationData, 1.0f,
                    sf::Color(160, 160, 160, 255));
    }
    if (showWholeGraph) {
      drawEdgeLayer(window, mapRenderer, wholeGraphEdges, stationData, 3.0f,
                    sf::Color(100, 0, 0, 255));
    }

    if (showResult) {
      drawEdgeLayer(window, mapRenderer, shortestPathEdges, stationData, 4.0f,
                    sf::Color(255, 220, 90, 255));
    }

    if (showResult) {
      for (const auto &edge : shortestPathEdges) {
        if (!stationData.stations_.contains(edge.to_)) {
          continue;
        }

        const Station &toStation = stationData.stations_.at(edge.to_);
        const sf::Vector2f pos = mapRenderer.geoToScreen(
            window, {.lat_ = toStation.lat(), .lon_ = toStation.lng()});

        sf::CircleShape point(4.0f);
        point.setOrigin({4.0f, 4.0f});
        point.setPosition(pos);
        point.setFillColor(sf::Color(255, 255, 255));
        point.setOutlineColor(sf::Color(20, 20, 20));
        point.setOutlineThickness(1.0f);
        window.draw(point);
      }
    }

    window.display();
  }
}