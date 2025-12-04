#include <chrono>
#include <random>
#include <vector>

#include <win/AssetRoll.hpp>
#include <win/Display.hpp>
#include <win/gl/GL.hpp>
#include <win/Utility.hpp>

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
	trees.reserve(10'000);

	std::mt19937 mersenne(69);

	const auto random = [&mersenne](float low, float high)
	{
		return std::uniform_real_distribution<float>(low, high)(mersenne);
	};

	char debug[100];
	int fps = 0;
	int fps_accum = 0;
	auto last_fps = std::chrono::high_resolution_clock::now();

	while (!quit)
	{
		display.process();

		for (int i = 0; i < 40; ++i)
			trees.emplace_back(random(area.left, area.right),
							   random(area.bottom, area.top),
							   random(Tree::WIDTH_LOW, Tree::WIDTH_HIGH),
							   random(Tree::WIDTH_LOW, Tree::WIDTH_HIGH));

		renderer.draw(trees);

		++fps_accum;
		const auto now = std::chrono::high_resolution_clock::now();
		if (std::chrono::duration<float>(now - last_fps).count() > 1.0f)
		{
			fps = fps_accum;
			fps_accum = 0;
			last_fps = now;
		}

		snprintf(debug, sizeof(debug), "%d fps, %lu trees", fps, trees.size());
		renderer.draw_text(debug, area.left + 0.1f, area.top - 0.25f);

		display.swap();
	}

	return 0;
}
