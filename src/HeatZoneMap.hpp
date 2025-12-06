#pragma once

#include <vector>

#include <win/Win.hpp>

#include "Entities.hpp"

class HeatZoneMap
{
	WIN_NO_COPY_MOVE(HeatZoneMap);

public:
	HeatZoneMap(float left, float right, float bottom, float top, int horizontal_zones, int vertical_zones);

	HeatZone &get_zone(float x, float y);

	float left, right, bottom, top;
	int horizontal_zones, vertical_zones;

	std::vector<HeatZone> zones;
};
