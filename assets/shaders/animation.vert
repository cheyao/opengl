#version 410 core

layout (location = 0) in vec2 aPos;
layout (location = 2) in vec2 aTexPos;

out vec2 vTexPos;

layout(std140) uniform Matrices {
	mat4 proj;
};

uniform sampler2D texture_diffuse;
uniform vec2 offset;
uniform ivec2 size;
uniform uint select;

uniform bool flip;

void main() {
	// Size of the sprite
	vec2 spriteSize = textureSize(texture_diffuse, 0) * 7 / vec2(size);

	// Player pos
	gl_Position = proj * vec4(aPos * spriteSize + offset, 0, 1);

	// Size of one cell on the texture
	vec2 texSpriteSize = vec2(1.0f, 1.0f) / vec2(size);

	vec2 texPos = aTexPos;
	if (flip) {
		texPos.x = texPos.x * -1.0f + 1.0f;
	}

	vec2 texSpritePos = texSpriteSize * texPos;

	vTexPos = texSpritePos; texSpriteSize * vec2(select % size.x, select / size.x) + texSpritePos;
}
