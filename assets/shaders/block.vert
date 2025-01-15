#version 410 core

layout (location = 0) in vec2 aPos;
layout (location = 2) in vec2 aTexPos;

out vec2 vTexPos;

layout(std140) uniform Matrices {
	mat4 proj;
};
uniform vec2 offset;
uniform float scale;
uniform int select;

uniform ivec2 position;

void main() {
	vec2 pos = (aPos * scale + vec2(position)) * vec2(16, 16) * 7.0f + offset;
	gl_Position = proj * vec4(pos, 0.0f, 1.0f);

	// One block is this size
	vec2 texSpriteSize = vec2(1.0f, 1.0f) / vec2(64.0f, 64.0f);
	vec2 texPos = aTexPos;
	texPos.y = texPos.y * -1.0f + 1.0f;
	vec2 texSpritePos = texSpriteSize * texPos;

	vTexPos = texSpriteSize * vec2(select % 64, select / 64) + texSpritePos;
}
