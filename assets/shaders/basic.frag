#version 400 core

in vec3 dstColor;

out vec4 color;

void main() {
    color = vec4(dstColor, 1.0);
} 
