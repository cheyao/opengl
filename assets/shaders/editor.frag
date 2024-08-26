#version 410 core
precision mediump float;

layout (location = 0) out vec4 color;

uniform bool wireframe;

void main() {
	// If we are painting wireframes
	// We need to set the color to white
	// To make the outline more visible
	if (wireframe) {
		color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	} else {
		color = vec4(0.373f, 0.451f, 0.957f, 0.7f);
	}
} 
