#version 460 core

layout (location = 0) out vec4 frag;
in vec2 ftexcoord;

uniform sampler2D tex;

float encode(float f)
{
	return f <= 0.0031308 ? 12.92 * f : ((1.055 * pow(f, 1.0 / 2.4)) - 0.055);
}

void main()
{
	frag = texture(tex, ftexcoord);
	frag = vec4(encode(frag.r), encode(frag.g), encode(frag.b), frag.a);

}
