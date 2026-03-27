#version 450
#extension GL_ARB_separate_shader_objects : enable
#define MAX_BONES 100

layout (location = 0) in vec4 vVertex;
layout (location = 3) in ivec4 aBoneIDs;      // CHANGED: location=3
layout (location = 4) in vec4 aBoneWeights;   // CHANGED: location=4

uniform mat4 bone_transforms[MAX_BONES];       // CHANGED: bone_transforms
uniform bool isAnimated;
uniform mat4 modelMatrix;

void main() {
    vec4 nPos = vVertex;
 
    //casts no shadow (though you could probably just not call it, its the same data size as bool either way)
    if (isAnimated) { 
        //animation
        mat4 boneTransform = mat4(0.0);
        boneTransform += bone_transforms[int(aBoneIDs.x)] * aBoneWeights.x;
        boneTransform += bone_transforms[int(aBoneIDs.y)] * aBoneWeights.y;
        boneTransform += bone_transforms[int(aBoneIDs.z)] * aBoneWeights.z;
        boneTransform += bone_transforms[int(aBoneIDs.w)] * aBoneWeights.w;
    
        nPos = boneTransform * vVertex;
    }
    // Same as scene shader — world pos into light clip space

    gl_Position = modelMatrix * nPos;//vec4(vWorldPos, 1.0);
}