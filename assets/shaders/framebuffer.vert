#version 410 core

layout (location = 0) in vec2 aPos;

out vec2 texPos;

void main() {
	gl_Position = vec4(aPos, 0.0f, 1.0f);
	texPos = vec2((aPos.x + 1.0f) / 2.0f, (aPos.y + 1.0f) / 2.0f);
}
