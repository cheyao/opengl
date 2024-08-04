#version 410 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform float time;

in VS_OUT {
	vec3 normal;
	vec3 fragPos;
	vec2 texPos;
} gs_in[];

out VS_OUT {
	vec3 normal;
	vec3 fragPos;
	vec2 texPos;
} gs_out;

vec3 getNormal();
vec4 explode(vec4 position, vec3 normal);

void main() {
	vec3 normal = getNormal();

	gl_Position = explode(gl_in[0].gl_Position, normal);
	gs_out.texPos = gs_in[0].texPos;
	gs_out.fragPos = gs_in[0].fragPos;
	gs_out.normal = gs_in[0].normal;
	EmitVertex();
	gl_Position = explode(gl_in[1].gl_Position, normal);
	gs_out.texPos = gs_in[1].texPos;
	gs_out.fragPos = gs_in[1].fragPos;
	gs_out.normal = gs_in[1].normal;
	EmitVertex();
	gl_Position = explode(gl_in[2].gl_Position, normal);
	gs_out.texPos = gs_in[2].texPos;
	gs_out.fragPos = gs_in[2].fragPos;
	gs_out.normal = gs_in[2].normal;
	EmitVertex();

	EndPrimitive();

	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
	
	EndPrimitive();
}

vec4 explode(vec4 position, vec3 normal) {
	// float magnitude = 2.0;
	// vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude; 
	// vec4 offset = vec4(0.0, 0.0, sin(time), 0.0f);
	return position;// + offset;
}

vec3 getNormal() {
	vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
	vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
	return normalize(cross(a, b));
}
