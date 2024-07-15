#version 400 core
precision mediump float;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
}; 


struct Light {
    vec3 position;
    vec3 direction;
    float cutOff;
	float outerCutOff;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	
    float constant;
    float linear;
    float quadratic;
};

uniform vec3 viewPos;
uniform Light light;
uniform Material material;

in vec3 normal;
in vec3 pos;
in vec2 texPos;

out vec4 color;

void main() {
	vec3 lightDir = normalize(light.position - pos);
	
	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

	// ambient: Const * texture
	vec3 ambient = light.ambient * texture(material.diffuse, texPos).rgb;
	
	// diffuse: diff of normal and lightDir * texture
	vec3 norm = normalize(normal);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, texPos).rgb;  
	
	// specular: Diff of view & light
	vec3 viewDir = normalize(viewPos - pos);
	vec3 reflectDir = reflect(-lightDir, norm);  
	// Real nice when max with 5.0
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * texture(material.specular, texPos).rgb;  
	
	// attenuation: Decrease as length gets farther
	float distance = length(light.position - pos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

	// ambient  *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	diffuse *= intensity;
	specular *= intensity;
		
	vec3 result = ambient + diffuse + specular;
	color = vec4(result, 1.0);
}
	/*
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, texPos));

	vec3 norm = normalize(normal);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, texPos));

	vec3 viewDir = normalize(-pos);
	vec3 lightOut = reflect(-lightDir, norm);
	float spec = pow(max(dot(lightOut, viewDir), 0.0f), material.shininess);
    vec3 specular = light.specular * spec * vec3(texture(material.specular, texPos));

	if (light.position.w == 1.0) {
		float distance    = length(light.position.xyz - pos);
		float attenuation = 1.0 / (light.constant + light.linear * distance + 
						light.quadratic * (distance * distance));

		ambient  *= attenuation; 
		diffuse  *= attenuation;
		specular *= attenuation;
	}

	vec3 result = ambient + diffuse + specular;
    color = vec4(result, 1.0f);
	*/
