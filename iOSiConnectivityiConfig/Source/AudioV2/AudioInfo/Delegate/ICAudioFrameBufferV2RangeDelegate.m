/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICAudioFrameBufferV2RangeDelegate.h"
#import "AudioGlobalParm.h"

using namespace GeneSysLib;

@interface ICAudioFrameBufferV2RangeDelegate ()

@end

@implementation ICAudioFrameBufferV2RangeDelegate

- (id)initWithDevice:(DeviceInfoPtr)device {
  self = [super init];

  if (self) {
    NSParameterAssert(device);
    self.device = device;
  }

  return self;
}

- (NSString *)title {
  return @"Audio Frames to Buffer";
}

- (NSInteger)getValue {
  const auto &audioGlobalParm = self.device->get<AudioGlobalParm>();
  return audioGlobalParm.currentAudioFrames();
}

- (void)setValue:(NSInteger)value {
  auto &audioGlobalParm = self.device->get<AudioGlobalParm>();
  audioGlobalParm.currentAudioFrames((Byte)value & 0x7F);

  self.device->send<SetAudioGlobalParmCommand>(audioGlobalParm);
}

- (NSInteger)getMin {
  const auto &audioGlobalParm = self.device->get<AudioGlobalParm>();
  return (NSInteger)audioGlobalParm.minAudioFrames();
}

- (NSInteger)getMax {
  const auto &audioGlobalParm = self.device->get<AudioGlobalParm>();
  return (NSInteger)audioGlobalParm.maxAudioFrames();
}

- (NSInteger)getStride {
  return 1;
}

@end
