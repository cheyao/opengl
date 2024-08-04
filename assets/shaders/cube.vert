#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexPos;

out VS_OUT {
	vec3 normal;
	vec3 fragPos;
	vec2 texPos;
} vs_out;

layout(std140) uniform Matrices {
	mat4 proj;
	mat4 view;
};
uniform mat4 model;
uniform vec3 offsets[10];

void main() {
	vs_out.normal = normalize(mat3(transpose(inverse(model))) * aNormal);
	vs_out.fragPos = vec3(model * vec4(aPos, 1.0f)) + offsets[gl_InstanceID];
	vs_out.texPos = aTexPos;
	gl_Position = proj * view * vec4(vs_out.fragPos, 1.0f);
}
