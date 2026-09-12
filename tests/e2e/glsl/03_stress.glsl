#version 330 core

// deep nesting stress
float deep(float n) {
    float r = 0.0;
    if (n > 0.0) {
        for (int i = 0; i < 3; i++) {
            while (r < float(i)) {
                if (r < 1.0) {
                    for (int j = 0; j < 2; j++) {
                        if (j == i) {
                            r = r + 1.0;
                        } else {
                            r = r + 2.0;
                        }
                    }
                } else {
                    r = r + 1.0;
                }
            }
        }
    }
    return r;
}
