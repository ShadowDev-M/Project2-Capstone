#version 450

layout(location = 0) in vec3 inPosition;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform vec3 objectIDColour;


void main() {
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(inPosition, 1.0);
}