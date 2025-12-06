#version 460 core

float[8] verts = float[](-0.5, 0.5, -0.5, -0.5, 0.5, 0.5, 0.5, -0.5);

uniform mat4 transform;

void main()
{
	vec2 pos = vec2(verts[gl_VertexID * 2], verts[(gl_VertexID * 2) + 1]);
	gl_Position = transform * vec4(pos, 0.0, 1.0);
}
