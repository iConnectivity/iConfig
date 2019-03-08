/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "AudioCfgInfo.h"
#import "AudioInfo.h"
#import "AudioPortCfgInfo.h"
#import "AudioPortInfo.h"
#import "AudioPortPatchbay.h"
#import "ICAudioPatchbayV1OutputSelection.h"
#import "ICIndicatorButton.h"
#import "MyAlgorithms.h"

using namespace GeneSysLib;
using namespace MyAlgorithms;
using namespace std;

@implementation ICAudioPatchbayOutputV1Selection

- (id)initWithPatchbay:(AudioPortPatchbay)_audioPortPatchbay
            blockIndex:(size_t)_configBlockIndex
         selectedTitle:(NSString *)_title {
  self = [super init];

  if (self) {
    audioPortPatchbay = _audioPortPatchbay;
    configBlockIndex = _configBlockIndex;
    title = [NSString stringWithFormat:@"%@ From", _title];
  }

  return self;
}

- (void)initializeProviderButtons:(ICViewController *)sender {
  [super initializeProviderButtons:sender];

  const auto &device = sender.device;

  assert(device->containsCommandData(Command::RetAudioInfo));

  const auto &audioInfo = device->get<AudioInfo>();

  if (audioInfo.versionNumber() == 0x01) {
    auto *const tempButtonNames = [NSMutableArray array];
    NSInteger buttonIndex = 0;
    [tempButtonNames addObject:@"None"];

    indexToOutputPair[buttonIndex] = make_pair(buttonIndex, 0);
    ++buttonIndex;

    device->for_each<AudioPortInfo>([&](AudioPortInfo &port) {
      // Audio Port Name
      assert(device->contains<AudioPortCfgInfo>(port.portID()));
      audioPortCfgInfo = device->get<AudioPortCfgInfo>(port.portID());
      for (auto channel = 1; channel <= audioPortCfgInfo.numOutputChannels();
           ++channel) {
        [tempButtonNames
            addObject:[NSString stringWithFormat:@"%s Ch %d",
                                                 port.portName().c_str(),
                                                 channel]];
        indexToOutputPair[buttonIndex] = make_pair(port.portID(), channel);
        ++buttonIndex;
      }
    });

    buttonNames = tempButtonNames;
  }
}

- (void)providerWillAppear:(ICViewController *)sender {
  for (auto i = 0; i < [sender.providerButtons count]; ++i) {
    try {
      const auto &pair = indexToOutputPair[i];
      const auto &configBlock =
          audioPortPatchbay.findOutputBlock(configBlockIndex);
      [(sender.providerButtons)[i]
          setSelected:((configBlock.portIDOfOutput() == pair.first) &&
                       (configBlock.outputChannelNumber() == pair.second))];
    }
    catch (...) {
    }
  }
}

- (NSString *)title {
  return title;
}

- (NSString *)providerName {
  return @"AudioPatchbayOutputSelection";
}

- (NSArray *)buttonNames {
  return buttonNames;
}

- (NSUInteger)numberOfRows {
  return MAX(1, (NSUInteger)ceil((float)[buttonNames count] /
                                 (float)[self numberOfColumns]));
}

- (NSUInteger)numberOfColumns {
  return MAX(1, (int)floor(sqrt([buttonNames count])));
}

- (NSUInteger)spanForIndex:(NSUInteger)index {
  NSUInteger result = 1;
  if (index == ([buttonNames count] - 1)) {
    result = 1 + ([self numberOfColumns] * [self numberOfRows]) %
                     [buttonNames count];
  }
  return result;
}

- (void)onButtonPressed:(ICViewController *)sender
                  index:(NSInteger)buttonIndex {
  if (contains(indexToOutputPair, buttonIndex)) {
    const auto &outputPair = indexToOutputPair[buttonIndex];
    try {
      auto &configBlock = audioPortPatchbay.findOutputBlock(configBlockIndex);
      configBlock.portIDOfOutput(outputPair.first);
      configBlock.outputChannelNumber(outputPair.second);
    }
    catch (...) {
    }
  }
  for (auto i = 0; i < [sender.providerButtons count]; ++i) {
    [(sender.providerButtons)[i] setSelected:(i == buttonIndex)];
  }

  [self startUpdateTimer];
}

@end
