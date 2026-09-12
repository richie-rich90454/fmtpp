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
