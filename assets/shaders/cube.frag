#version 410 core
precision mediump float;

uniform vec3 viewPos;

uniform sampler2D texture_diffuse0;
uniform sampler2D texture_specular0;

struct PointLight {
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform PointLight pointLight;

in vec3 vNormal;
in vec3 vFragPos;
in vec2 vTexPos;

layout (location = 0) out vec4 color;

// vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main() {
	vec3 viewDir = normalize(viewPos - vFragPos);

	vec3 lightDir = pointLight.position - vFragPos;
	vec3 lightDirNorm = normalize(lightDir);

	float diff = max(dot(vNormal, lightDirNorm), 0.0f);

	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(halfwayDir, viewDir), 0.0f), 32.0f);

	float distance = length(lightDir);
	float attenuation = 1.0 / (pointLight.constant + pointLight.linear * distance + 
						pointLight.quadratic * (distance * distance));

	vec3 ambient = pointLight.ambient * vec3(texture(texture_diffuse0, vTexPos)) * attenuation;
	vec3 diffuse = pointLight.diffuse * diff * vec3(texture(texture_diffuse0, vTexPos)) * attenuation;
	vec3 specular = pointLight.specular * spec * vec3(texture(texture_specular0, vTexPos)) * attenuation;

	// ambient  *= attenuation; 
	// diffuse  *= attenuation;
	// specular *= attenuation;

	color = vec4(ambient + diffuse + specular, 1.0f);
}

