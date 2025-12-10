#version 460 core

layout (location = 0) out float frag;

uniform sampler2D tex;
uniform bool horizontal;

uniform int burn_radius;

void main()
{
	float maximum = texelFetch(tex, ivec2(gl_FragCoord.xy), 0).r;

	if (horizontal)
	{
		for (int i = 1; i < burn_radius; ++i)
		{
			float search_left = texelFetch(tex, ivec2(gl_FragCoord.x + i, gl_FragCoord.y), 0).r;
			float search_right = texelFetch(tex, ivec2(gl_FragCoord.x - i, gl_FragCoord.y), 0).r;

			maximum = max(maximum, search_left);
			maximum = max(maximum, search_right);
		}
	}
	else
	{
		for (int i = 1; i < burn_radius; ++i)
		{
			float search_up = texelFetch(tex, ivec2(gl_FragCoord.x, gl_FragCoord.y + i), 0).r;
			float search_down = texelFetch(tex, ivec2(gl_FragCoord.x, gl_FragCoord.y - i), 0).r;

			maximum = max(maximum, search_up);
			maximum = max(maximum, search_down);
		}
	}

	frag = maximum;
}
