#include <chrono>
#include <random>

#include <win/AssetRoll.hpp>
#include <win/Display.hpp>
#include <win/gl/GL.hpp>
#include <win/Utility.hpp>

#include "Renderer.hpp"
#include "SimulationSettings.hpp"

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
		[&quit, &display, &fullscreen](win::Button button, bool press)
		{
			switch (button)
			{
				case win::Button::esc:
					if (press)
						quit = true;
					break;
				case win::Button::f11:
					if (press)
					{
						fullscreen = !fullscreen;
						display.set_fullscreen(fullscreen);
					}
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

	auto dims = win::Dimensions(display.width(), display.height());
	Renderer renderer(roll, dims);

	display.register_resize_handler(
		[&renderer, &dims](int w, int h)
		{
			dims.width = w;
			dims.height = h;

			renderer.resize(dims);
		});

	const auto start = std::chrono::high_resolution_clock::now();

	// clang-format off
	const SimulationSettings settings =
	{
		0.004f,
		0.001f,
		0.04f,
		3
	};
	// clang-format on

	int fps = 0;
	auto last_fps = std::chrono::high_resolution_clock::now();

	while (!quit)
	{
		display.process();

		const auto time = std::fmodf(std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - start).count() / 1000, 1.0);
		renderer.draw(settings, time);

		++fps;
		const auto now = std::chrono::high_resolution_clock::now();
		if (std::chrono::duration<float>(now - last_fps).count() > 1.0f)
		{
			printf("%d fps\n", fps);
			fps = 0;
			last_fps = now;
		}

		display.swap();
	}

	return 0;
}
