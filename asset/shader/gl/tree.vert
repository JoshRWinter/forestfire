#version 460 core

layout (location = 0) in vec2 vert;
layout (location = 1) in vec2 pos;
layout (location = 2) in vec2 size;
layout (location = 3) in int burnt;

uniform mat4 projection;

out int fburnt;

void main()
{
	mat4 model = mat4(size.x, 0.0, 0.0, 0.0, 0.0, size.y, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, pos.x, pos.y, 0.0, 1.0);

	fburnt = burnt;
	gl_Position = projection * model * vec4(vert, 0.0, 1.0);
}
