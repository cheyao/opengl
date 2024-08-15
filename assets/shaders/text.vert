#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexPos;

out vec2 vTexPos;

uniform mat4 proj;
uniform mat4 model; // TODO: Maybe offset vec2?

uniform vec2 size;

void main() {
	gl_Position = proj * model * vec4(aPos.xy, 0.0f, 1.0f);

	vTexPos = aTexPos;
}
