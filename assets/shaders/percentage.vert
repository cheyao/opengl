#version 410 core

layout (location = 0) in vec2 aPos;
layout (location = 2) in vec2 aTexPos;

out vec2 vTexPos;

layout(std140) uniform Matrices {
	mat4 proj;
};

uniform vec2 offset;
uniform vec2 size;
uniform float percent;
uniform bool vertical;

void main() {
	vec2 pos = aPos * size;

	vTexPos = aTexPos;
	if (vertical) {
		pos.y *= percent;

		// Reverse the texture position
		vTexPos.y = 1.0f - vTexPos.y;
		vTexPos.y *= percent;
		vTexPos.y = 1.0f - vTexPos.y;
	} else {
		pos.x *= percent;

		vTexPos.x *= percent;
	}

	pos += offset;

	gl_Position = proj * vec4(pos, 0.0f, 1.0f);
}
