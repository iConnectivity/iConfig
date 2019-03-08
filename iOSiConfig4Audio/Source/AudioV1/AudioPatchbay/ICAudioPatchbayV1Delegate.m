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

@interface ICAudioPatchbayV1Delegate () {
  std::vector<bool> isCollapsedInputVec;
  std::vector<bool> isCollapsedOutputVec;
}

@property(nonatomic) DeviceInfoPtr device;

@end

@implementation ICAudioPatchbayV1Delegate

- (id)initWithDevice:(DeviceInfoPtr)device {
  self = [super init];

  if (self) {
    NSParameterAssert(device);
    self.device = device;

    const auto &audioInfo = device->get<AudioInfo>();
    // because V1 was depreicated when only USB Device Jacks were supported I will
    // ignore all the other port types
    for (int i = 0; i < audioInfo.numberOfUSBDeviceJacks(); i++) {
      self->isCollapsedInputVec.push_back(NO);
      self->isCollapsedOutputVec.push_back(NO);
    }
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
  if (![self isCollapsedOutput:section])
    return audioPortCfgInfo.numOutputChannels();
  else
    return 1;
}

- (int)numDestinationsPerSection:(int)section {
  Word portID = section;
  AudioPortCfgInfo &audioPortCfgInfo =
      self.device->get<AudioPortCfgInfo>(portID);
  if (![self isCollapsedInput:section])
    return audioPortCfgInfo.numInputChannels();
  else
    return 1;
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

  int outPort, outCh, inPort, inCh;
  outPort = src.port;
  outCh = src.channel;
  inPort = dst.port;
  inCh = dst.channel;

  if (([self isCollapsedInput:outPort]) || ([self isCollapsedOutput:inPort])) {
    return true;
  }

  try {
    const AudioPortPatchbay &audioPatchbay = self.device->get<AudioPortPatchbay>(dst.port);

    const auto &block = audioPatchbay.findInputBlock(dst.channel);
    result = ((block.portIDOfOutput() == src.port) &&
              (block.outputChannelNumber() == src.channel));
  }
  catch (...) {
  }

  return result;
}

- (void)setPatchedFrom:(device_channel_t)src to:(device_channel_t)dst andRemove:(device_channel_t)toRemove {
  int outPort, outCh, inPort, inCh;
  outPort = src.port;
  outCh = src.channel;
  inPort = dst.port;
  inCh = dst.channel;

  if ((outPort == 0 && [self isCollapsedInput:toRemove.port]) ||
      ([self isCollapsedOutput:inPort]) ||
      (outPort != 0 && [self isCollapsedInput:outPort])) {
    return;
  }

  try {
    AudioPortPatchbay &audioPatchbay = self.device->get<AudioPortPatchbay>(dst.port);
    auto &block = audioPatchbay.findInputBlock((Byte)dst.channel);

    block.portIDOfOutput(src.port);
    block.outputChannelNumber(src.channel);

    self.device->send<SetAudioPortPatchbayCommand>(audioPatchbay);
  }
  catch (...) {
  }
}

- (bool)isMixerToo {
  return false;
}

- (void)toggleCollapseInput:(int)section
{
  self->isCollapsedInputVec.at(section - 1) = !self->isCollapsedInputVec.at(section - 1);
}

- (bool)isCollapsedInput:(int)section
{
  if (section == 0 || section == 65535)
    return false;
  return self->isCollapsedInputVec.at(section - 1);
}

- (void)toggleCollapseOutput:(int)section
{
  self->isCollapsedOutputVec.at(section - 1) = !self->isCollapsedOutputVec.at(section - 1);
}

- (bool)isCollapsedOutput:(int)section
{
  if (section == 0 || section == 65535)
    return false;
  return self->isCollapsedOutputVec.at(section - 1);
}

- (void)foreachMixer:(DevicePairOperator)deviceOperator {

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

      if ([self isCollapsedInput:item.outPortID])
        pair.source.channel = 1;
      if ([self isCollapsedOutput:item.inPortID])
        pair.destination.channel = 1;

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
