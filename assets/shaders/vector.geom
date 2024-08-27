#version 410 core

layout (triangles) in;

layout (line_strip, max_vertices = 2) out;

uniform vec2 position;
uniform vec2 velocity;
uniform float magnitude;

layout(std140) uniform Matrices {
	mat4 proj;
	mat4 view;
};

void main() {
	gl_Position = proj * vec4(position, 0.0f, 1.0f);
	EmitVertex();
	gl_Position = proj * vec4(position + velocity * magnitude, 0.0f, 1.0f);
	EmitVertex();
	EndPrimitive();
}
