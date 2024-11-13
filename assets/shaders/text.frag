#version 410 core
precision mediump float;

in vec2 vTexPos;

layout (location = 0) out vec4 color;

uniform sampler2D letter;
uniform vec3 textColor;

void main() {
	vec4 sampled = texture(letter, vTexPos).rrrr;

	if (sampled.a < 0.1) {
		discard;
	}

	color = vec4(textColor, 1.0) * sampled;
}  
