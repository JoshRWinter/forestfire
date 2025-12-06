#include <chrono>
#include <random>
#include <vector>

#include <win/AssetRoll.hpp>
#include <win/Display.hpp>
#include <win/gl/GL.hpp>
#include <win/Utility.hpp>

#include "HeatZoneMap.hpp"
#include "Renderer.hpp"

#if defined WINPLAT_WINDOWS && NDEBUG
int WinMain(HINSTANCE hinstance, HINSTANCE prev, PSTR cmd, int show)
#else
int main(int argc, char **argv)
#endif
{
	win::AssetRoll roll("ff.roll");

	// display setup
	win::DisplayOptions display_options;
#ifndef NDEBUG
	display_options.caption = "debug_window";
	display_options.fullscreen = false;
	display_options.width = 1600;
	display_options.height = 900;
	display_options.debug = false;
#else
	display_options.caption = "forestfire";
	display_options.fullscreen = true;
	display_options.width = 1600;
	display_options.height = 900;
#endif
	display_options.gl_major = 4;
	display_options.gl_minor = 6;

	win::Display display(display_options);
	display.vsync(true);
	// display.cursor(false);
	bool fullscreen = display_options.fullscreen;

	win::load_gl_functions();

	bool quit = false;
	display.register_button_handler(
		[&quit](win::Button button, bool press)
		{
			switch (button)
			{
				case win::Button::esc:
					if (press)
						quit = true;
					break;
				default:
					break;
			}
		});

	display.register_window_handler(
		[&quit](win::WindowEvent e)
		{
			if (e == win::WindowEvent::close)
				quit = true;
		});

	display.register_mouse_handler([](int x, int y) {});

	const auto dims = win::Dimensions(display.width(), display.height());
	const auto area = win::Area(-8.0f, 8.0f, -4.5f, 4.5f);
	Renderer renderer(roll, dims, area);

	std::vector<Tree> trees;

	const float zone_shift = 0.075f;
	win::Area zone_area(area.left - 0.5f, area.right + 0.5f, area.bottom - 0.5f, area.top + 0.5f);
	HeatZoneMap hzmap1(zone_area.left, zone_area.right, zone_area.bottom, zone_area.top, 100, 60);
	HeatZoneMap hzmap2(zone_area.left + zone_shift, zone_area.right + zone_shift, zone_area.bottom + zone_shift, zone_area.top + zone_shift, 100, 60);

	std::mt19937 mersenne(69);

	const auto randomf = [&mersenne](float low, float high)
	{
		return std::uniform_real_distribution<float>(low, high)(mersenne);
	};

	const auto randomi = [&mersenne](int low, int high)
	{
		return std::uniform_int_distribution<int>(low, high)(mersenne);
	};

	char debug[100];
	int fps = 0;
	int fps_accum = 0;
	auto last_fps = std::chrono::high_resolution_clock::now();

	int cycle = 0;

	while (!quit)
	{
		++cycle;
		display.process();

		// make new trees!
		const auto make_trees = trees.empty() ? 20'000 : 20;
		for (int i = 0; i < make_trees; ++i)
		{
			trees.emplace_back(randomf(area.left, area.right),
							   randomf(area.bottom, area.top),
							   randomf(Tree::size_low, Tree::size_high),
							   randomf(Tree::size_low, Tree::size_high));
		}

		// lightning that strike!
		if (/*cycle == 300) //*/ randomi(0, 1000))
		{
			const float x = randomf(area.left, area.right);
			const float y = randomf(area.bottom, area.top);

			hzmap1.get_zone(x, y).temp = 1;
		}

		// process them trees!
		int dead_trees = 0;
		for (auto &tree : trees)
		{
			if (tree.dead)
			{
				++dead_trees;
				continue;
			}

			auto &zone1 = hzmap1.get_zone(tree.x, tree.y);
			auto &zone2 = hzmap2.get_zone(tree.x, tree.y);

			if (tree.burnt > 0.0f)
			{
				// burn baby burn
				zone1.temp = std::min(1.0f, zone1.temp + Tree::zone_heat_rate); // heat up the zone
				zone2.temp = std::min(1.0f, zone2.temp + Tree::zone_heat_rate); // heat up the zone

				tree.burnt += Tree::burn_rate;
				if (tree.burnt > 1.0)
					tree.dead = true;
			}
			else
			{
				// potentially light this tree on fire
				if (zone1.temp > 0 || zone2.temp > 0)
					tree.burnt += Tree::burn_rate;
			}
		}

		// delete dead trees!
		if (dead_trees > 1000)
		{
			std::vector<Tree> newtrees;
			newtrees.reserve(trees.size());

			for (const auto &tree : trees)
			{
				if (!tree.dead)
					newtrees.push_back(tree);
			}

			trees = std::move(newtrees);
		}

		// cool off the heat zones!
		for (auto &zone : hzmap1.zones)
			zone.temp = std::max(zone.temp - HeatZone::cool_rate, 0.0f);
		for (auto &zone : hzmap2.zones)
			zone.temp = std::max(zone.temp - HeatZone::cool_rate, 0.0f);

		renderer.draw(trees);

		// #define SHOW_HEAT_ZONES

#if !defined NDEBUG && defined SHOW_HEAT_ZONES
		{
			std::vector<DebugBlock> blocks;
			const HeatZoneMap *maps[] = {&hzmap1, &hzmap2};
			for (const auto *map : maps)
			{
				const float width = map->right - map->left;
				const float height = map->top - map->bottom;
				const float zone_width = width / map->horizontal_zones;
				const float zone_height = height / map->vertical_zones;

				int i = 0;
				for (const auto &zone : map->zones)
				{
					int x = i % map->horizontal_zones;
					int y = i / map->horizontal_zones;

					const win::Color color(zone.temp, 0.0f, 0.3f, 0.2f);
					const float shrink = 0.005f;
					blocks.emplace_back(map->left + (x * zone_width) + shrink,
										(map->bottom + (y * zone_height)) + shrink,
										zone_width - (shrink * 2.0f),
										zone_height - (shrink * 2.0f),
										color);

					++i;
				}
			}

			renderer.draw(blocks);
		}
#endif

		++fps_accum;
		const auto now = std::chrono::high_resolution_clock::now();
		if (std::chrono::duration<float>(now - last_fps).count() > 1.0f)
		{
			fps = fps_accum;
			fps_accum = 0;
			last_fps = now;
		}

		snprintf(debug, sizeof(debug), "%d fps, %lu trees, %d dead trees\n", fps, trees.size(), dead_trees);
		renderer.draw_text(debug, area.left + 0.1f, area.top - 0.2f);

		display.swap();
	}

	return 0;
}
