/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICAudioMixerConfigurationV2ChoiceDelegate.h"
#import "MixerParm.h"
#import "MixerInterface.h"
#import "AudioGlobalParm.h"

using namespace GeneSysLib;

@interface ICAudioMixerConfigurationV2ChoiceDelegate ()
@property(nonatomic) DeviceInfoPtr device;
@property(nonatomic) MixerInterface* mixerInterface;
@end

@implementation ICAudioMixerConfigurationV2ChoiceDelegate

- (id)initWithDevice:(DeviceInfoPtr)device {
  self = [super init];

  if (self) {
    self.device = device;
    self.mixerInterface = new MixerInterface(self.device);
  }

  return self;
}

- (NSString *)title {
  return @"Mixer Configuration";
}

- (NSArray *)options {
  const auto &audioGlobalParm = self.device->get<AudioGlobalParm>();
  MixerParm &mixerParm = self.device->get<MixerParm>(audioGlobalParm.currentActiveConfig());

  NSMutableArray *options = [NSMutableArray array];

  for (int i = 0; i < mixerParm.mixerBlockCount(); i++) {
    NSString *str = [NSString stringWithFormat:@"%d - %d Input Buses per Mixer, %d Mix Buses",
                     i + 1,
                     mixerParm.mixerBlocks.at(i).maximumInputs(),
                     mixerParm.mixerBlocks.at(i).maximumOutputs()];
    [options addObject:str];
  }

  return options;
}

- (NSInteger)optionCount {
  return [[self options] count];
}

- (NSInteger)getChoice {
  const auto &audioGlobalParm = self.device->get<AudioGlobalParm>();
  MixerParm &mixerParm = self.device->get<MixerParm>(audioGlobalParm.currentActiveConfig());
  return mixerParm.activeMixerConfigurationBlock() - 1;
}

- (void)setChoice:(NSInteger)value {
  self.mixerInterface->activeMixerConfigurationNumber(value + 1);
  self.device->rereadAudioInfo();
}

@end
