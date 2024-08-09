#version 410 core
precision mediump float;

const float shininess = 32.0f;

uniform vec3 viewPos;

uniform sampler2D texture_diffuse0;
uniform sampler2D texture_specular0;

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
#define POINT_LIGHTS 1 // TODO: Non const
uniform PointLight pointLights[POINT_LIGHTS];
uniform SpotLight spotLight;

in VS_OUT {
	vec3 normal;
	vec3 fragPos;
	vec2 texPos;
} fs_in;

layout (location = 0) out vec4 color;

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
// vec3 cubeReflect(vec3 normal, vec3 viewDir);
// vec3 cubeRefract(vec3 normal, vec3 viewDir);

void main() {
	vec3 viewDir = normalize(viewPos - fs_in.fragPos);

	vec3 outColor = vec3(0);

	// outColor += calcDirLight(dirLight, fs_in.normal, viewDir);
	for (int i = 0; i < pointLights.length(); i++) {
		outColor += calcPointLight(pointLights[i], fs_in.normal, fs_in.fragPos, viewDir);
	}
	// outColor += calcSpotLight(spotLight, fs_in.normal, fs_in.fragPos, viewDir);

	// outColor += cubeReflect(norm, viewDir);
	// outColor += cubeRefract(fs_in.normal, viewDir);

	color = vec4(outColor, 1.0f);
}

// TODO: Concat common parts

/*
vec3 cubeReflect(vec3 normal, vec3 viewDir) {
	vec3 reflection = reflect(-viewDir, normal);
	return vec3(texture(texture_diffuse1, reflection));
}

vec3 cubeRefract(vec3 normal, vec3 viewDir) {
	const float ratio = 1.00f / 1.52f;
	vec3 refration = refract(-viewDir, normal, ratio);
	return vec3(texture(texture_diffuse1, refration));
}
*/

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
	vec3 lightDir = normalize(-light.direction);

	float diff = max(dot(normal, lightDir), 0.0f) * 1.9;

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);

	vec3 ambient  = light.ambient * vec3(texture(texture_diffuse0, fs_in.texPos));
	vec3 diffuse  = light.diffuse * diff * vec3(texture(texture_diffuse0, fs_in.texPos));
	vec3 specular = light.specular * spec * vec3(texture(texture_specular0, fs_in.texPos));

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

	vec3 ambient = light.ambient * vec3(texture(texture_diffuse0, fs_in.texPos));
	vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse0, fs_in.texPos));
	vec3 specular = light.specular * spec * vec3(texture(texture_specular0, fs_in.texPos));

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
	
	vec3 ambient = light.ambient * texture(texture_diffuse0, fs_in.texPos).rgb;
	vec3 diffuse = light.diffuse * diff * texture(texture_diffuse0, fs_in.texPos).rgb;  
	vec3 specular = light.specular * spec * texture(texture_specular0, fs_in.texPos).rgb;  

	return (ambient + diffuse + specular) * attenuation * intensity;
}
