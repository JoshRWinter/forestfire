#pragma once

#include <random>

#include <win/AssetRoll.hpp>
#include <win/gl/GL.hpp>
#include <win/gl/GLFont.hpp>
#include <win/gl/GLTextRenderer.hpp>
#include <win/Utility.hpp>
#include <win/Win.hpp>

class Renderer
{
	WIN_NO_COPY_MOVE(Renderer);

	static constexpr GLenum font_texture_unit = GL_TEXTURE0;
	static constexpr GLuint font_ssbo = 0;

	static constexpr GLenum noise_texture_unit = GL_TEXTURE1;
	static constexpr GLenum ff1_texture_unit = GL_TEXTURE2;
	static constexpr GLenum ff2_texture_unit = GL_TEXTURE3;
	static constexpr GLenum ffvisual_texture_unit = GL_TEXTURE4;
	static constexpr GLenum fire_a_texture_unit = GL_TEXTURE5;
	static constexpr GLenum fire_b_texture_unit = GL_TEXTURE6;

public:
	Renderer(win::AssetRoll &roll, const win::Dimensions<int> &dims, const win::Area<float> &area);

	void draw(float time);
	void draw_text(const char *str, float x, float y);

private:
	std::mt19937 mersenne;

	win::Dimensions<int> dims;

	struct
	{
		win::GLFramebuffer fbo_ff1, fbo_ff2;

		win::GLTexture ff1, ff2, ffvisual;
		win::GLTexture noise;
		win::GLProgram program;
		int uniform_tcshift;
		int uniform_trees;
		int uniform_strike;
		int uniform_time;

		bool pingpong = true; // if true, draw to ff1, source from ff2

		win::GLVertexArray vao;
	} ffmode;

	struct
	{
		win::GLFramebuffer fbo_a, fbo_b;
		win::GLTexture tex_a, tex_b;

		win::GLProgram program;
		int uniform_tex;
		int uniform_horizontal;

		win::GLVertexArray vao;
	} firemode;

	struct
	{
		win::GLProgram program;

		win::GLVertexArray vao;
	} postmode;

	win::GLTextRenderer text_renderer;
	win::GLFont font;
};
