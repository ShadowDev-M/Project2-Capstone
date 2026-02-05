#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;

//layout(location = 0) in vec3 vertNormal;
//layout(location = 1) in vec3 lightDir;
//layout(location = 2) in vec3 eyeDir; 
//layout(location = 3) in vec2 textureCoords; 
layout (location = 0) in vec3 vertNormal;
layout (location = 1) in vec3 eyeDir;
layout (location = 2) in vec2 textureCoords;
layout (location = 3) in vec3 worldPos; 

uniform sampler2D myTexture; 

void main() {
    vec4 ks = vec4(1.0, 0.0, 0.0, 0.0);
	vec4 kd = vec4(0.75, 0.6, 0.6, 0.0);
	vec4 ka = 0.1 * kd;
	vec4 kt = texture(myTexture, textureCoords); 

	vec3 lightWorldPos = normalize(eyeDir); 


	float diff = max(dot(vertNormal, lightWorldPos), 0.0);
	vec3 reflection = normalize(reflect(-lightWorldPos, vertNormal));
	float spec = max(dot(eyeDir, reflection), 0.0);
	spec = pow(spec, 14.0);

	// Compute screen-space edge width using normal vector
	float edge = fwidth(vertNormal.x) + fwidth(vertNormal.y) + fwidth(vertNormal.z);
	float normalLength = length(vertNormal);
	float outlineFactor = smoothstep(0.3, 1.2 + edge * 1, normalLength);

	// White outline if normal is near edge
	vec4 outlineColor = vec4(1.0, 1.0, 3.0, 0.0); // white

	vec4 litColor = (ka + (diff * kd) + (spec * ks)) * kt;
	fragColor = mix(outlineColor, litColor, outlineFactor);
}