/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICControllerSelectionProvider.h"
#import "ICControllerRemapProvider.h"
#import "ICControllerChoiceDelegate.h"
#import "MyAlgorithms.h"
#import "ICRunOnMain.h"
#import "ICRemapSourceControllerChoiceDelegate.h"
#import "ICRemapDestinationControllerChoiceDelegate.h"

#import <boost/bind.hpp>

#define kSourceButtonIndex 16
#define kDestinationButtonIndex 17

using namespace MyAlgorithms;
using namespace GeneSysLib;
using boost::bind;

enum CCRemapCell {
  Channel1Cell,
  Channel2Cell,
  Channel3Cell,
  Channel4Cell,
  Channel5Cell,
  Channel6Cell,
  Channel7Cell,
  Channel8Cell,
  Channel9Cell,
  Channel10Cell,
  Channel11Cell,
  Channel12Cell,
  Channel13Cell,
  Channel14Cell,
  Channel15Cell,
  Channel16Cell,
  SourceCell,
  DestinationCell
};

@interface ICControllerRemapProvider ()

@property(nonatomic, strong)
    ICRemapSourceControllerChoiceDelegate *sourceControllerDelegate;

@property(nonatomic, strong)
    ICRemapDestinationControllerChoiceDelegate *destinationControllerDelegate;

@end

@implementation ICControllerRemapProvider

- (id)initForInput:(BOOL)inputType
            portID:(Word)_portID
       withRemapID:(int)_remapID {
  self = [super init];
  if (self) {
    isInput = inputType;
    remapID = _remapID;
    portID = _portID;
    remapType = ((isInput) ? (RemapID::InputRemap) : (RemapID::OutputRemap));
  }
  return self;
}

