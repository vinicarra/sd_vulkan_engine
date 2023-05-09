#version 450

layout(binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
	mat4 model;
} ubo;

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}