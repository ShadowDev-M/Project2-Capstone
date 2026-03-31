#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNormal;
layout (location = 2) in vec2 texCoords;
layout (location = 5) in vec4 aTangent;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform mat4 lightSpaceMatrix;

layout (location = 0) out mat3 TBN;
layout (location = 3) out vec2 textureCoords;
layout (location = 4) out vec3 worldPos;
layout (location = 5) out vec3 localPos;
layout (location = 6) out vec3 localNormal;
layout (location = 7) out vec4 vFragPosLightSpace;

void main() {
    localPos = aPos.xyz;
    localNormal = aNormal.xyz;

    textureCoords = texCoords;
    textureCoords.y *= -1.0f;
    
    mat3 normalMatrix = mat3(transpose(inverse(modelMatrix)));
    
    // building the TBN matrix
    //vertNormal = normalize(normalMatrix * vNormal.xyz);
    vec3 N = normalize(normalMatrix * aNormal.xyz);
    vec3 T = normalize(normalMatrix * aTangent.xyz);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N,T);

    TBN = mat3(T, B, N);

    //vec3 eyePos = (viewMatrix * modelMatrix * vVertex).xyz;
    
    worldPos = (modelMatrix * aPos).xyz;  // Pass world pos to fragment
    
    vFragPosLightSpace = lightSpaceMatrix * modelMatrix * aPos;
    
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * aPos;
}