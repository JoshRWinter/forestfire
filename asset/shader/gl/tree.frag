#version 460 core

layout (location = 0) out vec4 frag;

in flat int fburnt;

void main()
{
	float burnt = fburnt / 255.0;
	float r = burnt;
	float g = 1.0 - burnt;

	frag = vec4(r, g, 0.0, 1.0);
}
