#version 330 core

layout(location = 0) in vec3 position;
uniform vec3 color;
uniform float time;
uniform vec3 fog_color;

struct Light {
    vec3 position;
    float intensity;
};

float brightness(vec3 c) {
    return (c.r + c.g + c.b) / 3.0;
}

void main() {
    vec3 c = color * brightness(color);
    if (c.r > 1.0) {
        c = vec3(1.0);
    }
    gl_FragColor = vec4(c, 1.0);
}

// fog helper below
float fog_factor(float depth) {
    float f = depth;
    if (f < 0.0) {
        f = 0.0;
    }
    // braces { } in comments stay put
    return f / (f + 1.0);
}

vec3 tint(vec3 c, float amount) {
    vec3 out_c = c;
    for (int i = 0; i < 2; i++) {
        out_c = out_c * amount;
    }
    while (out_c.r > 2.0) {
        out_c.r = out_c.r - 1.0;
    }
    return out_c;
}

// palette "{ warm }" tones below
const vec3 WARM = vec3(1.0, 0.8, 0.6);

uniform float brightness_cutoff;

// clamp helper below
float clamp01(float v) {
    if (v < 0.0) {
        return 0.0;
    }
    if (v > 1.0) {
        return 1.0;
    }
    return v;
}
