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
uniform vec3 lightPos; // array
uniform vec4 diffuse; // array
uniform vec4 specular; // array
uniform float intensity; // array
uniform vec4 ambient;
uniform uint numLights;

// outs
layout (location = 0) out vec3 vertNormal;
layout (location = 1) out vec3 eyeDir;
layout (location = 2) out vec2 textureCoords;
layout (location = 3) out vec3 lightDir; // array
layout (location = 10) out vec3 fragPos;
 

void main() {
	textureCoords = texCoords;
	textureCoords.y *= -1.0f;
	mat3 normalMatrix = mat3(transpose(inverse(modelMatrix)));

	vertNormal = normalize(normalMatrix * vNormal.xyz); /// Rotate the normal to the correct orientation 
	vec3 vertPos = vec3(viewMatrix * modelMatrix * vVertex); /// This is the position of the vertex from the origin
	vec3 vertDir = normalize(vertPos);
	eyeDir = -vertDir;

	fragPos = vec3(modelMatrix * vVertex);

	/// Light position from the point-of-view of each vertex
	vec3 lightLocFromVertex; // array
	//for(int i =0; i < numLights; i++){
		lightLocFromVertex = vec3(lightPos) - vertPos; // array
		lightDir = normalize(lightLocFromVertex); /// Create the light direction. // array
	//}
	
	gl_Position =  projectionMatrix * viewMatrix * modelMatrix * vVertex; 
}
