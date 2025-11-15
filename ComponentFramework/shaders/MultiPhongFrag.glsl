#version 450
#extension GL_ARB_separate_shader_objects : enable
#define MAX_LIGHTS 4


layout (location = 0) in vec3 vertNormal;
layout (location = 1) in vec3 eyeDir;
layout (location = 2) in vec2 textureCoords;
layout (location = 3) in vec3 vertPos;
layout (location = 4) in vec3 lightDir[MAX_LIGHTS];


uniform vec3 lightPos[MAX_LIGHTS]; // the position of the light
uniform vec4 diffuse[MAX_LIGHTS]; // the colour of the light
uniform vec4 specular[MAX_LIGHTS]; // the shininess of the light
uniform float intensity[MAX_LIGHTS]; // the brightness of the light
uniform uint lightType[MAX_LIGHTS]; // 0 is directional, 1 is point
uniform vec4 ambient; // the background "ambient" light / colour
uniform uint numLights; // number of lights in the scene

uniform int hasSpec; // this is technically a boolean, 1 for yes, 0 for no

layout (location = 0) out vec4 fragColor;

uniform sampler2D diffuseTexture; // a given texture's base colour
uniform sampler2D specularTexture; // a given texture's shininess

void main() {
	vec3 reflection; 
	float spec; 
	float diff; 
	vec4 ka = ambient;
	vec4 kt = texture(diffuseTexture,textureCoords);
	vec4 phongResult = vec4(0.0f,0.0f,0.0f,0.0f);

	phongResult += ka * kt;
	if (numLights > 0) {
		for(uint i = 0u; i < numLights; i++){
			float intense = intensity[i];
			// Lighting Spec + Diff + Reflect
			vec4 ks;
			if (hasSpec == 1){
				ks = texture(specularTexture,textureCoords) * specular[i];	
			}
			else {
				ks = specular[i];
			}
			vec4 kd = diffuse[i];

			// Attenuation (fall off)
			vec4 light;

			if (lightType[i] == 0u) {
				vec3 normal = normalize(vertNormal);
				vec3 dir = normalize(-lightPos[i]);
				vec3 eye  = normalize(eyeDir);
				reflection = reflect(-dir, normal);

				diff = max(dot(normal, dir), 0.0f);

				spec = 0.0f;
				if (diff > 0.0f) {
					spec = pow(max(dot(eye, reflection), 0.0f), 16.0f);
				}

				light = ((diff * kd) + (spec * ks)) * kt * intense;
			} else {
				diff = max(dot(vertNormal, lightDir[i]), 0.0f); 
				reflection = normalize(reflect(-lightDir[i], vertNormal)); 
				spec = max(dot(eyeDir, reflection), 0.0f); 
				spec = pow(spec,14.0f);

				float dist = length(lightPos[i].xyz - vertPos.xyz);
				float attenuation = (1.0f/dist) * intense; 
			
				light = (diff * kd * kt + spec * ks) * attenuation; //((diff * kd) + (spec * ks)) * kt * attenuation;
			}
			phongResult += light;
		}
	}
	fragColor = phongResult;
} 
