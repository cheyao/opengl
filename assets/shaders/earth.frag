#version 410 core
precision mediump float;

uniform vec3 viewPos;

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

in VS_OUT {
	vec3 normal;
	vec3 fragPos;
} fs_in;

layout (location = 0) out vec4 color;

const float shininess = 32.0f;

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main() {
	vec3 viewDir = normalize(viewPos - fs_in.fragPos);

	color = vec4(calcPointLight(pointLight, fs_in.normal, fs_in.fragPos, viewDir), 1.0f);
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
	vec3 lightDir = normalize(light.position - fragPos);

	float diff = max(dot(normal, lightDir), 0.0f);

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(reflectDir, viewDir), 0.0f), shininess);

	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + 
						light.quadratic * (distance * distance));

	vec3 ambient = light.ambient * vec3(0.0f, 0.411764705882f, 0.580392156863f);
	vec3 diffuse = light.diffuse * diff * vec3(0.0f, 0.411764705882f, 0.580392156863f);
	vec3 specular = light.specular * spec * vec3(0.0f, 0.411764705882f, 0.580392156863f);

	ambient  *= attenuation; 
	diffuse  *= attenuation;
	specular *= attenuation;

	return ambient + diffuse + specular;
}

