#include <chrono>
#include <atomic>
#include <vector>
#include <thread>

#include <win/AssetRoll.hpp>
#include <win/Display.hpp>
#include <win/gl/GL.hpp>
#include <win/Utility.hpp>
#include <win/MonitorEnumerator.hpp>

#include "Renderer.hpp"
#include "SimulationSettings.hpp"

std::vector<std::thread> make_secondary_displays(std::atomic<bool> &allstop);

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

	std::atomic<bool> allstop = false;

#if defined WINPLAT_WINDOWS && SCREENSAVER
	unsigned long long parentwindow = 0;

#if defined NDEBUG
	const std::string cmdstring = cmd;
	if (cmdstring.size() >= 4 && cmdstring.at(0) == '/' && cmdstring.at(1) == 'p' && cmdstring.at(2) == ' ')
	{
		if (sscanf(cmdstring.substr(3).c_str(), "%llu", &parentwindow) != 1)
			win::bug("Couldn't parse integer from " + cmdstring);

	}
#else
	if (argc == 3 && !strcmp(argv[1], "/p"))
	{
		if (sscanf(argv[2], "%llu", &parentwindow) != 1)
			win::bug("Couldn't parse integer from " + std::string(argv[2]));
	}
#endif
	if (parentwindow != 0)
		display_options.parent = (HWND)parentwindow;

	auto secondary_displays = make_secondary_displays(allstop);
#endif

	win::Display display(display_options);
	display.vsync(true);
#ifdef SCREENSAVER
	display.cursor(false);
#endif
	bool fullscreen = display_options.fullscreen;

	win::load_gl_functions();

	display.register_button_handler([&allstop, &display, &fullscreen](win::Button button, bool press)
	{
		switch (button)
		{
			case win::Button::space:
				display.vsync(!press);
				break;
#ifdef SCREENSAVER
			default:
				if (press)
					allstop.store(true);
				break;
#else
			case win::Button::esc:
				allstop = true;
				break;
			case win::Button::f11:
				if (press)
				{
					fullscreen = !fullscreen;
					display.set_fullscreen(fullscreen);
				}
			default:
				break;
#endif
			}
		});

	display.register_window_handler(
		[&allstop](win::WindowEvent e)
		{
			if (e == win::WindowEvent::close)
				allstop = true;
		});

#ifdef SCREENSAVER
	int mousex = -1, mousey = -1;
	display.register_mouse_handler([&allstop, &mousex, &mousey](int x, int y)
	{
		if (mousex == -1)
		{
			mousex = x;
			mousey = y;
		}
		else if (x != mousex || y != mousey)
		{
			allstop.store(true);
		}
	});
#endif

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

	while (!allstop.load())
	{
		display.process();

		renderer.draw();

		++fps;
		const auto now = std::chrono::high_resolution_clock::now();
		if (std::chrono::duration<float>(now - last_fps).count() > 1.0f)
		{
			//printf("%d fps\n", fps);
			fps = 0;
			last_fps = now;
		}

		display.swap();
	}

#ifdef SCREENSAVER
	for (auto &t : secondary_displays)
		t.join();
#endif

	return 0;
}

std::vector<std::thread> make_secondary_displays(std::atomic<bool> &allstop)
{
	auto callback = [](std::atomic<bool> &stop, std::string monitor)
	{
		win::DisplayOptions options;
		options.monitor_name = monitor;
		options.gl_major = 3;
		options.gl_minor = 3;
		options.fullscreen = true;
		options.caption = "ForestFire";
		options.caption = "ForestFire";

		win::Display display(options);
		display.cursor(false);

		display.register_button_handler([&stop](win::Button button, bool press)
		{
			if (press)
				stop.store(true);
		});

		display.register_window_handler(
			[&stop](win::WindowEvent e)
			{
				if (e == win::WindowEvent::close)
					stop.store(true);
			});

		int mousex = -1, mousey = -1;
		display.register_mouse_handler([&stop, &mousex, &mousey](int x, int y)
		{
			if (mousex == -1)
			{
				mousex = x;
				mousey = y;
			}
			else if (mousex != x || mousey != y)
			{
				stop.store(true);
			}
		});

		glClearColor(0.0, 0.0, 0.0, 0.0);

		while (!stop.load())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			display.process();

			glClear(GL_COLOR_BUFFER_BIT);

			display.swap();
		}
	};

	std::vector<std::thread> threads;
	win::MonitorEnumerator monitors;

	for (const auto &m : monitors)
		if (!m.primary)
			threads.emplace_back(callback, std::ref(allstop), m.id);

	return threads;
}
