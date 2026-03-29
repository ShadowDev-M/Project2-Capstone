#version 450

uniform vec3 objectIDColour;  

out vec4 fragColor;

void main() {
    fragColor = vec4(objectIDColour, 1.0);  // Write pure color to selection buffer
}