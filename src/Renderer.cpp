#include <cmath>
#include <type_traits>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Renderer.hpp"

using namespace win::gl;

GLint get_uniform(win::GLProgram &program, const char *name)
{
	const auto loc = glGetUniformLocation(program.get(), name);
	if (loc == -1)
		win::bug("No uniform " + std::string(name));

	return loc;
}

Renderer::Renderer(win::AssetRoll &roll, const win::Dimensions<int> &dims, const win::Area<float> &area)
	: text_renderer(dims, area, font_texture_unit, true, font_ssbo, true)
	, font(text_renderer.create_font(0.2f, roll["font/NotoSansMono-Regular.ttf"]))
{
	printf("%s\n%s\n", (const char *)glGetString(GL_VENDOR), (const char *)glGetString(GL_RENDERER));

	projection = glm::ortho(area.left, area.right, area.bottom, area.top);

	// Initialize tree mode
	{
		treemode.program = win::GLProgram(win::load_gl_shaders(roll["shader/gl/tree.vert"], roll["shader/gl/tree.frag"]));
		glUseProgram(treemode.program.get());
		treemode.uniform_projection = get_uniform(treemode.program, "projection");

		glUniformMatrix4fv(treemode.uniform_projection, 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(treemode.vao.get());

		glBindBuffer(GL_ARRAY_BUFFER, treemode.vbo_verts.get());

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

		glBindBuffer(GL_ARRAY_BUFFER, treemode.vbo_float_instance.get());

		// layout will be { x, y, w, h }

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, NULL);
		glVertexAttribDivisor(1, 1);

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void *)(sizeof(float) * 2));
		glVertexAttribDivisor(2, 1);

		glBindBuffer(GL_ARRAY_BUFFER, treemode.vbo_int_instance.get());

		glEnableVertexAttribArray(3);
		glVertexAttribIPointer(3, 1, GL_UNSIGNED_BYTE, 0, NULL);
		glVertexAttribDivisor(3, 1);
	}

	// initialize debug mode
	{
		debugmode.program = win::GLProgram(win::load_gl_shaders(roll["shader/gl/debug.vert"], roll["shader/gl/debug.frag"]));
		glUseProgram(debugmode.program.get());
		debugmode.uniform_transform = get_uniform(debugmode.program, "transform");
		debugmode.uniform_color = get_uniform(debugmode.program, "color");
	}

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	win::gl_check_error();
}

void Renderer::draw(const std::vector<Tree> &trees)
{
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(treemode.program.get());
	glBindVertexArray(treemode.vao.get());

	std::vector<float> fdata;
	fdata.reserve(trees.size() * 4);

	std::vector<unsigned char> idata;
	idata.reserve(trees.size());

	int count = 0;
	for (const auto &tree : trees)
	{
		if (tree.dead)
			continue;

		fdata.push_back(tree.x);
		fdata.push_back(tree.y);
		fdata.push_back(tree.w);
		fdata.push_back(tree.h);

		idata.push_back(std::roundf(std::max(0.0f, std::min(1.0f, tree.burnt)) * std::numeric_limits<unsigned char>::max()));

		++count;
	}

	glBindBuffer(GL_ARRAY_BUFFER, treemode.vbo_float_instance.get());
	glBufferData(GL_ARRAY_BUFFER, fdata.size(), fdata.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, treemode.vbo_int_instance.get());
	glBufferData(GL_ARRAY_BUFFER, idata.size(), idata.data(), GL_DYNAMIC_DRAW);

	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);

	win::gl_check_error();
}

void Renderer::draw(const std::vector<DebugBlock> &blocks)
{
	glUseProgram(debugmode.program.get());
	glBindVertexArray(debugmode.vao.get());

	const auto ident = glm::identity<glm::mat4>();
	for (const auto &block : blocks)
	{
		const auto translate = glm::translate(ident, glm::vec3(block.x + (block.w / 2.0f), block.y + (block.h / 2.0f), 0.0f));
		const auto scale = glm::scale(ident, glm::vec3(block.w, block.h, 1.0f));
		const auto transform = projection * translate * scale;

		glUniformMatrix4fv(debugmode.uniform_transform, 1, GL_FALSE, glm::value_ptr(transform));

		glUniform4f(debugmode.uniform_color, block.color.red, block.color.green, block.color.blue, block.color.alpha);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	win::gl_check_error();
}

void Renderer::draw_text(const char *str, float x, float y)
{
	text_renderer.draw(font, str, x, y, false);
	text_renderer.flush();
}
