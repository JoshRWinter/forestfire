#version 460 core

// tree_info:
// .r = burn
// .g = if r less than one, then g is age (fade-in), otherwise g is dying (fade-out)

layout (std140) uniform colordata
{
	vec4 colors[6];
};

layout (location = 0) out vec4 tree_info;
layout (location = 1) out vec3 tree_visual;

in vec2 noisetexcoord;

uniform ivec2 strike;
uniform float strike_color;
uniform float time;

uniform sampler2D noise;
uniform sampler2D trees;
uniform sampler2D fire;

const int strike_radius = 5;
const float fade_in_rate = 0.03;

// simulation settings
uniform float burn_rate;
uniform float fade_out_rate;
uniform float catch_fire_threshold;

vec3 get_color(float index)
{
	float color_a = floor(index);
	float color_b = ceil(index);
	color_b = color_b <= 5 ? color_b : 0;

	float progression = index - color_a;

	vec3 color = mix(colors[int(color_a)], colors[int(color_b)], progression).rgb;

	return color;
}

void main()
{
	bool newtree = texture(noise, noisetexcoord).r > 0.0;// make a new tree
	vec4 existing_tree = texelFetch(trees, ivec2(gl_FragCoord.x, gl_FragCoord.y), 0);
	vec2 existing_fire = texelFetch(fire, ivec2(gl_FragCoord.x, gl_FragCoord.y), 0).rg;// is a tree burning nearby

	bool exists = existing_tree.g > 0.0;// tree at this spot exists
	bool burning = existing_tree.r > 0.0;// tree is currently on fire
	bool burnt_up = existing_tree.r == 1.0;// tree is all burnt up

	if (exists)
	{
		bool lightning_strike = strike.x != -1 && abs(gl_FragCoord.x - strike.x) < strike_radius && abs(gl_FragCoord.y - strike.y) < strike_radius;
		bool near_burning_tree = existing_fire.r > catch_fire_threshold;

		if (lightning_strike)
		{
			existing_tree.a = mod(strike_color, 6);
		}

		if (near_burning_tree)
		{
			existing_tree.a = existing_fire.g;
		}

		burning = burning || lightning_strike || near_burning_tree;

		if (burning)
		{
			existing_tree.r = min(1.0, existing_tree.r + burn_rate);
		}

		existing_tree.g += burnt_up ? -fade_out_rate : fade_in_rate;
		existing_tree.g = min(1.0, max(0.0, existing_tree.g));

		// delete
		if (existing_tree.g == 0.0)
		{
			exists = false;
			existing_tree.r = 0.0;
			existing_tree.g = 0.0;
		}

		tree_info = existing_tree;
	}
	else if (newtree && existing_fire.r == 0.0)
	{
		// no tree at this spot, but we can plant one
		exists = true;
		float color_index = mod(time, 6);
		tree_info = vec4(0.0, fade_in_rate, color_index * 4, 0.0);
	}
	else
	{
		// no tree at this spot
		exists = false;
		tree_info = vec4(0.0, 0.0, 0.0, 0.0);
	}

	if (exists)
	{
		vec3 color = get_color(tree_info.b);
		vec3 firecolor = get_color(tree_info.a);

		tree_visual.r = (color.r * (1.0 - tree_info.r)) + (firecolor.r * tree_info.r);
		tree_visual.g = (color.g * (1.0 - tree_info.r)) + (firecolor.g * tree_info.r);
		tree_visual.b = (color.b * (1.0 - tree_info.r)) + (firecolor.b * tree_info.r);

		float twinkle_phase_shift = float(gl_FragCoord.x + gl_FragCoord.y);
		float twinkle_freq = time * (burning ? 3000 : 1000);
		float twinkle = abs(sin(twinkle_phase_shift + twinkle_freq));
		twinkle = (twinkle * 0.85) + 0.15;
		tree_visual = tree_visual.rgb * twinkle;

		// shorten the visual fade-out vs the real one
		float visual_fade = burnt_up ? max(0.0, 1.0 - ((1.0 - tree_info.g) * 4.0)) : tree_info.g;

		// fade effect
		tree_visual = vec3(tree_visual.rgb * visual_fade);
	}
	else
	{
		tree_visual = vec3(0.0, 0.0, 0.0);
	}

	//tree_visual = vec3(existing_fire * 0.2 + tree_visual.r * 0.8, tree_visual.gb);
}
