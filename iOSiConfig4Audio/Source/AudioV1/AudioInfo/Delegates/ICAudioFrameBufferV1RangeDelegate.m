/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICAudioFrameBufferV1RangeDelegate.h"
#import "AudioCfgInfo.h"

using namespace GeneSysLib;

@interface ICAudioFrameBufferV1RangeDelegate ()

@end

@implementation ICAudioFrameBufferV1RangeDelegate

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
  const auto &audioCfgInfo = self.device->get<AudioCfgInfo>();
  return audioCfgInfo.currentNumAudioFrames();
}

- (void)setValue:(NSInteger)value {
  auto &audioCfgInfo = self.device->get<AudioCfgInfo>();
  audioCfgInfo.currentNumAudioFrames((Byte)value);
  self.device->send<SetAudioCfgInfoCommand>(audioCfgInfo);
}

- (NSInteger)getMin {
  const auto &audioCfgInfo = self.device->get<AudioCfgInfo>();
  return (NSInteger)audioCfgInfo.minNumAudioFrames();
}

- (NSInteger)getMax {
  const auto &audioCfgInfo = self.device->get<AudioCfgInfo>();
  return (NSInteger)audioCfgInfo.maxNumAudioFrames();
}

- (NSInteger)getStride {
  return 1;
}

@end
