#import <Foundation/Foundation.h>

@protocol Pet <NSObject>
- (void)bark;
@end

@interface Dog : NSObject <Pet> {
    NSString *_name;
}

- (instancetype)initWithName:(NSString *)name;
- (void)bark;

@end

@implementation Dog

- (instancetype)initWithName:(NSString *)name {
    self = [super init];
    _name = name;
    return self;
}

- (void)bark {
    NSLog(@"Woof!");
}

@end

int main(int argc, const char *argv[]) {
    return 0;
}

// Another class with a loop and a C helper.
@interface Counter : NSObject {
    int _value;
}

- (void)add:(int)n;
- (int)value;

@end

@implementation Counter

- (void)add:(int)n {
    for (int i = 0; i < n; i++) {
        _value++;
    }
    while (_value > 100) {
        _value = _value - 100;
    }
}

- (int)value {
    // braces { } in comments stay put
    return _value;
}

@end

int total_upto(int n) {
    int total = 0;
    for (int i = 0; i <= n; i++) {
        total = total + i;
    }
    return total;
}
