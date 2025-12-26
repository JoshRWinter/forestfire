#version 460 core

const float[8] verts = float[8](-1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0, -1.0);
const float[8] tc = float[8](0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.0);

out vec2 noisetexcoord;
out vec2 patterntexcoord;

uniform vec2 tcshift;
uniform float patternsqueeze;

void main()
{
	float[8] patterntc = float[8](-0.1 - patternsqueeze, 1.1, -0.1 - patternsqueeze, -0.1, 1.1 + patternsqueeze, 1.1, 1.1 + patternsqueeze, -0.1);

	noisetexcoord = vec2(tc[gl_VertexID * 2 + 0] + tcshift.s, tc[gl_VertexID * 2 + 1] + tcshift.t);
	patterntexcoord = vec2(patterntc[gl_VertexID * 2 + 0], patterntc[gl_VertexID * 2 + 1]);
	gl_Position = vec4(verts[gl_VertexID * 2 + 0], verts[gl_VertexID * 2 + 1], 0.0, 1.0);
}
