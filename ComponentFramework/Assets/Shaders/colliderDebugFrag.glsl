#version 450

out vec4 fragColor;

uniform vec3 colliderColor;
uniform float uAlpha;

void main() {
    fragColor = vec4(colliderColor, uAlpha);
}
    