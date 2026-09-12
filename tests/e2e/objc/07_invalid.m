#import <Foundation/Foundation.h>

@interface Broken( {
    int _value;
}
@end

int main(int argc, const char *argv[]) {
    int x = = 5;
    if x > 0
        NSLog(@"hi");
    else
        orphan();
    return 0
}
