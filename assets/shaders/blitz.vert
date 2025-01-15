#version 410 core

layout (location = 0) in vec2 aPos;

out vec2 vTexPos;

layout(std140) uniform Matrices {
	mat4 proj;
};
uniform vec2 offset;
uniform int position;
uniform sampler2D texture_diffuse;

void main() {
	vec2 offset = vec2(position % 64 * 16, position / 64 * 16);

	vec2 pos = aPos * vec2(textureSize(texture_diffuse, 0)) + offset;

	gl_Position = proj * vec4(pos, 0.0f, 1.0f);

	vTexPos = vec2(aPos.x, aPos.y * -1.0f + 1.0f);
}
