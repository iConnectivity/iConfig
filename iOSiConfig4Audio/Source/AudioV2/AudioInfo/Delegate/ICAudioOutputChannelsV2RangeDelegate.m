/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICAudioOutputChannelsV2RangeDelegate.h"
#import "AudioGlobalParm.h"
#import "AudioPortParm.h"

using namespace GeneSysLib;

@implementation ICAudioOutputChannelsV2RangeDelegate

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
  AudioPortParm &audioPortParm = self.device->get<AudioPortParm>(self.portID);
  return audioPortParm.numOutputChannels();
}

- (void)setValue:(NSInteger)value {
  AudioGlobalParm &audioGlobalParm = self.device->get<AudioGlobalParm>();
  const auto &currentActiveConfigID = audioGlobalParm.currentActiveConfig();
  auto &audioPortParm = self.device->get<AudioPortParm>(self.portID);
  const auto &configBlock = audioPortParm.block_at(currentActiveConfigID);

  Byte outChannels = (value & 0x7F);
  if (audioPortParm.numOutputChannels() != outChannels) {
    audioPortParm.numOutputChannels(outChannels);

    if ((outChannels + audioPortParm.numInputChannels()) >
        configBlock.maxAudioChannels()) {

      audioPortParm.numInputChannels(configBlock.maxAudioChannels() -
                                     outChannels);
    }

    self.device->send<SetAudioPortParmCommand>(audioPortParm);
  }
}

- (NSInteger)getMin {
  const AudioGlobalParm &audioGlobalParm = self.device->get<AudioGlobalParm>();
  const auto &currentActiveConfigID = audioGlobalParm.currentActiveConfig();
  const AudioPortParm &audioPortParm =
      self.device->get<AudioPortParm>(self.portID);
  const auto &block = audioPortParm.block_at(currentActiveConfigID);

  return (NSInteger)block.minOutputChannels();
}

- (NSInteger)getMax {
  const auto &audioGlobalParm = self.device->get<AudioGlobalParm>();
  const auto &currentActiveConfigID = audioGlobalParm.currentActiveConfig();
  const auto &audioPortParm = self.device->get<AudioPortParm>(self.portID);
  const auto &block = audioPortParm.block_at(currentActiveConfigID);
  return (NSInteger)block.maxOutputChannels();
}

- (NSInteger)getStride {
  return 1;
}

@end
