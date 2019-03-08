/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICButtonBlockDelegate.h"

@implementation ICButtonBlockDelegate

- initWithTitle:(NSString *)title value:(NSString *)value func:(ButtonCellFunc)func {
  self = [super init];

  if (self) {
    NSParameterAssert(title);
    NSParameterAssert(value);
    self.title = title;
    self.value = value;
    self.func = func;
  }

  return self;
}

- (NSString *)value {
  if (_value != nil)
    return _value;
  return @"";
}

- (void) doFunc {
  if (self.func != nil)
    self.func();
}

@end
