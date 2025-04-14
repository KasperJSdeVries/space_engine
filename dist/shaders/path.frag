#version 450

layout(location = 0) in vec2 inUV;
layout(location = 1) in float inCurveState;

layout(location = 0) out vec4 outColor;

float quadratic_ps(vec2 p) {
    vec2 px = dFdx(p);
    vec2 py = dFdy(p);

    float fx = (2 * p.x) * px.x - px.y;
    float fy = (2 * p.x) * py.x - py.y;

    float sd = (p.x * p.x - p.y) / sqrt(fx * fx + fy * fy);

    return 0.5 - sd;
}

void main() {
    float alpha = quadratic_ps(inUV);
    alpha *= inCurveState;
    alpha = clamp(alpha, 0.0, 1.0);
    alpha = 1.0 - alpha;
	if (alpha == 0.0) {
		discard;
	}
    outColor = vec4(1.0, 1.0, 1.0, alpha);
}
