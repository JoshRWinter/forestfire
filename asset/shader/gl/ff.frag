#version 460 core

// tree_info:
// .r = burn
// .g = if r less than one, then g is age (fade-in), otherwise g is dying (fade-out)
// .b = tree color (integer index)
// .a = fire color (integer index)

// fire info
// .r [0.0, 1.0] fire intensitiy
// .g fire color (integer index)

uniform int color_data_len;
layout (std430) buffer color_data
{
	vec4 colors[];
};

uniform int fire_color_data_len;
layout (std430) buffer fire_color_data
{
	vec4 fire_colors[];
};

layout (location = 0) out vec4 tree_info;
layout (location = 1) out vec3 tree_visual;

in vec2 noisetexcoord;

uniform ivec2 strike;
uniform int strike_color;
uniform uint frame;

uniform sampler2D noise;
uniform sampler2D trees;
uniform sampler2D fire;

const int strike_radius = 5;
const float fade_in_rate = 0.03;

// simulation settings
uniform float burn_rate;
uniform float fade_out_rate;
uniform float catch_fire_threshold;

vec3 get_fire_color(int index)
{
	index = index % fire_color_data_len;
	return fire_colors[index].rgb;
}

vec3 get_color(int index)
{
	index = index % color_data_len;
	return colors[int(index)].rgb;
}

vec3 deviate_color(vec3 color)
{
	float shift = sin(gl_FragCoord.x + gl_FragCoord.y) * 0.1;
	float negate = cos(gl_FragCoord.x + gl_FragCoord.y) > 0.0 ? 1.0 : -1.0;

	color.r = abs(color.r + shift * negate);
	color.g = abs(color.g + shift * -negate);
	color.b = abs(color.b + shift * negate);

	return color;
}

void main()
{
	bool newtree = texture(noise, noisetexcoord).r > 0.0;// make a new tree
	tree_info = texelFetch(trees, ivec2(gl_FragCoord.x, gl_FragCoord.y), 0);
	vec2 existing_fire = texelFetch(fire, ivec2(gl_FragCoord.x, gl_FragCoord.y), 0).rg;// is a tree burning nearby

	bool exists = tree_info.g > 0.0;// tree at this spot exists
	bool burning = tree_info.r > 0.0;// tree is currently on fire
	bool burnt_up = tree_info.r == 1.0;// tree is all burnt up

	if (exists)
	{
		bool lightning_strike = strike.x != -1 && abs(gl_FragCoord.x - strike.x) < strike_radius && abs(gl_FragCoord.y - strike.y) < strike_radius;
		bool near_burning_tree = existing_fire.r > catch_fire_threshold;

		if (lightning_strike)
		{
			tree_info.a = strike_color;
		}
		else if (near_burning_tree)
		{
			tree_info.a = existing_fire.g;
		}

		burning = burning || lightning_strike || near_burning_tree;

		if (burning)
		{
			tree_info.r = min(1.0, tree_info.r + burn_rate);
		}

		tree_info.g += burnt_up ? -fade_out_rate : fade_in_rate;
		tree_info.g = min(1.0, max(0.0, tree_info.g));

		// delete
		if (tree_info.g == 0.0)
		{
			exists = false;
			tree_info = vec4(0.0, 0.0, 0.0, 0.0);
		}
	}
	else if (newtree && existing_fire.r == 0.0)
	{
		// no tree at this spot, but we can plant one
		exists = true;
		tree_info.g = fade_in_rate;
		tree_info.b = frame * 0.00005;
	}
	else
	{
		// no tree at this spot
		exists = false;
	}

	if (exists)
	{
		// figure out tree color
		vec3 color = deviate_color(get_color(int(tree_info.b)));
		vec3 firecolor = get_fire_color(int(tree_info.a));

		// blend the tree color with the fire color
		tree_visual.r = (color.r * (1.0 - tree_info.r)) + (firecolor.r * tree_info.r);
		tree_visual.g = (color.g * (1.0 - tree_info.r)) + (firecolor.g * tree_info.r);
		tree_visual.b = (color.b * (1.0 - tree_info.r)) + (firecolor.b * tree_info.r);

		// make the trees twinkle
		float twinkle_phase_shift = float(gl_FragCoord.x + gl_FragCoord.y);
		float twinkle_freq = frame * (burning ? 0.06 : 0.01);
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

	//tree_visual = vec3((colors[int(existing_fire.g)].rgb * existing_fire.r) + (tree_visual * (1.0 - existing_fire.r)));
}
