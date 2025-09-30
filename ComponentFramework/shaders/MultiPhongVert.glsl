#version 450
#extension GL_ARB_separate_shader_objects : enable
#define MAX_LIGHTS 4

// ins
layout (location = 0) in  vec4 vVertex;
layout (location = 1) in  vec4 vNormal;
layout (location = 2) in  vec2 texCoords;

// camera
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

// model matrix
uniform mat4 modelMatrix;

// lights
uniform vec3 lightPos[MAX_LIGHTS];
uniform vec4 diffuse[MAX_LIGHTS];
uniform vec4 specular[MAX_LIGHTS];
uniform float intensity[MAX_LIGHTS];
uniform uint lightType[MAX_LIGHTS];
uniform vec4 ambient;
uniform uint numLights;

// outs
layout (location = 0) out vec3 vertNormal;
layout (location = 1) out vec3 eyeDir;
layout (location = 2) out vec2 textureCoords;
layout (location = 3) out vec3 fragPos;
layout (location = 4) out vec3 lightDir[MAX_LIGHTS];

 

void main() {
	textureCoords = texCoords;
	textureCoords.y *= -1.0f;
	mat3 normalMatrix = mat3(transpose(inverse(modelMatrix)));

	vertNormal = normalize(normalMatrix * vNormal.xyz); /// Rotate the normal to the correct orientation 
	vec3 vertPos = vec3(viewMatrix * modelMatrix * vVertex); /// This is the position of the vertex from the origin
	vec3 vertDir = normalize(vertPos);
	eyeDir = -vertDir;

	fragPos = vertPos;

	/// Light position from the point-of-view of each vertex
	vec3 lightLocFromVertex[MAX_LIGHTS];
	for(uint i = 0u; i < numLights; i++){
		lightLocFromVertex[i] = vec3(lightPos[i]) - vertPos;
		lightDir[i] = normalize(lightLocFromVertex[i]); /// Create the light direction.
	}
	
	gl_Position =  projectionMatrix * viewMatrix * modelMatrix * vVertex; 
}
