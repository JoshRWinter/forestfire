#pragma once

struct SimulationSettings
{
	// How fast a tree progresses from green to burnt
	// Reasonable value: 0.001 to 0.1
	float burn_rate = 0;

	// How fast a tree fades out after reaching maximum burnout
	// Reasonable value: 0.001 to 0.1
	float fade_out_rate = 0;

	// How long until a neighboring tree catches fire
	// Reasonable value: 0.05 to 0.8
	float catch_fire_threshold = 0;

	// How far a burning tree will light its neighbors on fire
	// Reasonable value: 3 to 6
	int burn_radius = 0;
};
