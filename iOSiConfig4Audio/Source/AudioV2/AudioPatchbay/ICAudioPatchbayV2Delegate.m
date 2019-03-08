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

#include "CommandDefines.h"
#include "CommandList.h"

#import "MixerInterface.h"
#import "MixerPortParm.h"

using namespace GeneSysLib;

@interface ICAudioPatchbayV2Delegate () {
  std::vector<bool> isCollapsedInputVec;
  std::vector<bool> isCollapsedOutputVec;
  bool isMixerTooBool;
}

@property(nonatomic) DeviceInfoPtr device;
@property(nonatomic) MixerInterface* mixerInterface;

@end

@implementation ICAudioPatchbayV2Delegate

- (id)initWithDevice:(DeviceInfoPtr)device {
  self = [super init];

  //NSLog(@"patchbay: 1");
  if (self) {
    NSParameterAssert(device);
    self.device = device;

    if (device->get<CommandList>().contains(Command::GetMixerParm)) {
      self->isMixerTooBool = true;
      self.mixerInterface = new MixerInterface(device);
    }

    //NSLog(@"in self");

    const auto &audioInfo = device->get<AudioGlobalParm>();
    //NSLog(@"audioInfo.numAudioPorts(): %i", audioInfo.numAudioPorts());
    for (int i = 0; i < audioInfo.numAudioPorts(); i++) {
      self->isCollapsedInputVec.push_back(false);
      self->isCollapsedOutputVec.push_back(false);
      //NSLog(@"init size is: %lu", self->isCollapsedOutputVec.size());
      if (self->isMixerTooBool) {
        self->isCollapsedInputVec.push_back(false);
        self->isCollapsedOutputVec.push_back(false);
      }
    }

  }

  return self;
}

- (void)setCallback:(RefreshCallback)refreshCallback {
}

- (int)numSourceSections {
  //NSLog(@"patchbay: 2");

  const AudioGlobalParm &audioGlobalParm = self.device->get<AudioGlobalParm>();
  int toReturn = audioGlobalParm.numAudioPorts();
  if (self->isMixerTooBool)
    toReturn *= 2;
  //NSLog(@"patchbay: 2 returning %i", toReturn);
  return toReturn;
}

- (int)numDestinationSections {
  //NSLog(@"patchbay: 3");

  const AudioGlobalParm &audioGlobalParm = self.device->get<AudioGlobalParm>();
  int toReturn = audioGlobalParm.numAudioPorts();
  if (self->isMixerTooBool)
    toReturn *= 2;
  return toReturn;
}

- (int)numDestinationsPerSection:(int)section {
  //NSLog(@"patchbay: 4");

  int toReturn;
  Word portID = section;
  if (!self->isMixerTooBool) {
    const AudioPortParm &audioPortParm = self.device->get<AudioPortParm>(portID);
    return audioPortParm.numInputChannels();
  }
  else {
    int realSection = floor((section+1)/2);
    if (section % 2) { // odd = true port, not mixer
      const auto &portParm = self.device->get<AudioPortParm>(realSection);
      toReturn = portParm.numInputChannels();
    }
    else { // even == mixer port
      toReturn = self.mixerInterface->numberInputs(realSection);
      if (!self.mixerInterface->numberOutputs(realSection)) {
        toReturn = 0;
      }
    }
  }
  if (toReturn >= 1) {
    if ([self isCollapsedOutput:section])
      toReturn = 1;
  }
  return toReturn;
}

- (int)numSourcesPerSection:(int)section {
  //NSLog(@"patchbay: 5");

  int toReturn;
  Word portID = section;
  if (!self->isMixerTooBool) {
    const AudioPortParm &audioPortParm = self.device->get<AudioPortParm>(portID);
    return audioPortParm.numOutputChannels();
  }
  else {
    int realSection = floor((section+1)/2);
    if (section % 2) { // odd = true port, not mixer
      const auto &portParm = self.device->get<AudioPortParm>(realSection);
      toReturn = portParm.numOutputChannels();
    }
    else { // even == mixer port
      toReturn = self.mixerInterface->numberOutputs(realSection);
    }
  }
  if (toReturn >= 1) {
    if ([self isCollapsedInput:section])
      toReturn = 1;
  }
  return toReturn;
}

