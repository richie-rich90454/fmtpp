#version 330 core

uniform vec3 color;

// strings hold braces { like these } safely
void main() {
    vec3 c = color;
    // line comment with } brace
    /* block comment with { } inside */
    gl_FragColor = vec4(c, 1.0);
}
