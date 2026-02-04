#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec4 vVertex;
layout (location = 1) in vec4 vNormal;
layout (location = 2) in vec2 texCoords;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

layout (location = 0) out vec3 vertNormal;
layout (location = 1) out vec3 eyeDir;
layout (location = 2) out vec2 textureCoords;
layout (location = 3) out vec3 worldPos; 

void main() {
    textureCoords = texCoords;
    textureCoords.y *= -1.0f;
    
    mat3 normalMatrix = mat3(transpose(inverse(modelMatrix)));
    vertNormal = normalize(normalMatrix * vNormal.xyz);
    
    vec3 eyePos = (viewMatrix * modelMatrix * vVertex).xyz;
    eyeDir = normalize(-eyePos);
    
    worldPos = (modelMatrix * vVertex).xyz;  // Pass world pos to fragment
    
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vVertex;
}