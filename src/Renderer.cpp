#include <cmath>
#include <random>

#include "Renderer.hpp"

using namespace win::gl;

GLint get_uniform(win::GLProgram &program, const char *name)
{
	const auto loc = glGetUniformLocation(program.get(), name);
	if (loc == -1)
		win::bug("No uniform " + std::string(name));

	return loc;
}

Renderer::Renderer(win::AssetRoll &roll, const win::Dimensions<int> &dims)
	: mersenne(42'069)
	, dims(dims)
{
	printf("%s\n%s\n", (const char *)glGetString(GL_VENDOR), (const char *)glGetString(GL_RENDERER));

	glViewport(0, 0, dims.width, dims.height);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	glClearColor(0.0, 0.0, 0.0, 0.0);

	// initialize forestfire mode
	{
		// initialize framebuffer nonsense
		{
			// visual texture
			glActiveTexture(ffvisual_texture_unit);
			glBindTexture(GL_TEXTURE_2D, ffmode.ffvisual.get());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, dims.width, dims.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

			const GLenum drawbuffers[] {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
			glBindFramebuffer(GL_FRAMEBUFFER, ffmode.fbo_ff1.get());

			glActiveTexture(ff1_texture_unit);
			glBindTexture(GL_TEXTURE_2D, ffmode.ff1.get());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, dims.width, dims.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ffmode.ff1.get(), 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, ffmode.ffvisual.get(), 0);
			glDrawBuffers(2, drawbuffers);

			glClear(GL_COLOR_BUFFER_BIT);

			glBindFramebuffer(GL_FRAMEBUFFER, ffmode.fbo_ff2.get());

			glActiveTexture(ff2_texture_unit);
			glBindTexture(GL_TEXTURE_2D, ffmode.ff2.get());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, dims.width, dims.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ffmode.ff2.get(), 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, ffmode.ffvisual.get(), 0);
			glDrawBuffers(2, drawbuffers);

			glClear(GL_COLOR_BUFFER_BIT);
		}

		ffmode.program = win::GLProgram(win::load_gl_shaders(roll["shader/gl/ff.vert"], roll["shader/gl/ff.frag"]));
		glUseProgram(ffmode.program.get());
		const auto uniform_noise = get_uniform(ffmode.program, "noise");
		ffmode.uniform_tcshift = get_uniform(ffmode.program, "tcshift");
		ffmode.uniform_trees = get_uniform(ffmode.program, "trees");
		const auto uniform_fire = get_uniform(ffmode.program, "fire");
		ffmode.uniform_strike = get_uniform(ffmode.program, "strike");
		ffmode.uniform_strike_color = get_uniform(ffmode.program, "strike_color");
		ffmode.uniform_time = get_uniform(ffmode.program, "time");
		ffmode.uniform_burn_rate = get_uniform(ffmode.program, "burn_rate");
		ffmode.uniform_fade_out_rate = get_uniform(ffmode.program, "fade_out_rate");
		ffmode.uniform_catch_fire_threshold = get_uniform(ffmode.program, "catch_fire_threshold");
		ffmode.uniform_color_data_len = get_uniform(ffmode.program, "color_data_len");
		ffmode.uniform_fire_color_data_len = get_uniform(ffmode.program, "fire_color_data_len");

		glUniform1i(uniform_noise, noise_texture_unit - GL_TEXTURE0);
		glUniform1i(uniform_fire, fire_b_texture_unit - GL_TEXTURE0);

		// initialize noise texture
		{
			const auto data = generate_treegen_noise();

			glActiveTexture(noise_texture_unit);
			glBindTexture(GL_TEXTURE_2D, ffmode.noise.get());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, dims.width, dims.height, 0, GL_RED, GL_UNSIGNED_BYTE, data.get());
		}

		// set up colors
		{
			// tree colors
			{
				glUniform1i(ffmode.uniform_color_data_len, 0);

				const auto i = glGetProgramResourceIndex(ffmode.program.get(), GL_SHADER_STORAGE_BLOCK, "color_data");
				if (i == GL_INVALID_INDEX)
					win::bug("no buffer color_data");

				glShaderStorageBlockBinding(ffmode.program.get(), i, color_shader_storage_block_index);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, ffmode.colors.get());
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, color_shader_storage_block_index, ffmode.colors.get());
			}

			// fire colors
			{
				glUniform1i(ffmode.uniform_fire_color_data_len, 0);

				const auto i = glGetProgramResourceIndex(ffmode.program.get(), GL_SHADER_STORAGE_BLOCK, "fire_color_data");
				if (i == GL_INVALID_INDEX)
					win::bug("no buffer fire_color_data");

				glShaderStorageBlockBinding(ffmode.program.get(), i, fire_color_shader_storage_block_index);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, ffmode.firecolors.get());
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, fire_color_shader_storage_block_index, ffmode.firecolors.get());
			}
		}
	}

	// initialize fire mode
	{
		const GLenum drawbuffers[] {GL_COLOR_ATTACHMENT0};

		glBindFramebuffer(GL_FRAMEBUFFER, firemode.fbo_a.get());

		glActiveTexture(fire_a_texture_unit);
		glBindTexture(GL_TEXTURE_2D, firemode.tex_a.get());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, dims.width, dims.height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, firemode.tex_a.get(), 0);
		glDrawBuffers(1, drawbuffers);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindFramebuffer(GL_FRAMEBUFFER, firemode.fbo_b.get());

		glActiveTexture(fire_b_texture_unit);
		glBindTexture(GL_TEXTURE_2D, firemode.tex_b.get());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, dims.width, dims.height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, firemode.tex_b.get(), 0);
		glDrawBuffers(1, drawbuffers);
		glClear(GL_COLOR_BUFFER_BIT);

		firemode.program = win::GLProgram(win::load_gl_shaders(roll["shader/gl/fire.vert"], roll["shader/gl/fire.frag"]));
		glUseProgram(firemode.program.get());
		firemode.uniform_tex = get_uniform(firemode.program, "tex");
		firemode.uniform_horizontal = get_uniform(firemode.program, "horizontal");
		firemode.uniform_burn_radius = get_uniform(firemode.program, "burn_radius");
	}

	// initialize post mode
	{
		postmode.program = win::GLProgram(win::load_gl_shaders(roll["shader/gl/post.vert"], roll["shader/gl/post.frag"]));
		glUseProgram(postmode.program.get());
		const auto uniform_tex = get_uniform(postmode.program, "tex");
		glUniform1i(uniform_tex, ffvisual_texture_unit - GL_TEXTURE0);
	}

	win::gl_check_error();
}

