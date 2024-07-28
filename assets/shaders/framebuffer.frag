#version 400 core
precision mediump float;

in vec2 texPos;

out vec4 color;

uniform sampler2D screen;

uniform int width;
uniform int height;

void main() {
	vec2 fragCoord = gl_FragCoord.xy / vec2(width, height);
	vec2 center = vec2(0.5, 0.5); 
	float d = length(fragCoord - center);
	vec2 uv = texPos;
	
	uv = uv * 2.0f - 1.0f; // 4 textures
	vec2 offset = uv.yx / 5.0f; // offset?
	uv = uv + uv * offset * offset;
	uv = uv * 0.5f + 0.5f;

	if (int(gl_FragCoord.x) % 2 == 0) {
		color = vec4(0.0f, 0.0f, 0.0f, 1.0f);

		// return;
	}
	if (int(gl_FragCoord.y) % 2 == 0) {
		color = vec4(0.0f, 0.0f, 0.0f, 1.0f);

		// return;
	}

	color = vec4(vec3(texture(screen, uv)), 1.0f);
} 
