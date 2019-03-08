/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICAudioClockV1ChoiceDelegate.h"
#import "AudioClockInfo.h"

@interface ICAudioClockV1ChoiceDelegate ()
@property(nonatomic) DeviceInfoPtr device;
@end

@implementation ICAudioClockV1ChoiceDelegate

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
  using namespace GeneSysLib;
  AudioClockInfo &audioClockInfo = self.device->get<AudioClockInfo>();
  auto *const options = [NSMutableArray array];

  audioClockInfo.for_each([&](const AudioClockInfo::SourceBlock &block) {
    NSString *blockString =
        [NSString stringWithFormat:@"%s", block.toString().c_str()];
    [options addObject:blockString];
  });

  return options;
}

- (NSInteger)optionCount {
  return [[self options] count];
}

- (NSInteger)getChoice {
  using namespace GeneSysLib;
  const auto &audioClockInfo = self.device->get<AudioClockInfo>();

  return audioClockInfo.activeSourceBlock() - 1;
}

- (void)setChoice:(NSInteger)value {
  using namespace GeneSysLib;
  auto &audioClockInfo = self.device->get<AudioClockInfo>();

  audioClockInfo.activeSourceBlock(value);
  self.device->send<SetAudioClockInfoCommand>(audioClockInfo);
}

@end
