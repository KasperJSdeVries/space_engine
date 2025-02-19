#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
	mat4 projection;
} ubo;

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec3 inUV;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 outUV;
layout(location = 1) out vec3 outNormal;

void main() {
    gl_Position = ubo.projection * ubo.view * ubo.model * inPosition;
	outUV = inUV;
	outNormal = inNormal;
}
