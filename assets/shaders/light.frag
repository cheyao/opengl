#version 400 core
precision mediump float;

out vec4 color;

uniform vec3 aColor;

void main() {
    color = vec4(aColor, 1.0f); // vec4(1.0);
} 
