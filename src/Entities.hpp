#pragma once

#include <cmath>

#include <win/Utility.hpp>

struct Tree
{
	static constexpr float size_low = 0.01f;
	static constexpr float size_high = 0.02f;

	static constexpr float burn_rate = 0.01f;

	Tree(float x, float y, float w, float h)
		: x(x)
		, y(y)
		, w(w)
		, h(h)
	{
	}

	bool within(float xpos, float ypos, float box_radius) const { return std::fabs(x - xpos) < box_radius && std::fabs(y - ypos) < box_radius; }

	float x, y;
	float w, h;
	float burnt = 0.0f;
	bool dead = false;
};

struct DebugBlock
{
	DebugBlock(float x, float y, float w, float h, const win::Color<float> &color)
		: x(x)
		, y(y)
		, w(w)
		, h(h)
		, color(color)
	{
	}

	float x, y, w, h;
	win::Color<float> color;
};
