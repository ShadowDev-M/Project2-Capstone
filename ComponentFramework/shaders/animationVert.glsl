#version 450
#extension GL_ARB_separate_shader_objects : enable

#define MAX_LIGHTS 4
#define MAX_BONES 100

// FIXED LOCATIONS like reference (3+4 instead of 5+6)
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;  
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in ivec4 aBoneIDs;      // CHANGED: location=3
layout (location = 4) in vec4 aBoneWeights;   // CHANGED: location=4

// Reference-style uniforms
uniform mat4 bone_transforms[MAX_BONES];       // CHANGED: bone_transforms
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

// Your lights (UNCHANGED)
uniform vec3 lightPos[MAX_LIGHTS];
uniform vec4 diffuse[MAX_LIGHTS];
uniform vec4 specular[MAX_LIGHTS];
uniform float intensity[MAX_LIGHTS];
uniform uint lightType[MAX_LIGHTS];
uniform vec4 ambient;
uniform uint numLights;

layout (location = 0) out vec3 vertNormal;
layout (location = 1) out vec3 eyeDir;
layout (location = 2) out vec2 textureCoords;
layout (location = 3) out vec3 vertPos;
//layout (location = 4) out vec3 lightDir[MAX_LIGHTS];
layout (location = 4) out vec3 localPos;
layout (location = 5) out vec3 localNormal;
 
void main() {
    localPos = aPosition;
    localNormal = aNormal;

    textureCoords = aTexCoords;
    textureCoords.y *= -1.0f;
    
    // REFERENCE EXACT SKINNING LOGIC
    mat4 boneTransform = mat4(0.0);
    boneTransform += bone_transforms[int(aBoneIDs.x)] * aBoneWeights.x;
    boneTransform += bone_transforms[int(aBoneIDs.y)] * aBoneWeights.y;
    boneTransform += bone_transforms[int(aBoneIDs.z)] * aBoneWeights.z;
    boneTransform += bone_transforms[int(aBoneIDs.w)] * aBoneWeights.w;
    
    vec4 skinnedPos = boneTransform * vec4(aPosition, 1.0);
    vec3 skinnedNormal = normalize(mat3(boneTransform) * aNormal);
    
    vec4 worldPos = modelMatrix * skinnedPos;
    vec3 vertPosView = vec3(viewMatrix * worldPos);
    
    // Your lighting outputs (UNCHANGED)
    vertNormal = normalize(mat3(transpose(inverse(modelMatrix))) * skinnedNormal);
    vertPos = vec3(worldPos);
    eyeDir = normalize(-vertPosView);
    
//    if (numLights > 0) {
//        for(uint i = 0u; i < numLights; i++){
//            if (lightType[i] == 0u) {
//                lightDir[i] = normalize((viewMatrix * vec4(-lightPos[i], 0.0f)).xyz);
//            } else {
//                vec3 lightViewPos = (viewMatrix * vec4(lightPos[i], 1.0f)).xyz;
//                lightDir[i] = normalize(lightViewPos - vertPosView);
//            }
//        }
//    }


    
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * skinnedPos;
}