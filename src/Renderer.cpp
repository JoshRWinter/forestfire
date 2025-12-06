#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Renderer.hpp"

using namespace win::gl;

Renderer::Renderer(win::AssetRoll &roll, const win::Dimensions<int> &dims, const win::Area<float> &area)
	: text_renderer(dims, area, font_texture_unit, true, font_ssbo, true)
	, font(text_renderer.create_font(0.2f, roll["font/NotoSansMono-Regular.ttf"]))
{
	program = win::GLProgram(win::load_gl_shaders(roll["shader/gl/v.vert"], roll["shader/gl/f.frag"]));
	glUseProgram(program.get());
	uniform_transform = glGetUniformLocation(program.get(), "transform");
	if (uniform_transform == -1)
		win::bug("No uniform transform");
	uniform_color = glGetUniformLocation(program.get(), "color");
	if (uniform_color == -1)
		win::bug("No uniform color");

	projection = glm::ortho(area.left, area.right, area.bottom, area.top);

	glBindVertexArray(vao.get());
	glBindBuffer(GL_ARRAY_BUFFER, vbo.get());

	// clang-format off
	const float verts[] =
	{
		-0.5f, 0.5f,
		-0.5f, -0.5f,
		0.5f, 0.5f,
		0.5f, -0.5f
	};
	// clang-format on

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	win::gl_check_error();
}

void Renderer::draw(const std::vector<Tree> &trees)
{
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(program.get());
	glBindVertexArray(vao.get());

	const auto ident = glm::identity<glm::mat4>();
	for (const auto &tree : trees)
	{
		if (tree.dead)
			continue;

		const auto translate = glm::translate(ident, glm::vec3(tree.x, tree.y, 0.0f));
		const auto scale = glm::scale(ident, glm::vec3(tree.w, tree.h, 1.0f));
		const auto transform = projection * translate * scale;

		glUniformMatrix4fv(uniform_transform, 1, GL_FALSE, glm::value_ptr(transform));

		// work out the color
		const float burnt = std::max(0.0f, std::min(1.0f, tree.burnt));
		const float red = burnt / 1.5f;
		const float green = (1.0 - burnt) / 2.0f;
		const float blue = 0.0f;

		glUniform4f(uniform_color, red, green, blue, 1.0f);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	win::gl_check_error();
}

void Renderer::draw(const std::vector<DebugBlock> &blocks)
{
	glUseProgram(program.get());
	glBindVertexArray(vao.get());

	const auto ident = glm::identity<glm::mat4>();
	for (const auto &block : blocks)
	{
		const auto translate = glm::translate(ident, glm::vec3(block.x + (block.w / 2.0f), block.y + (block.h / 2.0f), 0.0f));
		const auto scale = glm::scale(ident, glm::vec3(block.w, block.h, 1.0f));
		const auto transform = projection * translate * scale;

		glUniformMatrix4fv(uniform_transform, 1, GL_FALSE, glm::value_ptr(transform));

		glUniform4f(uniform_color, block.color.red, block.color.green, block.color.blue, block.color.alpha);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	win::gl_check_error();
}

void Renderer::draw_text(const char *str, float x, float y)
{
	text_renderer.draw(font, str, x, y, false);
	text_renderer.flush();
}
