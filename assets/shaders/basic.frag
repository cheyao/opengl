#version 400 core

in vec3 dstColor;
in vec2 texPos;

uniform sampler2D box;
uniform sampler2D face;
uniform float mixer;

out vec4 color;

void main() {
    color = mix(texture(box, texPos), texture(face, texPos), mixer);
} 
