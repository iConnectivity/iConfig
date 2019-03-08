/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICAudioSetupV2ChoiceDelegate.h"
#import "AudioGlobalParm.h"
#import "AudioPatchbayParm.h"
#import "AudioPortParm.h"
#import "CommandList.h"

using namespace GeneSysLib;

@interface ICAudioSetupV2ChoiceDelegate ()

@property(nonatomic) DeviceInfoPtr device;

@end

@implementation ICAudioSetupV2ChoiceDelegate

+ (ICAudioSetupV2ChoiceDelegate *)audioSetupWithDevice:(DeviceInfoPtr)device {
  return [[ICAudioSetupV2ChoiceDelegate alloc] initWithDevice:device];
}

- (id)initWithDevice:(DeviceInfoPtr)device {
  using namespace GeneSysLib;
  self = [super init];

  if (self) {
    self.device = device;

    NSParameterAssert(device->contains<AudioGlobalParm>());

    const auto &audioGlobalParm = device->get<AudioGlobalParm>();
    NSMutableArray *optionArray = [NSMutableArray array];

    // audio configuration
    audioGlobalParm.for_each([&](
        const AudioGlobalParm::ConfigBlock &configBlock) {
      [optionArray
          addObject:[NSString
                        stringWithFormat:@"%d - %sHz, %s", configBlock.number(),
                                         SampleRate::toString(
                                             configBlock.sampleRate()).c_str(),
                                         BitDepth::toString(
                                             configBlock.bitDepth()).c_str()]];
    });

    self.options = optionArray;
  }

  return self;
}

- (NSString *)title {
  return @"Setup";
}

- (NSInteger)optionCount {
  return [self.options count];
}

- (NSInteger)getChoice {
  const auto &audioGlobalParm = self.device->get<AudioGlobalParm>();
  return (NSInteger)audioGlobalParm.currentActiveConfig() - 1;
}

- (void)setChoice:(NSInteger)value {
  auto &audioGlobalParm = self.device->get<AudioGlobalParm>();

  audioGlobalParm.currentActiveConfig(value + 1);

  self.device->send<SetAudioGlobalParmCommand>(audioGlobalParm);

  // Set the number of audio in/out channels for each port to 1/2 the total
  // number of channels
  self.device->for_each<AudioPortParm>([=](AudioPortParm &audioPortParm) {
    usleep(10000);
    const auto &configBlock =
        audioPortParm.block_at(audioGlobalParm.currentActiveConfig());

    Byte avgInChannels =
        (configBlock.minInputChannels() + configBlock.maxInputChannels()) / 2;
    Byte avgOutChannels =
        (configBlock.minOutputChannels() + configBlock.maxOutputChannels()) / 2;
    audioPortParm.numInputChannels(avgInChannels);
    audioPortParm.numOutputChannels(avgOutChannels);

    self.device->send<SetAudioPortParmCommand>(audioPortParm);
  });
}

@end
