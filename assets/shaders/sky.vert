#version 400 core

layout (location = 0) in vec3 aPos;

out vec3 texPos;

// uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
	texPos = aPos;

	vec4 pos = proj * mat4(mat3(view)) * vec4(aPos, 1.0f);
	gl_Position = pos.xyww;
}
