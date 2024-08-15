#version 410 core
precision mediump float;

in vec2 vTexPos;

layout (location = 0) out vec4 color;

uniform sampler2D texture_diffuse0;
uniform vec3 textColor;

void main() {    
	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(texture_diffuse0, vTexPos).r);

	color = vec4(textColor, 1.0) * sampled;
}  