- (int)totalSources {
  //NSLog(@"patchbay: 6");

  int count = 0;
  for (int i = 1; i <= [self numSourceSections]; ++i) {
    count += [self numSourcesPerSection:i];
  }
  return count;
}

- (int)totalDestinations {
  //NSLog(@"patchbay: 7");

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
  //NSLog(@"patchbay: 8");

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
  //NSLog(@"patchbay: 9");

  int index = 0;
  for (int sect = 1; sect <= [self numDestinationSections]; ++sect) {
    if (targetIndex.port == sect) {
      index += targetIndex.channel - 1;
      break;
    } else {
      //NSLog(@"here, weirdly");
      index += [self numDestinationsPerSection:sect];
    }
  }
  return index;
}

- (device_channel_t)destinationTotalToIndex:(int)totalIndex {
  //NSLog(@"patchbay: 10");

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
  //NSLog(@"patchbay: 11");

  NSString *toReturn;
  Word portID = section;
  if (!self->isMixerTooBool) {
    const AudioPortParm &audioPortParm = self.device->get<AudioPortParm>(portID);
    return @(audioPortParm.portName().c_str());
  }
  else {
    int realSection = floor((section+1)/2);
    //NSLog(@"real section is: %i", realSection);
    if (section % 2) { // odd = true port, not mixer
      const auto &portParm = self.device->get<AudioPortParm>(realSection);
      toReturn = @(portParm.portName().c_str());
    }
    else { // even == mixer port
      const auto &portParm = self.device->get<AudioPortParm>(realSection);
      toReturn = [@(portParm.portName().c_str()) stringByAppendingString:@" Mixer"];
    }
  }
  //NSLog(@"done patchbay: 11");
  return toReturn;
}

- (NSString *)nameForDestinationSection:(int)section {
  //NSLog(@"patchbay: 12");

  NSString *toReturn;
  Word portID = section;
  if (!self->isMixerTooBool) {
    const AudioPortParm &audioPortParm = self.device->get<AudioPortParm>(portID);
    return @(audioPortParm.portName().c_str());
  }
  else {
    int realSection = floor((section+1)/2);
    if (section % 2) { // odd = true port, not mixer
      const auto &portParm = self.device->get<AudioPortParm>(realSection);
      toReturn = @(portParm.portName().c_str());
    }
    else { // even == mixer port
      const auto &portParm = self.device->get<AudioPortParm>(realSection);
      toReturn = [@(portParm.portName().c_str()) stringByAppendingString:@" Mixer"];
    }
  }
  return toReturn;
}

