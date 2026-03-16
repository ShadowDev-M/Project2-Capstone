#version 450

#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;

void main() {
    float depth = gl_FragCoord.z;
    fragColor = vec4(vec3(depth), 1.0);  
}