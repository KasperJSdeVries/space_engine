#version 450

layout(location = 0) in vec2 inUV;   // [0.0, 1.0]
layout(location = 1) in float inHue; // [0.0, 6.2832]

layout(location = 0) out vec4 outColor;

float circle(vec2 position, float radius) {
    return step(radius, 1.0 - length(position - vec2(0.5)));
}

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
    vec3 hsv = vec3(inHue, 1.0, 1.0);
    outColor = vec4(hsv2rgb(hsv), circle(inUV, 0.5));
}