- (bool)isPatchedFrom:(device_channel_t)src to:(device_channel_t)dst {
  //NSLog(@"patchbay: 13 with data: %d, %d, %d, %d", src.port, src.channel, dst.port, dst.channel);

  bool result = false;

  int outPort, outCh, inPort, inCh, origOutPort, origInPort;
  outPort = src.port;
  outCh = src.channel;
  inPort = dst.port;
  inCh = dst.channel;

  if (([self isCollapsedInput:outPort]) || ([self isCollapsedOutput:inPort])) {
    return true;
  }
  origOutPort = outPort;
  origInPort = inPort;

  if (self->isMixerTooBool) {
    outPort = floor((outPort + 1.0)/2.0);
    inPort = floor((inPort + 1.0)/2.0);
  }

  if (((origOutPort % 2) && (origInPort % 2)) || (!self->isMixerTooBool)){ // doesn't involve mixer
    //NSLog(@"How are we here? dst.port: %d, origInPort: %d, ixMixer: %i", dst.port, origInPort, isMixerTooBool);
    const AudioPatchbayParm &audioPatchbay =
    self.device->get<AudioPatchbayParm>((Word)inPort);
    try {
      const auto &block = audioPatchbay.findInputBlock((Byte)inCh);
      result = ((block.portIDOfOutput() == outPort) &&
                (block.outputChannelNumber() == outCh));
    }
    catch (...) {
    }
  }
  else {
    //NSLog(@"isPatched() origOutPort: %d, origInPort: %d", origOutPort, origInPort);

    if (!((origOutPort % 2) || (origInPort % 2))) { // both are mixers, no mixer to mixer!!
      result = true; // make an X

    }
    else if (origInPort % 2) { // outPort (top) is even -> inPort is mixer
      // mixer out (inPort) -> some output (outPort)
      if (inPort == outPort) {
        std::vector<int8_t> outChs = self.mixerInterface->channelIDsForOutput(outPort, outCh);
        std::vector<int8_t>::iterator position = std::find(outChs.begin(), outChs.end(), inCh);
        if (position != outChs.end())
          result = true;
      }
      else {
        result = true; // make an X
      }
      // here!
    }
    else {
      // some input (inPort) -> mixer in (outPort)
      if ((self.mixerInterface->audioPortIDForInput(inPort, inCh) == outPort) &&
          (self.mixerInterface->channelIDForInput(inPort, inCh) == outCh)) {
        result = true;
      }
    }
  }
  //NSLog(@"done 13");
  return result;
}

- (void)setPatchedFrom:(device_channel_t)src to:(device_channel_t)dst andRemove:(device_channel_t)toRemove {
  //NSLog(@"patchbay: 14");

  int outPort, outCh, inPort, inCh, origOutPort, origInPort;
  outPort = src.port;
  outCh = src.channel;
  inPort = dst.port;
  inCh = dst.channel;


  if ((outPort == 0 && [self isCollapsedInput:toRemove.port]) || ([self isCollapsedOutput:inPort]) || (outPort != 0 && [self isCollapsedInput:outPort])) {
    return;
  }

  origOutPort = outPort;
  origInPort = inPort;

  if (self->isMixerTooBool) {
    outPort = floor((outPort + 1.0)/2.0);
    inPort = floor((inPort + 1.0)/2.0);
  }

  if ((((origOutPort % 2) || (origOutPort == 0 && toRemove.port % 2)) && ((origInPort % 2))) || (!self->isMixerTooBool)) { // doesn't involve mixer
    if (self.device->contains(AudioPatchbayParm::queryKey(inPort))) {
      try {
        auto &patchbay = self.device->get<AudioPatchbayParm>(inPort);
        auto &configBlock = patchbay.findInputBlock(inCh);

        configBlock.portIDOfOutput((Word) outPort);
        configBlock.outputChannelNumber((Byte) outCh);

        self.device->send<SetAudioPatchbayParmCommand>(patchbay);
      }
      catch (...) {
      }

      int numOutputs = self.mixerInterface->numberOutputs(inPort);
      for (int j = 1; j <= numOutputs; j++) {
        std::vector<int8_t> outputs = self.mixerInterface->channelIDsForOutput(inPort,j);
        for (int i = 0; i < outputs.size(); i++) {
          if (outputs.at(i) == inCh) {
            outputs.erase(outputs.begin() + i);
            break;
          }
        }
        self.mixerInterface->channelIDsForOutput(inPort, j, outputs);
      }
    }
  }
  else {
    //printf("origOutPort: %d, origInPort: %d\n", origOutPort, origInPort);
    bool remove = false;
    if (origOutPort == 0) {
      remove = true;
      origOutPort = toRemove.port;
      outPort = floor((origOutPort + 1.0)/2.0);
      outCh = toRemove.channel;

    }
    if (!((origOutPort % 2) || (origInPort % 2))) { // both are mixers, no mixer to mixer!!
      return;
    }
    else if (origInPort % 2) { // outPort (top) is even -> outPort is mixer
      if (inPort == outPort) {
        std::vector<int8_t> outChs = self.mixerInterface->channelIDsForOutput(outPort, outCh);
        if (remove) {
          outChs.erase(std::remove(outChs.begin(), outChs.end(), inCh), outChs.end());
        }
        else {
          outChs.push_back(inCh);

          // now erase it from other mixes.
          int numOutputs = self.mixerInterface->numberOutputs(outPort);
          for (int j = 1; j <= numOutputs; j++) {
            if (j != outCh) {
              std::vector<int8_t> outputs = self.mixerInterface->channelIDsForOutput(outPort,j);
              for (int i = 0; i < outputs.size(); i++) {
                if (outputs.at(i) == inCh) {
                  outputs.erase(outputs.begin() + i);
                  break;
                }
              }
              self.mixerInterface->channelIDsForOutput(outPort, j, outputs);
            }
          }
        }
        self.mixerInterface->channelIDsForOutput(outPort, outCh, outChs);
      }

      auto &patchbay = self.device->get<AudioPatchbayParm>(inPort);
      auto &configBlock = patchbay.findInputBlock(inCh);

      configBlock.portIDOfOutput((Word) 0);
      configBlock.outputChannelNumber((Byte) 0);

      self.device->send<SetAudioPatchbayParmCommand>(patchbay);
    }
    else {
      // inPort (side) is even -> inPort is mixer
      if (remove) {
        //NSLog(@"MIXER-IN: %d %d %d %d", inPort, inCh, 0, 0);
        self.mixerInterface->audioPortIDAndChannelIDForInput(inPort, inCh, 0, 0);
      }
      else {
        //NSLog(@"MIXER-IN: %d %d %d %d", inPort, inCh, outPort, outCh);
        self.mixerInterface->audioPortIDAndChannelIDForInput(inPort, inCh, outPort, outCh);
      }
    }
  }
}

