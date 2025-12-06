#include <cmath>

#include "HeatZoneMap.hpp"

HeatZoneMap::HeatZoneMap(float left, float right, float bottom, float top, int horizontal_zones, int vertical_zones)
	: left(left)
	, right(right)
	, bottom(bottom)
	, top(top)
	, horizontal_zones(horizontal_zones)
	, vertical_zones(vertical_zones)
	, zones(horizontal_zones * vertical_zones)
{
}

HeatZone &HeatZoneMap::get_zone(float x, float y)
{
	const float width = right - left;
	const float height = top - bottom;

	const int zone_x = std::floor(((x - left) / width) * horizontal_zones);
	const int zone_y = std::floor(((y - bottom) / height) * vertical_zones);

#ifndef NDEBUG
	if (zone_x >= horizontal_zones)
		win::bug("Point " + std::to_string(x) + ", " + std::to_string(y) + " is gte than horizontal_zones (" + std::to_string(horizontal_zones) + ")");
	if (zone_y >= vertical_zones)
		win::bug("Point " + std::to_string(x) + ", " + std::to_string(y) + " is gte than vertical_zones (" + std::to_string(vertical_zones) + ")");
#endif

	return zones[(zone_y * horizontal_zones) + zone_x];
}
