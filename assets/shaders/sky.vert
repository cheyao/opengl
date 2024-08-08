#version 410 core

layout (location = 0) in vec3 aPos;

out vec3 texPos;

layout(std140) uniform Matrices {
	mat4 proj;
	mat4 view;
};

void main() {
	texPos = aPos;

	vec4 pos = proj * mat4(mat3(view)) * vec4(aPos, 1.0f);
	gl_Position = pos.xyww;
}
