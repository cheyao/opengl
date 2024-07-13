#version 400 core
precision mediump float;

in vec3 normal;
in vec3 pos;
in vec3 lightPos;

out vec4 color;

uniform vec3 objectColor;
uniform vec3 lightColor;

void main() {
	const float ambientStrength = 0.1f;
	vec3 ambient = ambientStrength * lightColor;

	vec3 norm = normalize(normal);
	vec3 dir = normalize(lightPos - pos);
	
	float diff = max(dot(norm, dir), 0.0f);
	vec3 diffuse = diff * lightColor;

	float specularStrength = 0.5f;
	vec3 viewDir = normalize(-pos);
	vec3 lightOut = reflect(-dir, norm);
	float spec = pow(max(dot(lightOut, viewDir), 0.0f), 32);
    vec3 specular = specularStrength * spec * lightColor;

	vec3 result = (ambient + diffuse + specular) * objectColor;
    color = vec4(result, 1.0f);
} 
