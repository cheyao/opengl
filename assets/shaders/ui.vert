#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexPos;

out vec2 vTexPos;

uniform mat4 model;
uniform mat4 proj;

void main() {
	// gl_Position = proj * model * vec4(aPos.xy, 0.0f, 1.0f);
	gl_Position = vec4(vec2(proj * model * vec4(aPos, 1.0f)), 0.0f, 1.0f);

	vTexPos = aTexPos;
}
