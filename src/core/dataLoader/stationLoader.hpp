#pragma once

#include <string>

#include "core_export.hpp"
#include "structs/station.hpp"

CORE_API StationData retrieveStations(const std::string &fileName);
