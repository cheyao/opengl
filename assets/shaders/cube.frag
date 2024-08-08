#version 410 core
precision mediump float;

const float shininess = 32.0f;

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

#define POINT_LIGHTS 1 // TODO: Non const
uniform PointLight pointLights[POINT_LIGHTS];

in VS_OUT {
	vec3 normal;
	vec3 fragPos;
	vec2 texPos;
} fs_in;

layout (location = 0) out vec4 color;

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main() {
	vec3 viewDir = normalize(viewPos - fs_in.fragPos);

	vec3 outColor = vec3(0);

	for (int i = 0; i < POINT_LIGHTS; i++) {
		outColor += calcPointLight(pointLights[i], fs_in.normal, fs_in.fragPos, viewDir);
	}

	color = vec4(outColor, 1.0f);
}

// TODO: Concat common parts

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
	vec3 lightDir = normalize(light.position - fragPos);

	float diff = max(dot(normal, lightDir), 0.0f);

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(reflectDir, viewDir), 0.0f), shininess);

	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + 
						light.quadratic * (distance * distance));

	vec3 ambient = light.ambient * vec3(texture(texture_diffuse0, fs_in.texPos));
	vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse0, fs_in.texPos));
	vec3 specular = light.specular * spec * vec3(texture(texture_specular0, fs_in.texPos));

	ambient  *= attenuation; 
	diffuse  *= attenuation;
	specular *= attenuation;

	return ambient + diffuse + specular;
}

