/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICControllerFilterProvider.h"

#import "MyAlgorithms.h"
#import "ICControllerChoiceDelegate.h"
#import "ICRunOnMain.h"

#import <boost/bind.hpp>

enum FilterCells {
  Channel1Cell = 0,
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
  CCSelectCell
};

using namespace MyAlgorithms;
using namespace GeneSysLib;
using boost::bind;

@interface ICControllerFilterProvider () {
  ICControllerChoiceDelegate *choiceDelegate;
}

@end

@implementation ICControllerFilterProvider

- (id)initWithDevice:(DeviceInfoPtr)device
            forInput:(BOOL)inputType
              portID:(Word)_portID
         forFilterID:(int)_filterID {
  self = [super init];

  if (self) {
    isInput = inputType;
    filterID = _filterID;
    portID = _portID;
    filterType =
        ((isInput) ? (FilterID::InputFilter) : (FilterID::OutputFilter));

    auto &portFilter = device->midiPortFilter(portID, filterType);
    auto &controllerFilter = portFilter.controllerFilter_at(filterID);
    choiceDelegate = [[ICControllerChoiceDelegate alloc]
        initWithSelectedController:controllerFilter.controllerID];
  }

  return self;
}

- (void)initializeProviderButtons:(ICViewController *)sender {
  [super initializeProviderButtons:sender];

  assert(sender.device);
  auto &portFilter = sender.device->midiPortFilter(portID, filterType);
  auto &controllerFilter = portFilter.controllerFilter_at(filterID);

  setMap.clear();
  getMap.clear();

  setMap[Channel1Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::set,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel1, _1);
  getMap[Channel1Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::test,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel1);

  setMap[Channel2Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::set,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel2, _1);
  getMap[Channel2Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::test,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel2);

  setMap[Channel3Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::set,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel3, _1);
  getMap[Channel3Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::test,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel3);

  setMap[Channel4Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::set,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel4, _1);
  getMap[Channel4Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::test,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel4);

  setMap[Channel5Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::set,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel5, _1);
  getMap[Channel5Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::test,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel5);

  setMap[Channel6Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::set,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel6, _1);
  getMap[Channel6Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::test,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel6);

  setMap[Channel7Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::set,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel7, _1);
  getMap[Channel7Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::test,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel7);

  setMap[Channel8Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::set,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel8, _1);
  getMap[Channel8Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::test,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel8);

  setMap[Channel9Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::set,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel9, _1);
  getMap[Channel9Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::test,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel9);

  setMap[Channel10Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::set,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel10, _1);
  getMap[Channel10Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::test,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel10);

  setMap[Channel11Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::set,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel11, _1);
  getMap[Channel11Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::test,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel11);

  setMap[Channel12Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::set,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel12, _1);
  getMap[Channel12Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::test,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel12);

  setMap[Channel13Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::set,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel13, _1);
  getMap[Channel13Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::test,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel13);

  setMap[Channel14Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::set,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel14, _1);
  getMap[Channel14Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::test,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel14);

  setMap[Channel15Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::set,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel15, _1);
  getMap[Channel15Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::test,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel15);

  setMap[Channel16Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::set,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel16, _1);
  getMap[Channel16Cell] =
      bind(&MIDIPortFilter::ChannelBitmap::test,
           &(controllerFilter.channelBitmap), ChannelBitmapBit::channel16);

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
    [[ICControllerChoiceDelegate contollerList]
        objectAtIndex:controllerFilter.controllerID]
  ];

  title = [NSString stringWithFormat:@"Controller Filter %d", (filterID + 1)];
}

- (void)providerWillAppear:(ICViewController *)sender {
  for (const auto &getPair : getMap) {
    const auto &selected = !getPair.second();
    [(sender.providerButtons)[getPair.first] setSelected:selected];
  }

  assert(sender.device);
  auto &portFilter = sender.device->midiPortFilter(portID, filterType);
  auto &controllerFilter = portFilter.controllerFilter_at(filterID);

  if (controllerFilter.controllerID != [choiceDelegate getChoice]) {
    runOnMain(^{
        controllerFilter.controllerID = [choiceDelegate getChoice];
        [(sender.providerButtons)[CCSelectCell]
            setTitle:[ICControllerChoiceDelegate contollerList]
                         [controllerFilter.controllerID]
            forState:UIControlStateNormal];
        [self startUpdateTimer];
    });
  }
}

- (NSString *)providerName {
  return ((isInput) ? (@"ControllerFiltersInputControllerSelection")
                    : (@"ControllerFiltersOutputControllerSelection"));
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
  return ((index <= Channel16Cell) ? (1) : (4));
}

- (void)onButtonDown:(ICViewController *)sender index:(NSInteger)buttonIndex {
  assert(sender.device);
  __block auto &portFilter = sender.device->midiPortFilter(portID, filterType);

  if (buttonIndex <= Channel16Cell) {
    setMap[buttonIndex]([(sender.providerButtons)[buttonIndex] isSelected]);

    [(sender.providerButtons)[buttonIndex]
        setSelected:![(sender.providerButtons)[buttonIndex] isSelected]];
    [self startUpdateTimer];
  } else if (buttonIndex == CCSelectCell) {
    if ((filterID < portFilter.controllerFilters().size())) {
      [self onTimerHandler];

      auto *const selectionViewController =
          [[ICChoiceSelectionViewController alloc]
              initWithDelegate:choiceDelegate];
      [sender.navigationController pushViewController:selectionViewController
                                             animated:YES];
    }
  }
}

- (void)onUpdate:(DeviceID)deviceID transID:(Word)transID {
  assert(self.parent);
  assert(self.parent.comm);
  const auto &portFilter =
      self.parent.device->midiPortFilter(portID, filterType);
  self.parent.device->send<SetMIDIPortFilterCommand>(portFilter);
}

- (NSString *)title {
  return title;
}

@end
