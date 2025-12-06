#pragma once

#include <vector>

#include <glm/glm.hpp>

#include <win/AssetRoll.hpp>
#include <win/gl/GL.hpp>
#include <win/gl/GLFont.hpp>
#include <win/gl/GLTextRenderer.hpp>
#include <win/Utility.hpp>
#include <win/Win.hpp>

#include "Entities.hpp"

class Renderer
{
	WIN_NO_COPY_MOVE(Renderer);

	static constexpr GLenum font_texture_unit = GL_TEXTURE0;
	static constexpr GLuint font_ssbo = 0;

public:
	Renderer(win::AssetRoll &roll, const win::Dimensions<int> &dims, const win::Area<float> &area);

	void draw(const std::vector<Tree> &trees);
	void draw(const std::vector<DebugBlock> &blocks);
	void draw_text(const char *str, float x, float y);

private:
	win::GLProgram program;
	int uniform_transform;
	int uniform_color;

	glm::mat4 projection;

	win::GLVertexArray vao;
	win::GLBuffer vbo;

	win::GLTextRenderer text_renderer;
	win::GLFont font;
};