- (void)initializeProviderButtons:(ICViewController *)sender {
  [super initializeProviderButtons:sender];
  self.sourceControllerDelegate = [[ICRemapSourceControllerChoiceDelegate alloc]
      initWithDevice:sender.device
              portID:portID
           remapType:remapType
             remapID:remapID];

  self.destinationControllerDelegate =
      [[ICRemapDestinationControllerChoiceDelegate alloc]
          initWithDevice:sender.device
                  portID:portID
               remapType:remapType
                 remapID:remapID];

  NSParameterAssert(sender);
  assert(sender.device);
  assert(sender.device->contains<MIDIPortRemap>(portID, remapType));
  auto &portRemap = sender.device->midiPortRemap(portID, remapType);
  assert(portRemap.numControllers() > remapID);
  auto &controllerRemap = portRemap.controller_at(remapID);

  setMap.clear();
  getMap.clear();

  setMap[Channel1Cell] =
      bind(&MIDIPortRemap::ChannelBitmap::set, &(controllerRemap.channelBitmap),
           MIDIPortRemap::ChannelBitmapBits::channel1, _1);
  getMap[Channel1Cell] = bind(&MIDIPortRemap::ChannelBitmap::test,
                              &(controllerRemap.channelBitmap),
                              MIDIPortRemap::ChannelBitmapBits::channel1);

  setMap[Channel2Cell] =
      bind(&MIDIPortRemap::ChannelBitmap::set, &(controllerRemap.channelBitmap),
           MIDIPortRemap::ChannelBitmapBits::channel2, _1);
  getMap[Channel2Cell] = bind(&MIDIPortRemap::ChannelBitmap::test,
                              &(controllerRemap.channelBitmap),
                              MIDIPortRemap::ChannelBitmapBits::channel2);

  setMap[Channel3Cell] =
      bind(&MIDIPortRemap::ChannelBitmap::set, &(controllerRemap.channelBitmap),
           MIDIPortRemap::ChannelBitmapBits::channel3, _1);
  getMap[Channel3Cell] = bind(&MIDIPortRemap::ChannelBitmap::test,
                              &(controllerRemap.channelBitmap),
                              MIDIPortRemap::ChannelBitmapBits::channel3);

  setMap[Channel4Cell] =
      bind(&MIDIPortRemap::ChannelBitmap::set, &(controllerRemap.channelBitmap),
           MIDIPortRemap::ChannelBitmapBits::channel4, _1);
  getMap[Channel4Cell] = bind(&MIDIPortRemap::ChannelBitmap::test,
                              &(controllerRemap.channelBitmap),
                              MIDIPortRemap::ChannelBitmapBits::channel4);

  setMap[Channel5Cell] =
      bind(&MIDIPortRemap::ChannelBitmap::set, &(controllerRemap.channelBitmap),
           MIDIPortRemap::ChannelBitmapBits::channel5, _1);
  getMap[Channel5Cell] = bind(&MIDIPortRemap::ChannelBitmap::test,
                              &(controllerRemap.channelBitmap),
                              MIDIPortRemap::ChannelBitmapBits::channel5);

  setMap[Channel6Cell] =
      bind(&MIDIPortRemap::ChannelBitmap::set, &(controllerRemap.channelBitmap),
           MIDIPortRemap::ChannelBitmapBits::channel6, _1);
  getMap[Channel6Cell] = bind(&MIDIPortRemap::ChannelBitmap::test,
                              &(controllerRemap.channelBitmap),
                              MIDIPortRemap::ChannelBitmapBits::channel6);

  setMap[Channel7Cell] =
      bind(&MIDIPortRemap::ChannelBitmap::set, &(controllerRemap.channelBitmap),
           MIDIPortRemap::ChannelBitmapBits::channel7, _1);
  getMap[Channel7Cell] = bind(&MIDIPortRemap::ChannelBitmap::test,
                              &(controllerRemap.channelBitmap),
                              MIDIPortRemap::ChannelBitmapBits::channel7);

  setMap[Channel8Cell] =
      bind(&MIDIPortRemap::ChannelBitmap::set, &(controllerRemap.channelBitmap),
           MIDIPortRemap::ChannelBitmapBits::channel8, _1);
  getMap[Channel8Cell] = bind(&MIDIPortRemap::ChannelBitmap::test,
                              &(controllerRemap.channelBitmap),
                              MIDIPortRemap::ChannelBitmapBits::channel8);

  setMap[Channel9Cell] =
      bind(&MIDIPortRemap::ChannelBitmap::set, &(controllerRemap.channelBitmap),
           MIDIPortRemap::ChannelBitmapBits::channel9, _1);
  getMap[Channel9Cell] = bind(&MIDIPortRemap::ChannelBitmap::test,
                              &(controllerRemap.channelBitmap),
                              MIDIPortRemap::ChannelBitmapBits::channel9);

  setMap[Channel10Cell] =
      bind(&MIDIPortRemap::ChannelBitmap::set, &(controllerRemap.channelBitmap),
           MIDIPortRemap::ChannelBitmapBits::channel10, _1);
  getMap[Channel10Cell] = bind(&MIDIPortRemap::ChannelBitmap::test,
                               &(controllerRemap.channelBitmap),
                               MIDIPortRemap::ChannelBitmapBits::channel10);

  setMap[Channel11Cell] =
      bind(&MIDIPortRemap::ChannelBitmap::set, &(controllerRemap.channelBitmap),
           MIDIPortRemap::ChannelBitmapBits::channel11, _1);
  getMap[Channel11Cell] = bind(&MIDIPortRemap::ChannelBitmap::test,
                               &(controllerRemap.channelBitmap),
                               MIDIPortRemap::ChannelBitmapBits::channel11);

  setMap[Channel12Cell] =
      bind(&MIDIPortRemap::ChannelBitmap::set, &(controllerRemap.channelBitmap),
           MIDIPortRemap::ChannelBitmapBits::channel12, _1);
  getMap[Channel12Cell] = bind(&MIDIPortRemap::ChannelBitmap::test,
                               &(controllerRemap.channelBitmap),
                               MIDIPortRemap::ChannelBitmapBits::channel12);

  setMap[Channel13Cell] =
      bind(&MIDIPortRemap::ChannelBitmap::set, &(controllerRemap.channelBitmap),
           MIDIPortRemap::ChannelBitmapBits::channel13, _1);
  getMap[Channel13Cell] = bind(&MIDIPortRemap::ChannelBitmap::test,
                               &(controllerRemap.channelBitmap),
                               MIDIPortRemap::ChannelBitmapBits::channel13);

  setMap[Channel14Cell] =
      bind(&MIDIPortRemap::ChannelBitmap::set, &(controllerRemap.channelBitmap),
           MIDIPortRemap::ChannelBitmapBits::channel14, _1);
  getMap[Channel14Cell] = bind(&MIDIPortRemap::ChannelBitmap::test,
                               &(controllerRemap.channelBitmap),
                               MIDIPortRemap::ChannelBitmapBits::channel14);

  setMap[Channel15Cell] =
      bind(&MIDIPortRemap::ChannelBitmap::set, &(controllerRemap.channelBitmap),
           MIDIPortRemap::ChannelBitmapBits::channel15, _1);
  getMap[Channel15Cell] = bind(&MIDIPortRemap::ChannelBitmap::test,
                               &(controllerRemap.channelBitmap),
                               MIDIPortRemap::ChannelBitmapBits::channel15);

  setMap[Channel16Cell] =
      bind(&MIDIPortRemap::ChannelBitmap::set, &(controllerRemap.channelBitmap),
           MIDIPortRemap::ChannelBitmapBits::channel16, _1);
  getMap[Channel16Cell] = bind(&MIDIPortRemap::ChannelBitmap::test,
                               &(controllerRemap.channelBitmap),
                               MIDIPortRemap::ChannelBitmapBits::channel16);

  buttonNames = @[
    @"Channel 1",
    @"Channel 2",
    @"Channel 3",
    @"Channel 4",
    @"Channel 5",
    @"Channel 6",
    @"Channel 7",
    @"Channel 8",
    @"Channel 9",
    @"Channel 10",
    @"Channel 11",
    @"Channel 12",
    @"Channel 13",
    @"Channel 14",
    @"Channel 15",
    @"Channel 16",
    [self.sourceControllerDelegate title],
    [self.destinationControllerDelegate title]
  ];

  title = [NSString stringWithFormat:@"Controller Remap ID %d", (remapID + 1)];
}

