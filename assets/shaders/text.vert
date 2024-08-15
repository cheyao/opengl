#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexPos;

out vec2 vTexPos;

uniform mat4 proj;
uniform mat4 model; // TODO: Maybe offset vec2?

uniform vec2 size;

void main() {
	// gl_Position = proj * model * vec4(aPos.x * size.x, aPos.y * size.y, 0.0f, 1.0f);
	// gl_Position = vec4(vec2(proj * model * vec4(aPos.x * 100, aPos.y * 100, aPos.z, 1.0f)), 0.0f, 1.0f);
	gl_Position = vec4(vec2(proj * model * vec4(aPos, 1.0f)), 0.0f, 1.0f);

	vTexPos = aTexPos;
}
