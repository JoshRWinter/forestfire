#version 460 core

layout (location = 0) out float frag;
in vec2 ftexcoord;

uniform sampler2D tex;
uniform bool horizontal;

uniform int burn_radius;

void main()
{
	vec2 dims = textureSize(tex, 0);
	float maximum = 0.0;

	if (horizontal)
	{
		float shift = 1.0 / dims.x;
		for (int i = 0; i < burn_radius; ++i)
		{
			vec4 search_left = texture(tex, vec2(ftexcoord.x + (shift * i), ftexcoord.y));
			vec4 search_right = texture(tex, vec2(ftexcoord.x - (shift * i), ftexcoord.y));
			maximum = max(maximum, search_left.r);
			maximum = max(maximum, search_right.r);
		}
	}
	else
	{
		float shift = 1.0 / dims.y;
		for (int i = 0; i < burn_radius; ++i)
		{
			vec4 search_up = texture(tex, vec2(ftexcoord.x, ftexcoord.y + (shift * i)));
			vec4 search_down = texture(tex, vec2(ftexcoord.x, ftexcoord.y - (shift * i)));
			maximum = max(maximum, search_up.r);
			maximum = max(maximum, search_down.r);
		}
	}

	frag = maximum;
}
