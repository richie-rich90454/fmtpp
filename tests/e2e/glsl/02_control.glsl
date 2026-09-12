#version 330 core

uniform vec3 color;

// exercise every control form
float classify(float n) {
    if (n < 0.0) {
        return -1.0;
    } else if (n == 0.0) {
        return 0.0;
    } else {
        return 1.0;
    }
}

void main() {
    float total = 0.0;
    for (int i = 0; i < 4; i++) {
        total = total + classify(float(i));
    }
    while (total > 4.0) {
        total = total - 4.0;
    }
    if (total > 0.0) {
        gl_FragColor = vec4(total);
    } else {
        gl_FragColor = vec4(0.0);
    }
}
