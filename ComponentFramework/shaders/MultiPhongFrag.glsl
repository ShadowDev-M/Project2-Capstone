#version 450
#extension GL_ARB_separate_shader_objects : enable
#define MAX_LIGHTS 4


layout (location = 0) in vec3 vertNormal;
layout (location = 1) in vec3 eyeDir;
layout (location = 2) in vec2 textureCoords;
layout (location = 3) in vec3 fragPos;
layout (location = 4) in vec3 lightDir[MAX_LIGHTS];


uniform vec3 lightPos[MAX_LIGHTS]; 
uniform vec4 diffuse[MAX_LIGHTS]; 
uniform vec4 specular[MAX_LIGHTS]; 
uniform float intensity[MAX_LIGHTS]; 
uniform uint lightType[MAX_LIGHTS]; // 0 is directional, 1 is point
uniform vec4 ambient;
uniform uint numLights;

layout (location = 0) out vec4 fragColor;

uniform sampler2D myTexture; 

void main() {
	vec3 reflection; 
	float spec; 
	float diff; 
	vec4 ka = ambient;
	vec4 kt = texture(myTexture,textureCoords);
	vec4 phongResult = vec4(0.0,0.0,0.0,0.0);

	phongResult += ka * kt;

	for(uint i = 0u; i < numLights; i++){
		float intense = intensity[i];
		// Lighting Spec + Diff + Reflect
		vec4 ks = specular[i];	
		vec4 kd = diffuse[i]; 
		 
		

		// Attenuation (fall off)
		vec4 light;

		if (lightType[i] == 0u) {
			vec3 normal = normalize(vertNormal);
			vec3 dir = normalize(-lightPos[i]);
			vec3 eye  = normalize(eyeDir);
			reflection = reflect(-dir, normal);

			diff = max(dot(normal, dir), 0.0);

			spec = 0.0;
			if (diff > 0.0) {
				spec = pow(max(dot(eye, reflection), 0.0), 16.0);
			}

			light = ((diff * kd) + (spec * ks)) * kt * intense;
		} else {
			diff = max(dot(vertNormal, lightDir[i]), 0.0); 
			reflection = normalize(reflect(-lightDir[i], vertNormal)); 
			spec = max(dot(eyeDir, reflection), 0.0); 
			spec = pow(spec,14.0);

			float dist = length(lightPos[i].xyz - fragPos.xyz);
			float attenuation = (1/dist) * intense; 
			
			light = ((diff * kd) + (spec * ks)) * kt * attenuation;
		}
		phongResult += light;
	}
	fragColor = phongResult;
} 
