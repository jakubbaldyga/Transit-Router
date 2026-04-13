#include "mapRenderer.hpp"

#include <cpr/cpr.h>

#include <algorithm>
#include <cmath>
#include <numbers>

MapRenderer::MapRenderer(int initialZoom)
    : zoom_(std::clamp(initialZoom, kMinZoom, kMaxZoom)) {}

void MapRenderer::setCenter(float lat, float lon) {
  const sf::Vector2f world = latLonToWorld(lat, lon, zoom_);
  centerWorldX_ = world.x;
  centerWorldY_ = world.y;
}

void MapRenderer::setZoom(int newZoom) {
  const int clampedZoom = std::clamp(newZoom, kMinZoom, kMaxZoom);
  if (clampedZoom == zoom_) {
    return;
  }

  const float scale = std::pow(2.0f, static_cast<float>(clampedZoom - zoom_));
  centerWorldX_ *= scale;
  centerWorldY_ *= scale;
  zoom_ = clampedZoom;
}

int MapRenderer::zoom() const { return zoom_; }

void MapRenderer::handleEvent(const sf::Event &event, const sf::RenderWindow &window) {
  if (const auto *scroll = event.getIf<sf::Event::MouseWheelScrolled>()) {
    if (scroll->delta > 0) {
      setZoom(zoom_ + 1);
    } else if (scroll->delta < 0) {
      setZoom(zoom_ - 1);
    }
  } else if (event.is<sf::Event::MouseButtonPressed>()) {
    dragging_ = true;
    lastMouse_ = sf::Mouse::getPosition(window);
  } else if (event.is<sf::Event::MouseButtonReleased>()) {
    dragging_ = false;
  } else if (dragging_ && event.getIf<sf::Event::MouseMoved>() != nullptr) {
    const sf::Vector2i current = sf::Mouse::getPosition(window);
    const sf::Vector2i delta = current - lastMouse_;
    centerWorldX_ -= static_cast<float>(delta.x);
    centerWorldY_ -= static_cast<float>(delta.y);
    lastMouse_ = current;
  }
}

void MapRenderer::drawBaseMap(sf::RenderWindow &window) {
  const int tilesX = static_cast<int>(window.getSize().x) / kTileSize + 2;
  const int tilesY = static_cast<int>(window.getSize().y) / kTileSize + 2;

  const int baseX =
      static_cast<int>(std::floor(centerWorldX_ / static_cast<float>(kTileSize)));
  const int baseY =
      static_cast<int>(std::floor(centerWorldY_ / static_cast<float>(kTileSize)));

  const float offsetX = centerWorldX_ - static_cast<float>(baseX * kTileSize);
  const float offsetY = centerWorldY_ - static_cast<float>(baseY * kTileSize);

  for (int dx = -tilesX / 2 - 1; dx <= tilesX / 2 + 1; ++dx) {
    for (int dy = -tilesY / 2 - 1; dy <= tilesY / 2 + 1; ++dy) {
      const int x = baseX + dx;
      const int y = baseY + dy;

      const float screenX = (static_cast<float>(dx * kTileSize)) +
                            (static_cast<float>(window.getSize().x) / 2.0f) - offsetX;
      const float screenY = (static_cast<float>(dy * kTileSize)) +
                            (static_cast<float>(window.getSize().y) / 2.0f) - offsetY;

      Tile &tile = getTile(zoom_, x, y);
      if (tile.loaded_) {
        tile.sprite_->setPosition({screenX, screenY});
        window.draw(*tile.sprite_);
      }
    }
  }
}

void MapRenderer::drawPolyline(sf::RenderWindow &window, const Polyline &polyline) const {
  if (polyline.points_.size() < 2) {
    return;
  }

  for (std::size_t i = 1; i < polyline.points_.size(); ++i) {
    const sf::Vector2f a = geoToScreen(window, polyline.points_[i - 1]);
    const sf::Vector2f b = geoToScreen(window, polyline.points_[i]);
    drawThickLine(window, a, b, polyline.thickness_, polyline.color_);
  }
}

sf::Vector2f MapRenderer::geoToScreen(const sf::RenderWindow &window,
                                      GeoPoint point) const {
  return worldToScreen(window, latLonToWorld(point.lat_, point.lon_, zoom_));
}

MapRenderer::Tile &MapRenderer::getTile(int z, int x, int y) {
  const int maxTiles = 1 << z;
  const auto now = std::chrono::steady_clock::now();
  static constexpr auto kRetryDelay = std::chrono::seconds(2);

  x = (x % maxTiles + maxTiles) % maxTiles;

  if (y < 0 || y >= maxTiles) {
    static Tile empty;
    return empty;
  }

  const auto key = std::make_tuple(z, x, y);
  auto it = tileCache_.find(key);
  if (it == tileCache_.end()) {
    it = tileCache_.emplace(key, Tile{}).first;
  }

  Tile &tile = it->second;

  if (tile.loaded_) {
    return tile;
  }

  // Jeśli zapytanie już trwa, sprawdzamy czy się zakończyło
  if (tile.future_.valid()) {
    // Sprawdzamy czy wynik jest już dostępny (nie blokując wątku - timeout 0)
    if (tile.future_.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
      cpr::Response response = tile.future_.get();

      if (response.status_code == 200 && !response.text.empty()) {
        if (tile.texture_.loadFromMemory(response.text.data(), response.text.size())) {
          tile.sprite_ = std::make_unique<sf::Sprite>(tile.texture_);
          tile.loaded_ = true;
        }
      }
    }
    return tile;
  }

  const std::string url = "https://tile.openstreetmap.org/" + std::to_string(z) + "/" +
                          std::to_string(x) + "/" + std::to_string(y) + ".png";

  // Zamiast blokującego Get, używamy GetAsync - kod nie zablokuje wykonania GUI
  tile.future_ =
      cpr::GetAsync(cpr::Url{url}, cpr::Timeout{5000},
                    cpr::Header{{"User-Agent", "OSM Viewer"}, {"Accept", "image/png"}});

  return tile;
}

sf::Vector2f MapRenderer::latLonToWorld(float lat, float lon, int zoom) {
  const double n = std::pow(2.0, static_cast<double>(zoom));
  const double xTile = (static_cast<double>(lon) + 180.0) / 360.0 * n;
  const double latRad = static_cast<double>(lat) * std::numbers::pi / 180.0;
  const double yTile =
      (1.0 - std::log(std::tan(latRad) + (1.0 / std::cos(latRad))) / std::numbers::pi) /
      2.0 * n;

  return {static_cast<float>(xTile * kTileSize), static_cast<float>(yTile * kTileSize)};
}

sf::Vector2f MapRenderer::worldToScreen(const sf::RenderWindow &window,
                                        const sf::Vector2f &world) const {
  return {world.x - centerWorldX_ + (static_cast<float>(window.getSize().x) / 2.0f),
          world.y - centerWorldY_ + (static_cast<float>(window.getSize().y) / 2.0f)};
}

void MapRenderer::drawThickLine(sf::RenderWindow &window, const sf::Vector2f &a,
                                const sf::Vector2f &b, float thickness,
                                const sf::Color &color) {
  const sf::Vector2f diff = b - a;
  const float len = std::sqrt(diff.x * diff.x + diff.y * diff.y);
  if (len < 1e-3f) {
    return;
  }

  sf::RectangleShape line({len, thickness});
  line.setPosition(a);
  line.setFillColor(color);
  line.setRotation(sf::radians(std::atan2(diff.y, diff.x)));
  line.setOrigin({0.0f, thickness * 0.5f});
  window.draw(line);
}
