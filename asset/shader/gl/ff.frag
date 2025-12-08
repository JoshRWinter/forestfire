#version 460 core

// tree_info:
// .r = burn
// .g = if r less than one, then g is age (fade-in), otherwise g is dying (fade-out)

layout (location = 0) out vec2 tree_info;
layout (location = 1) out vec3 tree_visual;

in vec2 noisetexcoord;
in vec2 ftexcoord;

uniform ivec2 strike;
uniform float time;

uniform sampler2D noise;
uniform sampler2D trees;
uniform sampler2D fire;

const int strike_radius = 5;
const float burn_rate = 0.01;
const float fade_rate = 0.05;

void main()
{
	bool newtree = texture(noise, noisetexcoord).r > 0.0;
	vec2 existing_tree = texture(trees, ftexcoord).rg;
	float existing_fire = texture(fire, ftexcoord).r;

	bool exists = false;

	if (existing_tree.g > 0.0)
	{
		// there is a tree at this spot
		exists = true;

		bool lightning_strike = strike.x != -1 && abs(gl_FragCoord.x - strike.x) < strike_radius && abs(gl_FragCoord.y - strike.y) < strike_radius;
		bool near_fire = existing_fire > 0.0;

		// tree needs to burn
		if (existing_tree.r > 0.0 || lightning_strike || near_fire)
		{
			existing_tree.r = min(1.0, existing_tree.r + burn_rate);
		}

		existing_tree.g += existing_tree.r == 1.0 ? -fade_rate : fade_rate;
		existing_tree.g = min(1.0, max(0.0, existing_tree.g));

		// delete
		if (existing_tree.g == 0.0)
		{
			exists = false;
			existing_tree.r = 0.0;
			existing_tree.g = 0.0;
		}

		tree_info = existing_tree.rg;
	}
	else if (newtree)
	{
		// no tree at this spot, but we can plant one
		exists = true;
		tree_info = vec2(0.0, fade_rate);
	}
	else
	{
		// no tree at this spot
		exists = false;
		tree_info = vec2(0.0, 0.0);
	}

	if (exists)
	{
		tree_visual.r = tree_info.r;
		tree_visual.g = 1.0 - tree_info.r;
		tree_visual.b = 0.0f;

		float twinkle_phase_shift = float(gl_FragCoord.x + gl_FragCoord.y);
		float twinkle_freq = time * (tree_info.r > 0.0 ? 10000 : 1000);
		float twinkle = abs(sin(twinkle_phase_shift + twinkle_freq));
		twinkle = (twinkle * 0.85) + 0.15;
		tree_visual = tree_visual.rgb * twinkle;

		// fade effect
		tree_visual = vec3(tree_visual.rgb * tree_info.g);
	}
	else
	{
		tree_visual = vec3(0.0, 0.0, 0.0);
	}
}
