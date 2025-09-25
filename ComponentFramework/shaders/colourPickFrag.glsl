#version 330 core
out vec4 FragColor;
uniform vec3 uIDColor;

void main() {
    FragColor = vec4(uIDColor, 1.0);
}