#include "station.hpp"

#include <utility>

#include <numbers>

Station::Station(int parentID, std::string stationName, float lat, float lng) {
  parentID_ = parentID;
  lat_ = lat;
  lng_ = lng;
  stationName_ = std::move(stationName);
}

double Station::distTo(const Station &other) const {
  constexpr double kR = 6371e3; // promień Ziemi w metrach
  constexpr double kToRadian = std::numbers::pi / 180;
  double phi1 = lat_ * kToRadian;
  double phi2 = other.lat_ * kToRadian;
  double dphi = (other.lat_ - lat_) * kToRadian;
  double dlambda = (other.lng_ - lng_) * kToRadian;

  double a = sin(dphi / 2) * sin(dphi / 2) +
             cos(phi1) * cos(phi2) * sin(dlambda / 2) * sin(dlambda / 2);

  double c = 2 * atan2(sqrt(a), sqrt(1 - a));

  return kR * c; // metry
}

float Station::lat() const { return lat_; }

float Station::lng() const { return lng_; }

std::ostream &operator<<(std::ostream &os, const Station &obj) {
  os << "Stacja: " << obj.stationName_ << ", lat: " << obj.lat_ << ", lng: " << obj.lng_;
  return os;
}
