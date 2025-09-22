#version 450
#extension GL_ARB_separate_shader_objects : enable
#define MAX_LIGHTS 4


layout (location = 0) in vec3 vertNormal;
layout (location = 1) in vec3 eyeDir;
layout (location = 2) in vec2 textureCoords;
layout (location = 3) in vec3 lightDir;
layout (location = 10) in vec3 fragPos;

uniform vec3 lightPos; // needs array
uniform vec4 diffuse; // needs array
uniform vec4 specular; // needs array
uniform float intensity; // needs array
uniform vec4 ambient;
uniform uint numLights;

layout (location = 0) out vec4 fragColor;

uniform sampler2D myTexture; 

void main() { 
	vec3 reflection; // needs array
	float spec; // needs array
	float diff; // needs array
	vec4 ka = ambient;
	vec4 kt = texture(myTexture,textureCoords);
	vec4 phongResult = vec4(0.0,0.0,0.0,0.0);

	phongResult += ka * kt;

	//for(uint i = 0; i < 1; i++){
		// Lighting Spec + Diff + Reflect
		vec4 ks = specular;	// needs array
		vec4 kd = diffuse; // needs array 
		diff = max(dot(vertNormal, lightDir), 0.0); // needs array
		reflection = normalize(reflect(-lightDir, vertNormal)); // needs array
		spec = max(dot(eyeDir, reflection), 0.0); // needs array
		spec = pow(spec,14.0); // needs array
		float scale;
		// Attenuation (fall off)
		float dist = length(lightPos.xyz - fragPos); // needs array
		float attenuation = (1/dist) * intensity; // needs array

		vec4 light = ((diff * kd) + (spec * ks)) * kt * attenuation; // needs 2 array

		phongResult += light;
	//}
	fragColor = phongResult;
} 
