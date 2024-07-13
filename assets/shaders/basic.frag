#version 400 core
precision mediump float;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 
  
uniform Material material;


struct Light {
    vec3 position;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Light light;  

in vec3 normal;
in vec3 pos;
in vec3 lightPos;

out vec4 color;

void main() {
	vec3 ambient = light.ambient * material.ambient;

	vec3 norm = normalize(normal);
	vec3 dir = normalize(lightPos - pos);
	
	float diff = max(dot(norm, dir), 0.0f);
	vec3 diffuse = light.diffuse * (diff * material.diffuse);

	vec3 viewDir = normalize(-pos);
	vec3 lightOut = reflect(-dir, norm);
	float spec = pow(max(dot(lightOut, viewDir), 0.0f), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);

	vec3 result = ambient + diffuse + specular;
    color = vec4(result, 1.0f);
} 
