/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICNormalBlockDelegate.h"

@implementation ICNormalBlockDelegate

- initWithTitle:(NSString *)title value:(NSString *)value {
  self = [super init];

  if (self) {
    NSParameterAssert(title);
    NSParameterAssert(value);
    self.title = title;
    self.value = value;
    self.getValue = nil;
  }

  return self;
}

- initWithTitle:(NSString *)title getValue:(NormalCellGetBlock)getValue {
  self = [super init];

  if (self) {
    NSParameterAssert(title);
    NSParameterAssert(getValue);
    self.title = title;
    self.value = nil;
    self.getValue = getValue;
  }

  return self;
}

- (NSString *)value {
  if (self.getValue != nil) {
    return self.getValue();
  } else if (_value != nil) {
    return _value;
  }
  return @"";
}

@end
