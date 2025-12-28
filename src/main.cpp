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
#ifdef WINPLAT_WINDOWS
	{
		const std::string cmdstring = cmd;
		if (cmdstring.size() >= 4 && cmdstring.at(0) == '/' && cmdstring.at(1) == 'p' && cmdstring.at(2) == ' ')
		{
			unsigned long long u;
			if (sscanf(cmdstring.substr(3).c_str(), "%llu", &u) != 1)
				win::bug("Couldn't parse integer from " + cmdstring);

			display_options.parent = (HWND)u;
		}
	}
#endif
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
				case win::Button::space:
					display.vsync(!press);
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

	auto dims = win::Dimensions(display.width(), display.height());
	Renderer renderer(roll, dims);

	display.register_resize_handler(
		[&renderer, &dims](int w, int h)
		{
			dims.width = w;
			dims.height = h;

			renderer.resize(dims);
		});

	// clang-format off
	const SimulationSettings settings =
	{
		0.004f,
		0.001f,
		0.04f,
		3,
		{
			win::Color<unsigned char>(255, 0, 0),
			win::Color<unsigned char>(0, 255, 0),
			win::Color<unsigned char>(0, 0, 255),
			win::Color<unsigned char>(255, 255, 0),
			win::Color<unsigned char>(255, 0, 255),
			win::Color<unsigned char>(0, 255, 255)
		},
		{
			win::Color<unsigned char>(0xd5, 0xd1, 0xe9),
			win::Color<unsigned char>(0xd0, 0xe4, 0xee),
			win::Color<unsigned char>(0xf3, 0xf5, 0xa9),
			win::Color<unsigned char>(0xf5, 0xcf, 0x9f),
			win::Color<unsigned char>(0xf5, 0xa7, 0xa6)
		},
		{
			"texture/tree.tga",
			"texture/package.tga",
			"texture/candycane.tga"
		}
	};
	// clang-format on

	int fps = 0;
	auto last_fps = std::chrono::high_resolution_clock::now();
	renderer.set_settings(settings);

	while (!quit)
	{
		display.process();

		renderer.draw();

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
