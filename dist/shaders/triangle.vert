#version 460
#extension GL_GOOGLE_include_directive : require
#include "uniform_buffer_object.glsl"

layout(binding = 0) readonly uniform uniform_buffer_object_struct {
	uniform_buffer_object ubo; 
};

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 fragColor;

void main() {
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
	fragColor = vec3(1.0,0.0,0.0);
}
