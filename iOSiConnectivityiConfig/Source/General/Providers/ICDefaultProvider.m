/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICDefaultProvider.h"

#define MAIN_MENU_INPUT_FILTER_INDEX 0
#define MAIN_MENU_OUTPUT_FILTER_INDEX 1
#define MAIN_MENU_ROUTING_INDEX 2
#define MAIN_MENU_MULTIPORT_INDEX 3
#define MAIN_MENU_ABOUT_INDEX 4

#define UIColorFromRGB(rgbValue)                                       \
  [UIColor colorWithRed:((float)((rgbValue & 0xFF0000) >> 16)) / 255.0 \
                  green:((float)((rgbValue & 0xFF00) >> 8)) / 255.0    \
                   blue:((float)(rgbValue & 0xFF)) / 255.0             \
                  alpha:1.0]

using namespace GeneSysLib;

@interface ICDefaultProvider () {
  NSString *title;
  NSArray *buttonNames;
  NSDictionary *_providerDictionary;
  bool providerInitialized;
}

- (NSDictionary *)providerDictionary;

@end

@implementation ICDefaultProvider

+ (NSDictionary *)dictionaryForMenuName:(NSString *)menuName {
  NSParameterAssert(menuName);

  NSString *const path =
      [[NSBundle mainBundle] pathForResource:@"Provider" ofType:@"plist"];
  NSDictionary *const providerDictionary =
      [NSDictionary dictionaryWithContentsOfFile:path];
  NSDictionary *result = nil;

  if (providerDictionary != nil) {
    result = providerDictionary[menuName];
  }

  return result;
}

- (id)init {
  self = [super init];
  if (self) {
    providerInitialized = false;
    _providerDictionary = nil;

    timer = nil;
  }
  return self;
}

- (void)initializeProviderButtons:(ICViewController *)sender {
  self.parent = sender;
}

- (void)providerWillDissappear:(ICViewController *)sender {
  if ([self stopTimer]) {
    [self onTimerHandler];
  }
}

- (NSDictionary *)providerDictionary {
  if (!providerInitialized) {
    _providerDictionary =
        [ICDefaultProvider dictionaryForMenuName:[self providerName]];
    providerInitialized = true;
  }

  return _providerDictionary;
}

- (NSString *)providerName {
  return @"unknown";
}

- (NSString *)title {
  auto *const dictionary = [self providerDictionary];
  auto *result = @"";

  if ((dictionary) && (dictionary[@"title"])) {
    result = (NSString *)dictionary[@"title"];
  }

  return result;
}

- (NSArray *)buttonNames {
  auto *const dictionary = [self providerDictionary];
  auto *result = @[ @"" ];

  if ((dictionary != nil) && (dictionary[@"buttonNames"])) {
    result = (NSArray *)dictionary[@"buttonNames"];
  }

  return result;
}

- (void)onButtonPressed:(ICViewController *)sender
                  index:(NSInteger)buttonIndex {
}

- (UIColor *)titleBackgroundColor {
  auto *result = [UIColor whiteColor];
  auto *const dictionary = [self providerDictionary];

  if ((dictionary != nil) && (dictionary[@"titleBackgroundColor"])) {
    NSScanner *const scanner = [NSScanner
        scannerWithString:(NSString *)dictionary[@"titleBackgroundColor"]];
    unsigned hex;
    const auto &success = [scanner scanHexInt:&hex];
    if (success) {
      result = UIColorFromRGB(hex);
    }
  }

  return result;
}

- (UIColor *)titleBorderColor {
  auto *result = [UIColor whiteColor];
  auto *const dictionary = [self providerDictionary];

  if ((dictionary != nil) && (dictionary[@"titleBorderColor"])) {
    NSScanner *const scanner = [NSScanner
        scannerWithString:(NSString *)dictionary[@"titleBorderColor"]];
    unsigned hex;
    const auto &success = [scanner scanHexInt:&hex];
    if (success) {
      result = UIColorFromRGB(hex);
    }
  }

  return result;
}