- (NSString *)title {
  return title;
}

- (NSString *)providerName {
  return ((isInput) ? (@"ControllerRemapInputSetup")
                    : (@"ControllerRemapOutputSetup"));
}

- (NSArray *)buttonNames {
  return buttonNames;
}

- (NSUInteger)numberOfColumns {
  return 4;
}

- (NSUInteger)numberOfRows {
  return 5;
}

- (NSUInteger)spanForIndex:(NSUInteger)index {
  return ((index <= Channel16Cell) ? (1) : (2));
}

- (void)providerWillAppear:(ICViewController *)sender {
  for (const auto &getPair : getMap) {
    bool selected = !getPair.second();
    [(sender.providerButtons)[getPair.first] setSelected:selected];
  }

  // Format source button
  if (self.sourceControllerDelegate) {
    [(sender.providerButtons)[kSourceButtonIndex]
        setTitle:[self.sourceControllerDelegate title]
        forState:UIControlStateNormal];
  }

  // Format destination button
  {
    [(sender.providerButtons)[kDestinationButtonIndex]
        setTitle:[self.destinationControllerDelegate title]
        forState:UIControlStateNormal];
  }
}

- (void)onButtonDown:(ICViewController *)sender index:(NSInteger)buttonIndex {
  if (buttonIndex <= Channel16Cell) {
    setMap[buttonIndex]([(sender.providerButtons)[buttonIndex] isSelected]);

    [(sender.providerButtons)[buttonIndex]
        setSelected:![(sender.providerButtons)[buttonIndex] isSelected]];
    [self startUpdateTimer];
  } else if (buttonIndex == SourceCell) {
    ICChoiceSelectionViewController *choiceSelectionViewController =
        [[ICChoiceSelectionViewController alloc]
            initWithDelegate:self.sourceControllerDelegate];

    [sender.navigationController
        pushViewController:choiceSelectionViewController
                  animated:YES];
  } else if (buttonIndex == DestinationCell) {
    ICChoiceSelectionViewController *controllerSelectionView =
        [[ICChoiceSelectionViewController alloc]
            initWithDelegate:self.destinationControllerDelegate];

    [sender.navigationController pushViewController:controllerSelectionView
                                           animated:YES];
  }
}

- (void)onUpdate:(DeviceID)deviceID transID:(Word)transID {
  assert(self.parent);
  assert(self.parent.comm);
  assert(self.parent.device);
  assert(self.parent.device->contains<MIDIPortRemap>(portID, remapType));
  const auto &portRemap = self.parent.device->midiPortRemap(portID, remapType);
  self.parent.device->send<SetMIDIPortRemapCommand>(portRemap);
}

@end
