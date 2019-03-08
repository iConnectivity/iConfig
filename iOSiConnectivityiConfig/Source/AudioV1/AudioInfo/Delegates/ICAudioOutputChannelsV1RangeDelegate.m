/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICAudioOutputChannelsV1RangeDelegate.h"
#import "AudioCfgInfo.h"
#import "AudioPortCfgInfo.h"

using namespace GeneSysLib;

@implementation ICAudioOutputChannelsV1RangeDelegate

- (id)initWithDevice:(DeviceInfoPtr)device portID:(Word)portID {
  self = [super init];

  if (self) {
    NSParameterAssert(device);
    NSParameterAssert(portID >= 1);
    self.device = device;
    self.portID = portID;
  }

  return self;
}

- (NSString *)title {
  return @"Output Channels";
}

- (NSInteger)getValue {
  const auto &audioPortCfgInfo =
      self.device->get<AudioPortCfgInfo>(self.portID);

  return (NSInteger)audioPortCfgInfo.numOutputChannels();
}

- (void)setValue:(NSInteger)value {
  const auto &audioCfgInfo = self.device->get<AudioCfgInfo>();

  auto &audioPortCfgInfo = self.device->get<AudioPortCfgInfo>(self.portID);

  const auto &block =
      audioPortCfgInfo.block_at(audioCfgInfo.currentActiveConfig() - 1);

  audioPortCfgInfo.numOutputChannels(value);

  auto audioCfgBlock = audioCfgInfo.activeConfigBlock();

  if ((audioPortCfgInfo.numInputChannels() +
       audioPortCfgInfo.numOutputChannels()) > audioCfgBlock.numAudioChannels) {
    audioPortCfgInfo.numInputChannels(((1 + block.maxInputChannels()) -
                                       audioPortCfgInfo.numOutputChannels()));
  }
  self.device->send<SetAudioPortCfgInfoCommand>(audioPortCfgInfo);
}

- (NSInteger)getMin {
  auto &audioCfgInfo = self.device->get<AudioCfgInfo>();
  auto &audioPortCfgInfo = self.device->get<AudioPortCfgInfo>(self.portID);
  const auto &block =
      audioPortCfgInfo.block_at(audioCfgInfo.currentActiveConfig() - 1);
  return (int)block.minOutputChannels();
}

- (NSInteger)getMax {
  auto &audioCfgInfo = self.device->get<AudioCfgInfo>();
  auto &audioPortCfgInfo = self.device->get<AudioPortCfgInfo>(self.portID);
  const auto &_block =
      audioPortCfgInfo.block_at(audioCfgInfo.currentActiveConfig() - 1);
  return (int)_block.maxOutputChannels();
}

- (NSInteger)getStride {
  return 1;
}

@end