- (UIColor *)colorForIndex:(NSUInteger)index {
  auto *result = [UIColor whiteColor];
  auto *const dictionary = [self providerDictionary];
  NSString *stringToScan = nil;

  if (dictionary) {
    if (dictionary[@"buttonColors"]) {
      auto *const colorArray = (NSArray *)dictionary[@"buttonColors"];
      if ((colorArray != nil) && (colorArray[(index % [colorArray count])])) {
        stringToScan = (NSString *)colorArray[(index % [colorArray count])];
      }
    } else if ((![self isIndexStandard:index]) &&
               (dictionary[@"alternativeColor"])) {
      stringToScan = (NSString *)dictionary[@"alternativeColor"];
    } else if (dictionary[@"standardColor"] != nil) {
      stringToScan = (NSString *)dictionary[@"standardColor"];
    }
  }

  if (stringToScan) {
    NSScanner *const scanner = [NSScanner scannerWithString:stringToScan];
    unsigned hex;
    const auto &success = [scanner scanHexInt:&hex];
    if (success) {
      result = UIColorFromRGB(hex);
    }
  }

  return result;
}

- (UIColor *)selectedColorForIndex:(NSUInteger)index {
  auto *result = [UIColor whiteColor];
  auto *const dictionary = [self providerDictionary];
  NSString *stringToScan = nil;

  if (dictionary) {
    if (dictionary[@"selectedButtonColors"]) {
      auto *const colorArray = (NSArray *)dictionary[@"selectedButtonColors"];
      if ((colorArray != nil) && (colorArray[(index % [colorArray count])])) {
        stringToScan = (NSString *)colorArray[(index % [colorArray count])];
      }
    } else if ((![self isIndexStandard:index]) &&
               (dictionary[@"alternativeSelectionColor"])) {
      stringToScan = (NSString *)dictionary[@"alternativeSelectionColor"];
    } else if (dictionary[@"standardSelectionColor"]) {
      stringToScan = (NSString *)dictionary[@"standardSelectionColor"];
    }
  }

  if (stringToScan) {
    NSScanner *const scanner = [NSScanner scannerWithString:stringToScan];
    unsigned hex;
    auto success = [scanner scanHexInt:&hex];
    if (success) {
      result = UIColorFromRGB(hex);
    }
  }

  return result;
}

- (UIColor *)borderColorForIndex:(NSUInteger)index {
  auto *result = [UIColor whiteColor];
  auto *const dictionary = [self providerDictionary];
  NSString *stringToScan = nil;

  if (dictionary) {
    if (dictionary[@"borderButtonColors"]) {
      auto *const colorArray = (NSArray *)dictionary[@"borderButtonColors"];
      if ((colorArray != nil) && (colorArray[(index % [colorArray count])])) {
        stringToScan = (NSString *)colorArray[(index % [colorArray count])];
      }
    } else if ((![self isIndexStandard:index]) &&
               (dictionary[@"alternativeBorderColor"])) {
      stringToScan = (NSString *)dictionary[@"alternativeBorderColor"];
    } else if (dictionary[@"standardBorderColor"]) {
      stringToScan = (NSString *)dictionary[@"standardBorderColor"];
    }
  }

  if (stringToScan) {
    NSScanner *const scanner = [NSScanner scannerWithString:stringToScan];
    unsigned hex;
    auto success = [scanner scanHexInt:&hex];
    if (success) {
      result = UIColorFromRGB(hex);
    }
  }

  return result;
}

- (NSUInteger)spanForIndex:(NSUInteger)index {
  return 1;
}

- (void)startUpdateTimer {
  if (timer != nil) {
    [self stopTimer];
  }

  timer = [NSTimer timerWithTimeInterval:1.0
                                  target:self
                                selector:@selector(onTimerHandler)
                                userInfo:nil
                                 repeats:NO];
  [[NSRunLoop currentRunLoop] addTimer:timer forMode:NSDefaultRunLoopMode];
}

- (BOOL)stopTimer {
  BOOL stopped = NO;

  if (timer != nil) {
    [timer invalidate];
    timer = nil;
    stopped = YES;
  }

  return stopped;
}

- (void)onTimerHandler {
  timer = nil;
  assert(self.parent);
  assert(self.parent.device);

  DeviceID deviceID = self.parent.device->getDeviceID();
  Word transID = self.parent.device->getTransID();

  [self onUpdate:deviceID transID:transID];
}

- (void)onUpdate:(DeviceID)deviceID transID:(Word)transID {
  // Default handler
}

- (BOOL)isIndexStandard:(NSUInteger)index {
  return YES;
}

@end
