#version 410 core

layout (location = 0) in vec2 aPos;
layout (location = 3) in ivec3 data;

out vec2 vTexPos;

layout(std140) uniform Matrices {
	mat4 proj;
};
uniform vec2 offset;

void main() {
	vec2 pos = (aPos + vec2(data.xy)) * vec2(16.0f, 16.0f) * 7.0f + offset;
	gl_Position = proj * vec4(pos, 0.0f, 1.0f);

	// One block is this size
	vec2 texSpriteSize = vec2(1.0f, 1.0f) / vec2(64.0f, 64.0f);
	vec2 texPos = aPos;
	vec2 texSpritePos = texSpriteSize * texPos;

	vTexPos = texSpriteSize * vec2(data.z % 64, data.z / 64) + texSpritePos;
}
