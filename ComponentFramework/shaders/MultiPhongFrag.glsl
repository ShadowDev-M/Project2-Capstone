#version 450
#extension GL_ARB_separate_shader_objects : enable
#define MAX_LIGHTS 4


layout (location = 0) in vec3 vertNormal;
layout (location = 1) in vec3 eyeDir;
layout (location = 2) in vec2 fragTexCoords;
layout (location = 3) in vec3 lightDir[MAX_LIGHTS];
layout (location = 4) in vec3 fragPos;

uniform vec4 lightPos[MAX_LIGHTS];
uniform vec4 diffuse[MAX_LIGHTS];
uniform vec4 specular[MAX_LIGHTS];
uniform float intensity[MAX_LIGHTS];
uniform vec4 ambient;
uniform uint numLights;

layout (location = 0) out vec4 fragColor;

layout(binding = 2) uniform sampler2D texSampler;

void main() { 
	vec3 reflection[MAX_LIGHTS];
	float spec[MAX_LIGHTS];
	float diff[MAX_LIGHTS];
	vec4 ka = ambient;
	vec4 kt = texture(texSampler,fragTexCoords); 
	vec4 phongResult = vec4(0.0,0.0,0.0,0.0);

	phongResult += ka * kt;

	for(uint i =0; i < numLights; i++){
		// Lighting Spec + Diff + Reflect
		vec4 ks = specular[i];
		vec4 kd = diffuse[i]; 
		diff[i] = max(dot(vertNormal, lightDir[i]), 0.0);
		reflection[i] = normalize(reflect(-lightDir[i], vertNormal));
		spec[i] = max(dot(eyeDir, reflection[i]), 0.0);
		spec[i] = pow(spec[i],14.0);
		float scale;
		// Attenuation (fall off)
		float dist = length(lightPos[i].xyz - fragPos);
		float attenuation = (1/dist) * intensity[i];

		vec4 light = ((diff[i] * kd) + (spec[i] * ks)) * kt * attenuation;

		phongResult += light;
	}
	fragColor = phongResult;
} 
