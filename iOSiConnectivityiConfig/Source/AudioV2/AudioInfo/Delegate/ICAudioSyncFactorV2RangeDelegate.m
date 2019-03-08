/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICAudioSyncFactorV2RangeDelegate.h"
#import "AudioGlobalParm.h"

using namespace GeneSysLib;

@interface ICAudioSyncFactorV2RangeDelegate ()

@end

@implementation ICAudioSyncFactorV2RangeDelegate

+ (ICAudioSyncFactorV2RangeDelegate *)syncFactorWithDevice:
                                          (DeviceInfoPtr)device {
  return [[ICAudioSyncFactorV2RangeDelegate alloc] initWithDevice:device];
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
  const auto &audioGlobalParm = self.device->get<AudioGlobalParm>();
  return (NSInteger)audioGlobalParm.currentSyncFactor();
}

- (void)setValue:(NSInteger)value {
  auto &audioGlobalParm = self.device->get<AudioGlobalParm>();
  audioGlobalParm.currentSyncFactor((Byte)(value & 0x7F));
  self.device->send<SetAudioGlobalParmCommand>(audioGlobalParm);
}

- (NSInteger)getMin {
  const auto &audioGlobalParm = self.device->get<AudioGlobalParm>();
  return audioGlobalParm.minSyncFactor();
}

- (NSInteger)getMax {
  const auto &audioGlobalParm = self.device->get<AudioGlobalParm>();
  return audioGlobalParm.maxSyncFactor();
}

- (NSInteger)getStride {
  return 1;
}

@end
