/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICAudioPatchbayV2Delegate.h"
#import "AudioGlobalParm.h"
#import "AudioPatchbayParm.h"
#import "AudioPortParm.h"

using namespace GeneSysLib;

@interface ICAudioPatchbayV2Delegate ()

@property(nonatomic) DeviceInfoPtr device;

@end

@implementation ICAudioPatchbayV2Delegate

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
  const AudioGlobalParm &audioGlobalParm = self.device->get<AudioGlobalParm>();
  return audioGlobalParm.numAudioPorts();
}

- (int)numDestinationSections {
  const AudioGlobalParm &audioGlobalParm = self.device->get<AudioGlobalParm>();
  return audioGlobalParm.numAudioPorts();
}

- (int)numSourcesPerSection:(int)section {
  Word portID = section;
  const AudioPortParm &audioPortParm = self.device->get<AudioPortParm>(portID);
  return audioPortParm.numOutputChannels();
}

- (int)numDestinationsPerSection:(int)section {
  Word portID = section;
  const AudioPortParm &audioPortParm = self.device->get<AudioPortParm>(portID);
  return audioPortParm.numInputChannels();
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
  const AudioPortParm &audioPortParm = self.device->get<AudioPortParm>(portID);
  return @(audioPortParm.portName().c_str());
}

- (NSString *)nameForDestinationSection:(int)section {
  Word portID = section;
  const AudioPortParm &audioPortParm = self.device->get<AudioPortParm>(portID);
  return @(audioPortParm.portName().c_str());
}

- (bool)isPatchedFrom:(device_channel_t)src to:(device_channel_t)dst {
  bool result = false;

  const AudioPatchbayParm &audioPatchbay =
      self.device->get<AudioPatchbayParm>((Word)dst.port);
  try {
    const auto &block = audioPatchbay.findInputBlock((Byte)dst.channel);
    result = ((block.portIDOfOutput() == src.port) &&
              (block.outputChannelNumber() == src.channel));
  }
  catch (...) {
  }

  return result;
}

- (void)setPatchedFrom:(device_channel_t)src to:(device_channel_t)dst {
  AudioPatchbayParm &audioPatchbay =
      self.device->get<AudioPatchbayParm>((Word)dst.port);
  try {
    auto &block = audioPatchbay.findInputBlock((Byte)dst.channel);
    block.portIDOfOutput(src.port);
    block.outputChannelNumber(src.channel);

    self.device->send<SetAudioPatchbayParmCommand>(audioPatchbay);
  }
  catch (...) {
  }
}

- (void)foreach:(DevicePairOperator)deviceOperator {
  for (int sect = 1; sect <= [self numSourceSections]; ++sect) {
    Word portID = sect;
    AudioPatchbayParm &audioPatchbay =
        self.device->get<AudioPatchbayParm>(portID);
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
