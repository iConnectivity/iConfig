/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICAudioMixerChannelV2ChoiceDelegate.h"
#import "MixerParm.h"
#import "MixerPortParm.h"
#import "MixerInterface.h"
#import "AudioGlobalParm.h"
#import "AudioPortParm.h"

using namespace GeneSysLib;

@interface ICAudioMixerChannelV2ChoiceDelegate ()
@property(nonatomic) DeviceInfoPtr device;
@property(nonatomic) MixerInterface* mixerInterface;
@property(nonatomic) Word audioPortID;
@property(nonatomic) Word isInput;
@end

@implementation ICAudioMixerChannelV2ChoiceDelegate

- (id)initWithDevice:(DeviceInfoPtr)device portID:(Word)portID isInput:(bool)isInput {
  self = [super init];

  if (self) {
    self.device = device;
    self.mixerInterface = new MixerInterface(self.device);
    self.audioPortID = portID;
    self.isInput = isInput;
  }

  return self;
}

- (NSString *)title {
  const AudioPortParm &audioPortParm =
  self.device->get<AudioPortParm>(self.audioPortID);
  NSString *domain = @(audioPortParm.portName().c_str());

  if (self.isInput) {
    return [NSString stringWithFormat:@"Input channels for mixer on %@ domain", domain];
  }
  else {
    return [NSString stringWithFormat:@"Mixes to outputs on %@ domain", domain];
  }

}

- (NSArray *)options {
  NSMutableArray *options = [NSMutableArray array];

  if (self.isInput) {
    for (int i = 0; i <= self.mixerInterface->maximumInputs(self.mixerInterface->activeMixerConfigurationNumber()); i++) {
      NSString *str = [NSString stringWithFormat:@"%d", i];
      [options addObject:str];
    }
  }
  else {
    for (int i = 0; i <= self.mixerInterface->maximumOutputs(self.mixerInterface->activeMixerConfigurationNumber()); i=i+2) {
      NSString *str = [NSString stringWithFormat:@"%d", i];
      [options addObject:str];
    }
  }

  return options;
}

- (NSInteger)optionCount {
  return [[self options] count];
}

- (NSInteger)getChoice {
  if (self.isInput) {
    MixerPortParm &mixerPortParm = self.device->get<MixerPortParm>();
    return mixerPortParm.audioPortMixerBlocks.at(self.audioPortID - 1).numInputs();
  }
  else {
    MixerPortParm &mixerPortParm = self.device->get<MixerPortParm>();
    return mixerPortParm.audioPortMixerBlocks.at(self.audioPortID - 1).numOutputs()/2;
  }
}

- (void)setChoice:(NSInteger)value {
  if (self.isInput) {
    MixerPortParm &mixerPortParm = self.device->get<MixerPortParm>();
    mixerPortParm.audioPortMixerBlocks.at(self.audioPortID - 1).numInputs(value);
    self.device->send<SetMixerPortParmCommand>(mixerPortParm);
  }
  else {
    MixerPortParm &mixerPortParm = self.device->get<MixerPortParm>();
    mixerPortParm.audioPortMixerBlocks.at(self.audioPortID - 1).numOutputs(value);
    self.device->send<SetMixerPortParmCommand>(mixerPortParm);
  }
}

@end
