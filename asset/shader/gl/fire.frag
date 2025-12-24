#version 460 core

layout (location = 0) out vec2 frag;

uniform sampler2D tex;
uniform bool horizontal;

uniform int burn_radius;

void main()
{

	if (horizontal)
	{
		vec2 maximum = texelFetch(tex, ivec2(gl_FragCoord.xy), 0).rb;

		for (int i = 1; i < burn_radius; ++i)
		{
			vec2 search_left = texelFetch(tex, ivec2(gl_FragCoord.x + i, gl_FragCoord.y), 0).rb;
			vec2 search_right = texelFetch(tex, ivec2(gl_FragCoord.x - i, gl_FragCoord.y), 0).rb;

			maximum = search_left.r > maximum.r ? search_left : maximum;
			maximum = search_right.r > maximum.r ? search_right : maximum;
		}

		frag = maximum;
	}
	else
	{
		vec2 maximum = texelFetch(tex, ivec2(gl_FragCoord.xy), 0).rg;

		for (int i = 1; i < burn_radius; ++i)
		{
			vec2 search_up = texelFetch(tex, ivec2(gl_FragCoord.x, gl_FragCoord.y + i), 0).rg;
			vec2 search_down = texelFetch(tex, ivec2(gl_FragCoord.x, gl_FragCoord.y - i), 0).rg;

			maximum = search_up.r > maximum.r ? search_up : maximum;
			maximum = search_down.r > maximum.r ? search_down : maximum;
		}

		frag = maximum;
	}
}
