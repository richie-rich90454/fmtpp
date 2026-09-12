#version 330 core

layout(location = 0) in vec3 position;
uniform vec3 color;

// brightness of a color
float brightness(vec3 c) {
    return (c.r + c.g + c.b) / 3.0;
}

void main() {
    vec3 c = color * brightness(color);
    gl_FragColor = vec4(c, 1.0);
}
