/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICAudioPatchbayV1Delegate.h"
#import "AudioInfo.h"
#import "AudioCfgInfo.h"
#import "AudioPortInfo.h"
#import "AudioPortCfgInfo.h"
#import "AudioPortPatchbay.h"

using namespace GeneSysLib;

@interface ICAudioPatchbayV1Delegate ()

@property(nonatomic) DeviceInfoPtr device;

@end

@implementation ICAudioPatchbayV1Delegate

- (id)initWithDevice:(DeviceInfoPtr)device {
  self = [super init];

  if (self) {
    NSParameterAssert(device);
    self.device = device;
  }

  return self;
}

- (void)setCallback:(RefreshCallback)refreshCallback {
}

- (int)numSourceSections {
  const AudioInfo &audioInfo = self.device->get<AudioInfo>();
  return audioInfo.numberOfAudioPorts();
}

- (int)numDestinationSections {
  const AudioInfo &audioInfo = self.device->get<AudioInfo>();
  return audioInfo.numberOfAudioPorts();
}

- (int)numSourcesPerSection:(int)section {
  Word portID = section;
  AudioPortCfgInfo &audioPortCfgInfo =
      self.device->get<AudioPortCfgInfo>(portID);
  return audioPortCfgInfo.numOutputChannels();
}

- (int)numDestinationsPerSection:(int)section {
  Word portID = section;
  AudioPortCfgInfo &audioPortCfgInfo =
      self.device->get<AudioPortCfgInfo>(portID);
  return audioPortCfgInfo.numInputChannels();
}

- (int)totalSources {
  int count = 0;
  for (int i = 1; i <= [self numSourceSections]; ++i) {
    count += [self numSourcesPerSection:i];
  }
  return count;
}

- (int)totalDestinations {
  int count = 0;
  for (int i = 1; i <= [self numDestinationSections]; ++i) {
    count += [self numDestinationsPerSection:i];
  }
  return count;
}

- (int)sourceIndexToTotal:(device_channel_t)targetIndex {
  int index = 0;
  for (int sect = 1; sect <= [self numSourceSections]; ++sect) {
    if (targetIndex.port == sect) {
      index += targetIndex.channel - 1;
      break;
    } else {
      index += [self numSourcesPerSection:sect];
    }
  }
  return index;
}

// This method will return the port and channel given the total 1 based offset
- (device_channel_t)sourceTotalToIndex:(int)totalIndex {
  device_channel_t result = {0, 0};

  if (totalIndex > 0) {
    result = {1, 1};
    --totalIndex;
  }

  for (int sect = 1; sect <= [self numSourceSections]; ++sect) {
    if ((totalIndex - [self numSourcesPerSection:sect]) < 0) {
      result.channel += totalIndex;
      break;
    } else {
      ++result.port;
      totalIndex -= [self numSourcesPerSection:sect];
    }
  }

  return result;
}

- (int)destinationIndexToTotal:(device_channel_t)targetIndex {
  int index = 0;
  for (int sect = 1; sect <= [self numDestinationSections]; ++sect) {
    if (targetIndex.port == sect) {
      index += targetIndex.channel - 1;
      break;
    } else {
      index += [self numDestinationsPerSection:sect];
    }
  }
  return index;
}

- (device_channel_t)destinationTotalToIndex:(int)totalIndex {
  device_channel_t result = {0, 0};

  if (totalIndex > 0) {
    result = {1, 1};
    --totalIndex;
  }

  for (int sect = 1; sect <= [self numDestinationSections]; ++sect) {
    if ((totalIndex - [self numDestinationsPerSection:sect]) < 0) {
      result.channel += totalIndex;
      break;
    } else {
      ++result.port;
      totalIndex -= [self numDestinationsPerSection:sect];
    }
  }

  return result;
}

- (NSString *)nameForSourceSection:(int)section {
  Word portID = section;
  const AudioPortInfo &audioPortInfo = self.device->get<AudioPortInfo>(portID);
  return @(audioPortInfo.portName().c_str());
}

- (NSString *)nameForDestinationSection:(int)section {
  Word portID = section;
  const AudioPortInfo &audioPortInfo = self.device->get<AudioPortInfo>(portID);
  return @(audioPortInfo.portName().c_str());
}

- (bool)isPatchedFrom:(device_channel_t)src to:(device_channel_t)dst {
  bool result = false;
  const AudioPortPatchbay &audioPatchbay =
      self.device->get<AudioPortPatchbay>(dst.port);
  try {
    const auto &block = audioPatchbay.findInputBlock(dst.channel);
    result = ((block.portIDOfOutput() == src.port) &&
              (block.outputChannelNumber() == src.channel));
  }
  catch (...) {
  }

  return result;
}

- (void)setPatchedFrom:(device_channel_t)src to:(device_channel_t)dst {
  AudioPortPatchbay &audioPatchbay =
      self.device->get<AudioPortPatchbay>(dst.port);
  try {
    auto &block = audioPatchbay.findInputBlock((Byte)dst.channel);
    block.portIDOfOutput(src.port);
    block.outputChannelNumber(src.channel);

    self.device->send<SetAudioPortPatchbayCommand>(audioPatchbay);
  }
  catch (...) {
  }
}

- (void)foreach:(DevicePairOperator)deviceOperator {
  for (int sect = 1; sect <= [self numSourceSections]; ++sect) {
    Word portID = sect;
    AudioPortPatchbay &audioPatchbay =
        self.device->get<AudioPortPatchbay>(portID);
    for (const auto &item : audioPatchbay.flatList()) {
      device_pair_t pair;
      pair.source.port = item.outPortID;
      pair.source.channel = item.outChannelNumber;
      pair.destination.port = item.inPortID;
      pair.destination.channel = item.inChannelNumber;
      deviceOperator(&pair);
    }
  }
}

- (BOOL)isSourceDeviceInBounds:(device_channel_t)device {
  return ((device.port >= 1) && (device.port <= [self numSourceSections]) &&
          (device.channel >= 1) &&
          (device.channel <= [self numSourcesPerSection:device.port]));
}

- (BOOL)isDestinationDeviceInBounds:(device_channel_t)device {
  return ((device.port >= 1) &&
          (device.port <= [self numDestinationSections]) &&
          (device.channel >= 1) &&
          (device.channel <= [self numDestinationsPerSection:device.port]));
}

@end
