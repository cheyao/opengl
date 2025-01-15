#version 410 core

layout (location = 0) in vec2 aPos;

out vec2 vTexPos;

layout(std140) uniform Matrices {
	mat4 proj;
};
uniform sampler2D letter;
uniform vec2 offset;

void main() {
	vec2 size = vec2(textureSize(letter, 0));
	gl_Position = proj * vec4(offset + aPos * size, 0.0f, 1.0f);

	vTexPos = vec2(aPos.x, aPos.y * -1.0f + 1.0f);
}
