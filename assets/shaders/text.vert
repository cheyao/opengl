#version 410 core

layout (location = 0) in vec2 aPos;
layout (location = 2) in vec2 aTexPos;

out vec2 vTexPos;

uniform mat4 proj;
uniform vec2 offset; // TODO: Maybe offset vec2?

uniform vec2 size;

void main() {
	// Ugh all these casts, I got a problem with the ortho matrix
	gl_Position = vec4(vec2(proj * vec4(offset + vec2(aPos.x * size.x, aPos.y * size.y), 0.0f, 1.0f)), 0.0f, 1.0f);

	vTexPos = aTexPos;
}
