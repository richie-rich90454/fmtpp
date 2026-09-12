#import <Foundation/Foundation.h>

// strings hold braces { like these } safely
NSString* openMsg() {
    return @"{ hello }";
}

int main(int argc, const char *argv[]) {
    NSString* s = @"a { b } c";
    // line comment with } brace
    /* block comment with { } inside */
    NSLog(@"%@", s);
    return 0;
}
