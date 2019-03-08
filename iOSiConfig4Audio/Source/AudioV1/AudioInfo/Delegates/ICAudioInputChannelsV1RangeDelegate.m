/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICAudioInputChannelsV1RangeDelegate.h"
#import "AudioCfgInfo.h"
#import "AudioPortCfgInfo.h"

using namespace GeneSysLib;

@implementation ICAudioInputChannelsV1RangeDelegate

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
  return @"Input Channels";
}

- (NSInteger)getValue {
  const auto &audioPortCfgInfo =
      self.device->get<AudioPortCfgInfo>(self.portID);

  return (NSInteger)audioPortCfgInfo.numInputChannels();
}

- (void)setValue:(NSInteger)value {
  const auto &audioCfgInfo = self.device->get<AudioCfgInfo>();

  auto &audioPortCfgInfo = self.device->get<AudioPortCfgInfo>(self.portID);

  const auto &_block =
      audioPortCfgInfo.block_at(audioCfgInfo.currentActiveConfig() - 1);

  audioPortCfgInfo.numInputChannels(value);

  const auto audioPortBlock = audioCfgInfo.activeConfigBlock();
  if ((audioPortCfgInfo.numInputChannels() +
       audioPortCfgInfo.numOutputChannels()) >
      audioPortBlock.numAudioChannels) {
    audioPortCfgInfo.numOutputChannels(((_block.maxOutputChannels() + 1) -
                                        audioPortCfgInfo.numInputChannels()));
  }
  self.device->send<SetAudioPortCfgInfoCommand>(audioPortCfgInfo);
}

- (NSInteger)getMin {
  auto &audioCfgInfo = self.device->get<AudioCfgInfo>();
  auto &audioPortCfgInfo = self.device->get<AudioPortCfgInfo>(self.portID);
  const auto &block =
      audioPortCfgInfo.block_at(audioCfgInfo.currentActiveConfig() - 1);
  return (int)block.minInputChannels();
}

- (NSInteger)getMax {
  auto &audioCfgInfo = self.device->get<AudioCfgInfo>();
  auto &audioPortCfgInfo = self.device->get<AudioPortCfgInfo>(self.portID);
  const auto &_block =
      audioPortCfgInfo.block_at(audioCfgInfo.currentActiveConfig() - 1);
  return (int)_block.maxInputChannels();
}

- (NSInteger)getStride {
  return 1;
}

@end
