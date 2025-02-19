#version 450

layout(location = 0) in vec3 inUV;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec4 outColor;
layout(binding = 1) uniform sampler2D textureSampler;

void main() {
	outColor = texture(textureSampler, inUV.xy);
}
