/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICAudioClockV2ChoiceDelegate.h"
#import "AudioClockParm.h"

using namespace GeneSysLib;

@interface ICAudioClockV2ChoiceDelegate ()
@property(nonatomic) DeviceInfoPtr device;
@end

@implementation ICAudioClockV2ChoiceDelegate

- (id)initWithDevice:(DeviceInfoPtr)device {
  self = [super init];

  if (self) {
    self.device = device;
  }

  return self;
}

- (NSString *)title {
  return @"Clock Select";
}

- (NSArray *)options {
  const auto &clockParm = self.device->get<AudioClockParm>();
  NSMutableArray *options = [NSMutableArray array];

  clockParm.for_each([&](const AudioClockParm::SourceBlock &block) {
    [options addObject:@(block.toString().c_str())];
  });

  return options;
}

- (NSInteger)optionCount {
  return [[self options] count];
}

- (NSInteger)getChoice {
  const auto &clockParm = self.device->get<AudioClockParm>();

  return clockParm.activeSourceBlock() - 1;
}

- (void)setChoice:(NSInteger)value {
  auto &clockParm = self.device->get<AudioClockParm>();

  clockParm.activeSourceBlock(value + 1);
  self.device->send<SetAudioClockParmCommand>(clockParm);
}

@end
