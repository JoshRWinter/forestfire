#pragma once

struct Tree
{
	static constexpr float WIDTH_LOW = 0.01f;
	static constexpr float WIDTH_HIGH = 0.02f;

	Tree(float x, float y, float w, float h)
		: x(x)
		, y(y)
		, w(w)
		, h(h)
	{
	}

	float x, y;
	float w, h;
	bool dead = false;
};
