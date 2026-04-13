#pragma once

#include <SFML/Graphics.hpp>
#include <cpr/cpr.h>

#include <map>
#include <memory>
#include <tuple>
#include <vector>

class MapRenderer {
public:
  struct GeoPoint {
    float lat_;
    float lon_;
  };

  struct Polyline {
    std::vector<GeoPoint> points_;
    sf::Color color_;
    float thickness_ = 1.0f;
  };

  explicit MapRenderer(int initialZoom = 3);

  void setCenter(float lat, float lon);
  void setZoom(int newZoom);
  [[nodiscard]] int zoom() const;

  void handleEvent(const sf::Event &event, const sf::RenderWindow &window);
  void drawBaseMap(sf::RenderWindow &window);
  void drawPolyline(sf::RenderWindow &window, const Polyline &polyline) const;

  [[nodiscard]] sf::Vector2f geoToScreen(const sf::RenderWindow &window,
                                         GeoPoint point) const;

private:
  struct Tile {
    sf::Texture texture_;
    std::unique_ptr<sf::Sprite> sprite_;
    bool loaded_ = false;
    cpr::AsyncResponse future_; // std::future<cpr::Response> z biblioteki cpr
  };

  static constexpr int kTileSize = 256;
  static constexpr int kMinZoom = 1;
  static constexpr int kMaxZoom = 18;

  int zoom_;
  float centerWorldX_ = 0.0f;
  float centerWorldY_ = 0.0f;

  bool dragging_ = false;
  sf::Vector2i lastMouse_{};

  std::map<std::tuple<int, int, int>, Tile> tileCache_;

  Tile &getTile(int z, int x, int y);

  static sf::Vector2f latLonToWorld(float lat, float lon, int zoom);
  [[nodiscard]] sf::Vector2f worldToScreen(const sf::RenderWindow &window,
                                           const sf::Vector2f &world) const;

  static void drawThickLine(sf::RenderWindow &window, const sf::Vector2f &a,
                            const sf::Vector2f &b, float thickness,
                            const sf::Color &color);
};
