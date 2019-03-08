/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICAudioSetupV1ChoiceDelegate.h"
#import "AudioCfgInfo.h"
#import "AudioPortCfgInfo.h"
#import "AudioPortInfo.h"

@interface ICAudioSetupV1ChoiceDelegate ()

@property(nonatomic) DeviceInfoPtr device;

@end

@implementation ICAudioSetupV1ChoiceDelegate

+ (ICAudioSetupV1ChoiceDelegate *)audioSetupWithDevice:(DeviceInfoPtr)device {
  return [[ICAudioSetupV1ChoiceDelegate alloc] initWithDevice:device];
}

- (id)initWithDevice:(DeviceInfoPtr)device {
  using namespace GeneSysLib;
  self = [super init];

  if (self) {
    self.device = device;

    AudioCfgInfo &audioCfgInfo = self.device->get<AudioCfgInfo>();

    // audio configuration
    auto *const optionArray = [NSMutableArray array];

    for (const auto &block : audioCfgInfo.configBlocks) {
      auto *const configuration = [NSMutableString string];
      [configuration appendFormat:@"%s", SampleRate::toString(
                                             block.sampleRateCode).c_str()];
      [configuration appendString:@"Hz, "];
      [configuration appendFormat:@"%d Ch., ", block.numAudioChannels];
      [configuration
          appendFormat:@"%s", BitDepth::toString(block.bitDepthCode).c_str()];
      [optionArray addObject:configuration];
    }

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
  using namespace GeneSysLib;
  AudioCfgInfo &audioCfgInfo = self.device->get<AudioCfgInfo>();
  return (NSInteger)audioCfgInfo.currentActiveConfig() - 1;
}

- (void)setChoice:(NSInteger)value {
  using namespace GeneSysLib;
  AudioCfgInfo &audioCfgInfo = self.device->get<AudioCfgInfo>();
  audioCfgInfo.currentActiveConfig(value + 1);
  self.device->send<SetAudioCfgInfoCommand>(audioCfgInfo);
  usleep(10000);

  for (auto portID = 1; portID <= self.device->audioPortInfoCount(); ++portID) {
    AudioPortCfgInfo &audioCfgPortInfo =
        self.device->get<AudioPortCfgInfo>(portID);

    auto numChannels = audioCfgInfo.activeConfigBlock().numAudioChannels / 2;

    audioCfgPortInfo.numInputChannels(numChannels);
    audioCfgPortInfo.numOutputChannels(numChannels);
    self.device->send<SetAudioPortCfgInfoCommand>(audioCfgPortInfo);

    usleep(10000);

    self.device->send<GetAudioPortInfoCommand>(audioCfgPortInfo.portID());
  }
}

@end
