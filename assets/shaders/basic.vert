#version 450

layout(binding = 0) uniform UniformBufferObject {
    vec2 screenSize;
    vec2 cameraPosition;
}
ubo;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in float inScale;
layout(location = 2) in uint inHue;

layout(location = 0) out vec2 outUV;
layout(location = 1) out float outHue;

vec2 positions[] = vec2[](vec2(-1.0, -1.0),
                          vec2(1.0, -1.0),
                          vec2(1.0, 1.0),
                          vec2(-1.0, -1.0),
                          vec2(1.0, 1.0),
                          vec2(-1.0, 1.0));

vec2 uvs[] = vec2[](vec2(0.0, 0.0),
                    vec2(1.0, 0.0),
                    vec2(1.0, 1.0),
                    vec2(0.0, 0.0),
                    vec2(1.0, 1.0),
                    vec2(0.0, 1.0));

void main() {
    vec2 screen_normalization_factor = ubo.screenSize / 2.0;
    vec2 vertex_offset = (positions[gl_VertexIndex] * inScale) / screen_normalization_factor;
    vec2 sphere_position = (inPosition - ubo.cameraPosition) / screen_normalization_factor;
    vec2 vertex_position = sphere_position + vertex_offset;

    gl_Position = vec4(vertex_position, 0.0, 1.0);
    outUV = uvs[gl_VertexIndex];

    outHue = float(inHue) / 255.0 * 6.2832;
}
