/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICAudioSyncFactorV1RangeDelegate.h"
#import "AudioCfgInfo.h"

using namespace GeneSysLib;

@interface ICAudioSyncFactorV1RangeDelegate ()

@end

@implementation ICAudioSyncFactorV1RangeDelegate

+ (ICAudioSyncFactorV1RangeDelegate *)syncFactorWithDevice:
                                          (DeviceInfoPtr)device {
  return [[ICAudioSyncFactorV1RangeDelegate alloc] initWithDevice:device];
}

- (id)initWithDevice:(DeviceInfoPtr)device {
  self = [super init];

  if (self) {
    NSParameterAssert(device);
    self.device = device;
  }

  return self;
}

- (NSString *)title {
  return @"Sync Factor Value";
}

- (NSInteger)getValue {
  const auto &audioCfgInfo = self.device->get<AudioCfgInfo>();
  return (NSInteger)audioCfgInfo.currentSyncFactor();
}

- (void)setValue:(NSInteger)value {
  auto &audioCfgInfo = self.device->get<AudioCfgInfo>();
  audioCfgInfo.currentSyncFactor((Byte)value);
  self.device->send<SetAudioCfgInfoCommand>(audioCfgInfo);
}

- (NSInteger)getMin {
  const auto &audioCfgInfo = self.device->get<AudioCfgInfo>();
  return audioCfgInfo.minAllowedSyncFactor();
}

- (NSInteger)getMax {
  const auto &audioCfgInfo = self.device->get<AudioCfgInfo>();
  return audioCfgInfo.maxAllowedSyncFactor();
}

- (NSInteger)getStride {
  return 1;
}

@end
