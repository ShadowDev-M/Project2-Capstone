#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 5) in vec3 aTangent;
//layout (location = 6) in vec3 aBitangent;


uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

layout (location = 0) out vec3 vertNormal;
layout (location = 1) out vec3 eyeDir;
layout (location = 2) out vec2 textureCoords;
layout (location = 3) out vec3 worldPos;
layout (location = 4) out vec3 localPos;
layout (location = 5) out vec3 localNormal;
layout (location = 6) out mat3 TBN; //tangent-bitangent-normal matrix for normal mapping

void main() {
    localPos = aPos.xyz;
    localNormal = aNormal.xyz;
    textureCoords = aTexCoords;
    textureCoords.y *= -1.0f;
    
    vec3 T = normalize(vec3(modelMatrix * vec4(aTangent, 0.0)));
    vec3 N = normalize(vec3(modelMatrix * aNormal));
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N,T);

    TBN = mat3(T, B, N);

    mat3 normalMatrix = mat3(transpose(inverse(modelMatrix)));
    vertNormal = normalize(normalMatrix * aNormal.xyz);
    
    vec3 eyePos = (viewMatrix * modelMatrix * aPos).xyz;
    eyeDir = normalize(-eyePos);
    
    worldPos = (modelMatrix * aPos).xyz;  // Pass world pos to fragment
    
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * aPos;
}