- (bool) isMixerToo {
  return self->isMixerTooBool;
}

- (void)toggleCollapseInput:(int)section
{
  //NSLog(@"patchbay: 15");

  self->isCollapsedInputVec.at(section - 1) = !self->isCollapsedInputVec.at(section - 1);
}

- (bool)isCollapsedInput:(int)section
{
  //NSLog(@"patchbay: 16 & section is %i", section);
  //NSLog(@"size is: %lu", self->isCollapsedInputVec.size());


  if (section == 0 || section == 65535)
    return false;
  return self->isCollapsedInputVec.at(section - 1);
}

- (void)toggleCollapseOutput:(int)section
{
  //NSLog(@"patchbay: 17");

  self->isCollapsedOutputVec.at(section - 1) = !self->isCollapsedOutputVec.at(section - 1);
}

- (bool)isCollapsedOutput:(int)section
{
  //NSLog(@"patchbay: 18 & section is %i", section);
  //NSLog(@"size is: %lu", self->isCollapsedOutputVec.size());

  if (section == 0 || section == 65535)
    return false;
  return self->isCollapsedOutputVec.at(section - 1);
}

- (void) foreachMixer:(DevicePairOperator)deviceOperator {
  //NSLog(@"patchbay: 19");

  if (self->isMixerTooBool) {
    for (int i = 2; i <= [self numDestinationSections]; i = i+2) {
      for (int j = 1; j <= self.mixerInterface->numberInputs(i / 2); j++) {
        if (self.mixerInterface->channelIDForInput(i/2, j) &&
            self.mixerInterface->audioPortIDForInput(i/2,j)) {
          device_pair_t pair;

          pair.source.port = self.mixerInterface->audioPortIDForInput(i/2,j) * 2 - 1;
          pair.source.channel = self.mixerInterface->channelIDForInput(i/2, j);
          pair.destination.port = i;
          pair.destination.channel = j;

          if ([self isCollapsedInput:(pair.source.port)])
            pair.source.channel = 1;
          if ([self isCollapsedOutput:(pair.destination.port) ])
            pair.destination.channel = 1;

          //NSLog(@"doing device Operator 19a: %d, %d, %d, %d\n",pair.source.port, pair.source.channel, pair.destination.port, pair.destination.channel);

          deviceOperator(&pair);
        }
      }
      for (int k = 1; k <= self.mixerInterface->numberOutputs(i / 2); k++) {
        std::vector<int8_t> outChs = self.mixerInterface->channelIDsForOutput(i / 2,k);

        if (outChs.size() != 0) {
          for (int m = 0; m < outChs.size(); m++) {
            device_pair_t pair;

            pair.source.port = i;
            pair.source.channel = k;
            pair.destination.port = i - 1;
            pair.destination.channel = outChs.at(m);

            if ([self isCollapsedInput:(pair.source.port)])
              pair.source.channel = 1;
            if ([self isCollapsedOutput:(pair.destination.port) ])
              pair.destination.channel = 1;

            //NSLog(@"doing device Operator 19b: %d, %d, %d, %d\n",pair.source.port, pair.source.channel, pair.destination.port, pair.destination.channel);
            deviceOperator(&pair);
          }
        }
      }
    }
  }
  //NSLog(@"done patchbay: 19");
}

