#import <Foundation/Foundation.h>

// a tiny dog
@interface Dog : NSObject {
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