void Renderer::resize(const win::Dimensions<int> &dims)
{
	const auto olddims = this->dims;
	this->dims = dims;

	const auto data = generate_treegen_noise();
	glActiveTexture(noise_texture_unit);
	glBindTexture(GL_TEXTURE_2D, ffmode.noise.get());
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, dims.width, dims.height, 0, GL_RED, GL_UNSIGNED_BYTE, data.get());

	{
		glActiveTexture(ff1_texture_unit);
		glBindTexture(GL_TEXTURE_2D, ffmode.ff1.get());

		const std::unique_ptr<float[]> olddata(new float[olddims.width * olddims.height * 4]);
		glFlush();
		glFinish();
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, olddata.get());

		std::unique_ptr<float[]> newdata(new float[dims.width * dims.height * 4]);
		for (int i = 0; i < dims.width * dims.height * 4; ++i)
			newdata[i] = 0.0f;

		for (int x = 0; x < std::min(olddims.width, dims.width); ++x)
		{
			for (int y = 0; y < std::min(olddims.height, dims.height); ++y)
			{
				const auto oldindex = y * olddims.width + x;
				const auto newindex = y * dims.width + x;

				newdata[newindex * 4] = olddata[oldindex * 4];
				newdata[newindex * 4 + 1] = olddata[oldindex * 4 + 1];
				newdata[newindex * 4 + 2] = olddata[oldindex * 4 + 2];
				newdata[newindex * 4 + 3] = olddata[oldindex * 4 + 3];
			}
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, dims.width, dims.height, 0, GL_RGBA, GL_FLOAT, newdata.get());

		glActiveTexture(ff2_texture_unit);
		glBindTexture(GL_TEXTURE_2D, ffmode.ff2.get());
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, dims.width, dims.height, 0, GL_RGBA, GL_FLOAT, newdata.get());
	}

	glActiveTexture(ffvisual_texture_unit);
	glBindTexture(GL_TEXTURE_2D, ffmode.ffvisual.get());
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, dims.width, dims.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	// resize fire textures
	{
		glActiveTexture(fire_a_texture_unit);
		glBindTexture(GL_TEXTURE_2D, firemode.tex_a.get());
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, dims.width, dims.height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
		glBindFramebuffer(GL_FRAMEBUFFER, firemode.fbo_a.get());
		glClear(GL_COLOR_BUFFER_BIT);

		glActiveTexture(fire_b_texture_unit);
		glBindTexture(GL_TEXTURE_2D, firemode.tex_b.get());
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, dims.width, dims.height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
		glBindFramebuffer(GL_FRAMEBUFFER, firemode.fbo_b.get());
		glClear(GL_COLOR_BUFFER_BIT);
	}

	glViewport(0, 0, dims.width, dims.height);

	win::gl_check_error();
}

