#import <Foundation/Foundation.h>

// exercise control forms in plain C
int classify(int n) {
    if (n < 0) {
        return -1;
    } else if (n == 0) {
        return 0;
    } else {
        return 1;
    }
}

int run(int n) {
    int total = 0;
    for (int i = 0; i < n; i++) {
        total = total + classify(i);
    }
    while (total > 4) {
        total = total - 4;
    }
    switch (total) {
        case 0:
            return 0;
        default:
            break;
    }
    return total;
}

int main(int argc, const char *argv[]) {
    return run(4);
}