- (void)foreach:(DevicePairOperator)deviceOperator {
  //NSLog(@"patchbay: 20");

  for (int sect = 1; sect <= [self numSourceSections]; ++sect) {
    int realSection = sect;
    if (self->isMixerTooBool)
      realSection = floor((sect+1)/2);
    Word portID = realSection;

    AudioPatchbayParm &audioPatchbay =
    self.device->get<AudioPatchbayParm>(portID);
    for (const auto &item : audioPatchbay.flatList()) {
      if (!self->isMixerTooBool) {
        device_pair_t pair;

        pair.source.port = item.outPortID;
        pair.source.channel = item.outChannelNumber;
        pair.destination.port = item.inPortID;
        pair.destination.channel = item.inChannelNumber;

        if ([self isCollapsedInput:(pair.source.port)])
          pair.source.channel = 1;
        if ([self isCollapsedOutput:(pair.destination.port)])
          pair.destination.channel = 1;

        //NSLog(@"doing device Operator: %d, %d, %d, %d\n",pair.source.port, pair.source.channel, pair.destination.port, pair.destination.channel);
        deviceOperator(&pair);
      }
      else {
        device_pair_t pair;

        pair.source.port = item.outPortID * 2 - 1;
        pair.source.channel = item.outChannelNumber;
        pair.destination.port = item.inPortID * 2 - 1;
        pair.destination.channel = item.inChannelNumber;

        if (pair.source.port < 0 || pair.source.port == 65535)
          pair.source.port = 0;
        if (pair.destination.port < 0 || pair.destination.port == 65535)
          pair.destination.port = 0;

        if ([self isCollapsedInput:(pair.source.port)])
          pair.source.channel = 1;
        if ([self isCollapsedOutput:(pair.destination.port)])
          pair.destination.channel = 1;

        //NSLog(@"doing device Operator 2: %d, %d, %d, %d\n",pair.source.port, pair.source.channel, pair.destination.port, pair.destination.channel);
        deviceOperator(&pair);
      }
    }
  }
  //NSLog(@"done patchbay: 20");

}

- (BOOL)isSourceDeviceInBounds:(device_channel_t)device {
  //NSLog(@"patchbay: 21 w/ info: %i, %i", device.port, device.channel);

  return ((device.port >= 1) && (device.port <= [self numSourceSections]) &&
          (device.channel >= 1) &&
          (device.channel <= [self numSourcesPerSection:device.port]));
}

- (BOOL)isDestinationDeviceInBounds:(device_channel_t)device {
  //NSLog(@"patchbay: 22 w/ info: %i, %i", device.port, device.channel);

  return ((device.port >= 1) &&
          (device.port <= [self numDestinationSections]) &&
          (device.channel >= 1) &&
          (device.channel <= [self numDestinationsPerSection:device.port]));
}

@end
