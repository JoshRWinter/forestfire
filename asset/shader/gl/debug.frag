#version 460 core

layout (location = 0) out vec4 frag;

uniform vec4 color;

void main()
{
	frag = color;
}
