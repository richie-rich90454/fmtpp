#version 330 core

layout(location = 0) in vec3 position;
uniform vec3 color;

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
