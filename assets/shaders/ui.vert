#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexPos;

out vec2 vTexPos;

uniform mat4 model;
uniform mat4 proj;
// uniform float width;
// uniform float height;

void main() {
	gl_Position = vec4(vec2(proj * model * vec4(aPos, 1.0f)), 0.0f, 1.0f);
	// vec2 pos = vec2(model * vec4(aPos, 1.0f));
	// gl_Position = vec4(aPos.x / 1024, aPos.y / 768, 0.0f, 1.0f);

	vTexPos = aTexPos;
}
