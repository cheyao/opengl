#version 400 core
precision mediump float;

const float shininess = 32.0f;

uniform vec3 viewPos;

uniform sampler2D texture_diffuse0;
uniform sampler2D texture_specular0;
uniform samplerCube texture_diffuse1;

struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight {
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct SpotLight {
	vec3 position;
	vec3 direction;
	float cutOff;
	float outerCutOff;

	float constant;
	float linear;
	float quadratic;  

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform DirLight dirLight;
// #define POINT_LIGHTS 0  
// uniform PointLight pointLights[POINT_LIGHTS];
uniform SpotLight spotLight;

in vec3 normal;
in vec3 fragPos;
in vec2 texPos;

out vec4 color;

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 cubeReflect(vec3 normal, vec3 viewDir);
vec3 cubeRefract(vec3 normal, vec3 viewDir);

void main() {
	vec3 norm = normalize(normal);
	vec3 viewDir = normalize(viewPos - fragPos);

	vec3 outColor = vec3(0);

	/*
	outColor += calcDirLight(dirLight, norm, viewDir);
	// for (int i = 0; i < POINT_LIGHTS; i++) {
	// 	outColor += calcPointLight(pointLights[i], norm, fragPos, viewDir);
	// }
	*/
	// outColor += calcSpotLight(spotLight, norm, fragPos, viewDir);
	// FIXME: Cannot be added together?

	// outColor += cubeReflect(norm, viewDir);
	outColor += cubeRefract(norm, viewDir);

	color = vec4(outColor, 1.0f);
}

// TODO: Concat common parts

vec3 cubeReflect(vec3 normal, vec3 viewDir) {
	vec3 reflection = reflect(-viewDir, normal);
	return vec3(texture(texture_diffuse1, reflection));
}

vec3 cubeRefract(vec3 normal, vec3 viewDir) {
	float ratio = 1.00f / 1.52f;
	vec3 refration = refract(-viewDir, normal, ratio);
	return vec3(texture(texture_diffuse1, refration));
}

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
	vec3 lightDir = normalize(-light.direction);

	float diff	= max(dot(normal, lightDir), 0.0f) * 1.9;

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec	= pow(max(dot(viewDir, reflectDir), 0.0f), shininess);

	vec3 ambient  = light.ambient * vec3(texture(texture_diffuse0, texPos));
	vec3 diffuse  = light.diffuse * diff * vec3(texture(texture_diffuse0, texPos));
	vec3 specular = light.specular * spec * vec3(texture(texture_specular0, texPos));

	return ambient + diffuse + specular;
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
	vec3 lightDir = normalize(light.position - fragPos);

	float diff = max(dot(normal, lightDir), 0.0f);

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(reflectDir, viewDir), 0.0f), shininess);

	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + 
						light.quadratic * (distance * distance));

	vec3 ambient = light.ambient * vec3(texture(texture_diffuse0, texPos));
	vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse0, texPos));
	vec3 specular = light.specular * spec * vec3(texture(texture_specular0, texPos));

	ambient  *= attenuation; 
	diffuse  *= attenuation;
	specular *= attenuation;

	return ambient + diffuse + specular;
}

vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
	vec3 lightDir = normalize(light.position - fragPos);

	float diff = max(dot(normal, lightDir), 0.0);
	
	// specular: Diff of view & light
	vec3 reflectDir = reflect(-lightDir, normal);  
	// Real nice when max with 5.0
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	
	// attenuation: Decrease as length gets farther
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));	
	
	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	
	vec3 ambient = light.ambient * texture(texture_diffuse0, texPos).rgb;
	vec3 diffuse = light.diffuse * diff * texture(texture_diffuse0, texPos).rgb;  
	vec3 specular = light.specular * spec * texture(texture_specular0, texPos).rgb;  

	return (ambient + diffuse + specular) * attenuation * intensity;
}
