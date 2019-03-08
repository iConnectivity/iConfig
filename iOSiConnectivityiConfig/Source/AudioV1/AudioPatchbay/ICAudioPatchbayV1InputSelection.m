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
#import "ICAudioPatchbayV1InputSelection.h"
#import "ICAudioPatchbayV1OutputSelection.h"
#import "ICRunOnMain.h"
#import "MyAlgorithms.h"
#include "AudioPortPatchbay.h"
#include "blockNotFound.h"

#import <boost/range/adaptors.hpp>

using namespace GeneSysLib;
using namespace MyAlgorithms;
using namespace boost::adaptors;
using namespace std;

@implementation ICAudioPatchbayV1InputSelection

- (void)initializeProviderButtons:(ICViewController *)sender {
  auto device = sender.device;

  assert(device->containsCommandData(Command::RetAudioInfo));

  const AudioInfo &audioInfo = device->get<AudioInfo>();

  if (audioInfo.versionNumber() == 0x01) {
    auto *const tempButtonNames = [NSMutableArray array];
    NSInteger buttonIndex = 0;
    device->for_each<AudioPortInfo>([&](AudioPortInfo &audioPortInfo) {
      Word portID = audioPortInfo.portID();
      assert(device->contains<AudioPortPatchbay>(portID));
      const AudioPortCfgInfo &audioPortCfgInfo =
          device->get<AudioPortCfgInfo>(portID);
      AudioPortPatchbay &portPatchbay = device->get<AudioPortPatchbay>(portID);

      // validate the input
      for (auto channel = 1; channel <= audioPortCfgInfo.numInputChannels();
           ++channel) {

        try {
          auto &configBlock = portPatchbay.findInputBlock(channel);

          bool sanitized = false;
          if (!device->contains<AudioPortCfgInfo>(
                  configBlock.portIDOfOutput())) {
            if (configBlock.portIDOfOutput() != 0x00) {
              // patch the port to nothing
              configBlock.portIDOfOutput(0);
              configBlock.outputChannelNumber(0);
              sanitized = true;
            }
          } else {
            const auto &outConfig =
                device->get<AudioPortCfgInfo>(configBlock.portIDOfOutput());

            if (configBlock.outputChannelNumber() >
                outConfig.numOutputChannels()) {
              // patch the port to nothing
              configBlock.portIDOfOutput(0);
              configBlock.outputChannelNumber(0);
              sanitized = true;
            }
          }
          if (sanitized) {
            runOnMainAfter(0.2, ^{
                sender.device->send<SetAudioPortPatchbayCommand>(portPatchbay);
            });
          }
        }
        catch (blockNotFoundException) {
          // could not find block
#if 0
          AudioPortPatchbay::PatchbayConfigBlock block;
          AudioPortPatchbay::ConfigBlock
          block.inputChannelNumber = channel;
          block.portIDOfOutput = 0;
          block.outputChannelNumber = 0;
          portPatchbay.configBlocks.push_back(block);
#endif
        }
      }

      // create the buttons
      for (auto channel = 1; channel <= audioPortCfgInfo.numInputChannels();
           ++channel) {
        [tempButtonNames
            addObject:[NSString
                          stringWithFormat:@"%s Ch %d",
                                           audioPortInfo.portName().c_str(),
                                           channel]];
        try {
          const auto &configBlockIndex =
              portPatchbay.indexOfInputBlock(channel);
          indexToAudioPatchbayBlockPair[buttonIndex] =
              make_pair(portPatchbay, configBlockIndex);
        }
        catch (blockNotFoundException) {
        }
        ++buttonIndex;
      }
    });
    buttonNames = tempButtonNames;
  }
}

- (NSString *)providerName {
  return @"AudioPatchbayInput";
}

- (NSArray *)buttonNames {
  return buttonNames;
}

- (NSUInteger)numberOfRows {
  return MAX(1, (NSUInteger)ceil((float)[buttonNames count] /
                                 (float)[self numberOfColumns]));
}

- (NSUInteger)numberOfColumns {
  NSUInteger num = [buttonNames count];
  NSUInteger result = num;

  for (NSUInteger i = 2; i < num; ++i) {
    if (((num % i) == 0) && ((i + num / i) < (result + num / result))) {
      result = i;
    }
  }
  if (result == num) {
    result = MAX(1, (int)floor(sqrt([buttonNames count])));
  }

  return result;
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
  if (contains(indexToAudioPatchbayBlockPair, buttonIndex)) {
    auto audioPatchbayBlockPair = indexToAudioPatchbayBlockPair[buttonIndex];
    auto *const outputProvider = [[ICAudioPatchbayOutputV1Selection alloc]
        initWithPatchbay:audioPatchbayBlockPair.first
              blockIndex:audioPatchbayBlockPair.second
           selectedTitle:[(sender.providerButtons)[buttonIndex] title]];
    [sender pushToProvider:outputProvider animated:YES];
  }
}

@end