void Renderer::draw(float time)
{
	// do tree simulation
	{
		glBindFramebuffer(GL_FRAMEBUFFER, ffmode.pingpong ? ffmode.fbo_ff1.get() : ffmode.fbo_ff2.get());

		glUseProgram(ffmode.program.get());
		glUniform1i(ffmode.uniform_trees, (ffmode.pingpong ? ff2_texture_unit : ff1_texture_unit) - GL_TEXTURE0);
		glBindVertexArray(ffmode.vao.get());

		glUniform2f(ffmode.uniform_tcshift,
					std::uniform_real_distribution<float>(-10.0f, 10.0f)(mersenne),
					std::uniform_real_distribution<float>(-10.0f, 10.0f)(mersenne));

		// lightning strike?
		if (std::uniform_int_distribution<int>(0, 300)(mersenne) == 0) // || true)
		{
			glUniform1i(ffmode.uniform_strike_color, std::uniform_int_distribution<int>(0, 1)(mersenne));
			glUniform2i(ffmode.uniform_strike,
						std::uniform_int_distribution<int>(0, dims.width)(mersenne),
						std::uniform_int_distribution<int>(0, dims.height)(mersenne));
		}
		else
		{
			glUniform2i(ffmode.uniform_strike, -1, -1);
		}

		glUniform1f(ffmode.uniform_burn_rate, settings.burn_rate);
		glUniform1f(ffmode.uniform_fade_out_rate, settings.fade_out_rate);
		glUniform1f(ffmode.uniform_catch_fire_threshold, settings.catch_fire_threshold);

		glUniform1f(ffmode.uniform_time, time);

		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	// do fire simulation
	{
		glBindFramebuffer(GL_FRAMEBUFFER, firemode.fbo_a.get());

		glBindVertexArray(firemode.vao.get());

		glUseProgram(firemode.program.get());

		glUniform1i(firemode.uniform_tex, (ffmode.pingpong ? ff1_texture_unit : ff2_texture_unit) - GL_TEXTURE0);
		glUniform1i(firemode.uniform_horizontal, 1);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		glBindFramebuffer(GL_FRAMEBUFFER, firemode.fbo_b.get());

		glUniform1i(firemode.uniform_tex, fire_a_texture_unit - GL_TEXTURE0);
		glUniform1i(firemode.uniform_horizontal, 0);
		glUniform1i(firemode.uniform_burn_radius, settings.burn_radius);

		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	// post processing
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glUseProgram(postmode.program.get());
		glBindVertexArray(postmode.vao.get());

		glClear(GL_COLOR_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	ffmode.pingpong = !ffmode.pingpong;

	win::gl_check_error();
}

void Renderer::set_settings(const SimulationSettings &settings)
{
	this->settings = settings;

	auto decode = [](unsigned char c)
	{
		const float norm = c / 255.0f;
		return norm <= 0.04045f ? norm / 12.92f : std::powf((norm + 0.055f) / 1.055f, 2.4f);
	};

	glBindVertexArray(ffmode.vao.get());
	glUseProgram(ffmode.program.get());

	{
		std::unique_ptr<float[]> colors(new float[settings.tree_colors.size() * 4]);
		for (int i = 0; i < settings.tree_colors.size(); ++i)
		{
			colors[i * 4] = decode(settings.tree_colors[i].red);
			colors[i * 4 + 1] = decode(settings.tree_colors[i].green);
			colors[i * 4 + 2] = decode(settings.tree_colors[i].blue);
			colors[i * 4 + 3] = 0.0f;
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ffmode.colors.get());
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * settings.tree_colors.size() * 4, colors.get(), GL_STATIC_DRAW);
		glUniform1i(ffmode.uniform_color_data_len, settings.tree_colors.size());
	}

	{
		std::unique_ptr<float[]> colors(new float[settings.fire_colors.size() * 4]);
		for (int i = 0; i < settings.fire_colors.size(); ++i)
		{
			colors[i * 4] = decode(settings.fire_colors[i].red);
			colors[i * 4 + 1] = decode(settings.fire_colors[i].green);
			colors[i * 4 + 2] = decode(settings.fire_colors[i].blue);
			colors[i * 4 + 3] = 0.0f;
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ffmode.firecolors.get());
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * settings.fire_colors.size() * 4, colors.get(), GL_STATIC_DRAW);
		glUniform1i(ffmode.uniform_fire_color_data_len, settings.fire_colors.size());
	}
}

std::unique_ptr<unsigned char[]> Renderer::generate_treegen_noise()
{
	std::unique_ptr<unsigned char[]> data(new unsigned char[dims.width * dims.height]);
	for (int i = 0; i < dims.width * dims.height; ++i)
	{
		data[i] = std::uniform_int_distribution<int>(0, 50'000)(mersenne) == 0 ? 255 : 0;
	}

	return data;
}
