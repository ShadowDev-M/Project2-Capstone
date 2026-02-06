#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 vVertex;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 uvCoord;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform vec3 lightPos;

layout(location = 0) out vec3 vertNormal;
layout(location = 1) out vec3 lightDir;
layout(location = 2) out vec3 eyeDir;
layout(location = 3) out vec2 textureCoords; 
layout(location = 4) out vec3 worldPos;
layout(location = 5) out vec3 localPos;
layout(location = 6) out vec3 localNormal;

void main() {
    textureCoords = uvCoord;
    textureCoords.y *= -1.0f;
    mat3 normalMatrix = mat3(transpose(inverse(modelMatrix)));
    vertNormal = normalize(normalMatrix * vNormal); /// Rotate the normal to the correct orientation 
    vec3 vertPos = vec3(viewMatrix * modelMatrix * vVertex);
    vec3 vertDir = normalize(vertPos);
    eyeDir = -vertDir;
    lightDir = normalize(vec3(lightPos) - vertPos); 
    worldPos = (modelMatrix * vVertex).xyz;  // Pass world pos to fragment
    localPos = vVertex.xyz;
    localNormal = vNormal.xyz;
    gl_Position = (projectionMatrix * viewMatrix) * modelMatrix * vVertex;
    
}
