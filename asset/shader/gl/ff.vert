#version 460 core

const float[6] verts = float[](-1.0, 3.0, -1.0, -1.0, 3.0, -1.0);
const float[6] tc = float[](0.0, 2.0, 0.0, 0.0, 2.0, 0.0);

out vec2 noisetexcoord;
out vec2 ftexcoord;

uniform vec2 tcshift;

void main()
{
	noisetexcoord = vec2(tc[(gl_VertexID * 2) + 0] + tcshift.s, tc[(gl_VertexID * 2) + 1] + tcshift.t);
	ftexcoord = vec2(tc[(gl_VertexID * 2) + 0], tc[(gl_VertexID * 2) + 1]);
	gl_Position = vec4(verts[(gl_VertexID * 2) + 0], verts[(gl_VertexID * 2) + 1], 0.0, 1.0);
}
