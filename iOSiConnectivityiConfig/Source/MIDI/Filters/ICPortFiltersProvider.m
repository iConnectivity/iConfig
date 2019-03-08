/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICPortFiltersProvider.h"
#import "ICPortFiltersChannelProvider.h"
#import "MyAlgorithms.h"
#include "MIDIPortInfo.h"

#import <boost/bind.hpp>

using namespace MyAlgorithms;
using namespace GeneSysLib;
using boost::bind;

enum PortFilterCells {
  PitchBendCell,
  ChannelPressureCell,
  ProgramChangeCell,
  ControlChangeCell,
  PolyKeyPressureCell,
  NoteCell,
  ResetCell,
  ActiveSensingCell,
  RealtimeCell,
  TuneRequestCell,
  SongSelectCell,
  SongPositionPointerCell,
  TimeCodeCell,
  SystemExclusiveCell
};

@implementation ICPortFiltersProvider

- (id)initWithInputType:(BOOL)inputType withPortID:(Word)_portID {
  self = [super init];

  if (self) {
    isInput = inputType;
    filterID = ((isInput) ? (FilterID::InputFilter) : (FilterID::OutputFilter));
    portID = _portID;

    parent = nil;
  }

  return self;
}

- (void)initializeProviderButtons:(ICViewController *)sender {
  [super initializeProviderButtons:sender];

  parent = sender;

  assert(sender.device);
  self.parent = sender;
  auto &portFilter = sender.device->midiPortFilter(portID, filterID);

  setMap.clear();
  getMap.clear();

  getMap[PitchBendCell] = bind(&MIDIPortFilter::allChannelsSet, &portFilter,
                               ChannelFilterStatusBit::pitchBendEvents);
  setMap[PitchBendCell] = bind(&MIDIPortFilter::setAllChannels, &portFilter,
                               ChannelFilterStatusBit::pitchBendEvents, _1);

  getMap[ChannelPressureCell] =
      bind(&MIDIPortFilter::allChannelsSet, &portFilter,
           ChannelFilterStatusBit::channelPressureEvents);
  setMap[ChannelPressureCell] =
      bind(&MIDIPortFilter::setAllChannels, &portFilter,
           ChannelFilterStatusBit::channelPressureEvents, _1);

  getMap[ProgramChangeCell] = bind(&MIDIPortFilter::allChannelsSet, &portFilter,
                                   ChannelFilterStatusBit::programChangeEvents);
  setMap[ProgramChangeCell] =
      bind(&MIDIPortFilter::setAllChannels, &portFilter,
           ChannelFilterStatusBit::programChangeEvents, _1);

  getMap[ControlChangeCell] = bind(&MIDIPortFilter::allChannelsSet, &portFilter,
                                   ChannelFilterStatusBit::controlChangeEvents);
  setMap[ControlChangeCell] =
      bind(&MIDIPortFilter::setAllChannels, &portFilter,
           ChannelFilterStatusBit::controlChangeEvents, _1);

  getMap[PolyKeyPressureCell] =
      bind(&MIDIPortFilter::allChannelsSet, &portFilter,
           ChannelFilterStatusBit::polyKeyPressureEvents);
  setMap[PolyKeyPressureCell] =
      bind(&MIDIPortFilter::setAllChannels, &portFilter,
           ChannelFilterStatusBit::polyKeyPressureEvents, _1);

  getMap[NoteCell] = bind(&MIDIPortFilter::allChannelsSet, &portFilter,
                          ChannelFilterStatusBit::noteEvents);
  setMap[NoteCell] = bind(&MIDIPortFilter::setAllChannels, &portFilter,
                          ChannelFilterStatusBit::noteEvents, _1);
  getMap[ResetCell] =
      bind(&MIDIPortFilter::FilterStatus::test, portFilter.filterStatus(),
           FilterStatusBit::resetEvents);
  setMap[ResetCell] = bind(&MIDIPortFilter::FilterStatus::set,
                           boost::ref(portFilter.filterStatus()),
                           FilterStatusBit::resetEvents, _1);

  getMap[ActiveSensingCell] = bind(&MIDIPortFilter::FilterStatus::test,
                                   boost::cref(portFilter.filterStatus()),
                                   FilterStatusBit::activeSensingEvents);
  setMap[ActiveSensingCell] = bind(&MIDIPortFilter::FilterStatus::set,
                                   boost::ref(portFilter.filterStatus()),
                                   FilterStatusBit::activeSensingEvents, _1);

  getMap[RealtimeCell] = bind(&MIDIPortFilter::FilterStatus::test,
                              boost::cref(portFilter.filterStatus()),
                              FilterStatusBit::realtimeEvents);
  setMap[RealtimeCell] = bind(&MIDIPortFilter::FilterStatus::set,
                              boost::ref(portFilter.filterStatus()),
                              FilterStatusBit::realtimeEvents, _1);

  getMap[TuneRequestCell] = bind(&MIDIPortFilter::FilterStatus::test,
                                 boost::cref(portFilter.filterStatus()),
                                 FilterStatusBit::tuneRequestEvents);
  setMap[TuneRequestCell] = bind(&MIDIPortFilter::FilterStatus::set,
                                 boost::ref(portFilter.filterStatus()),
                                 FilterStatusBit::tuneRequestEvents, _1);

  getMap[SongSelectCell] = bind(&MIDIPortFilter::FilterStatus::test,
                                boost::cref(portFilter.filterStatus()),
                                FilterStatusBit::songSelectEvents);
  setMap[SongSelectCell] = bind(&MIDIPortFilter::FilterStatus::set,
                                boost::ref(portFilter.filterStatus()),
                                FilterStatusBit::songSelectEvents, _1);

  getMap[SongPositionPointerCell] =
      bind(&MIDIPortFilter::FilterStatus::test,
           boost::cref(portFilter.filterStatus()),
           FilterStatusBit::songPositionPointerEvents);
  setMap[SongPositionPointerCell] = bind(
      &MIDIPortFilter::FilterStatus::set, boost::ref(portFilter.filterStatus()),
      FilterStatusBit::songPositionPointerEvents, _1);

  getMap[TimeCodeCell] = bind(&MIDIPortFilter::FilterStatus::test,
                              boost::cref(portFilter.filterStatus()),
                              FilterStatusBit::timeCodeEvents);
  setMap[TimeCodeCell] = bind(&MIDIPortFilter::FilterStatus::set,
                              boost::ref(portFilter.filterStatus()),
                              FilterStatusBit::timeCodeEvents, _1);

  getMap[SystemExclusiveCell] = bind(&MIDIPortFilter::FilterStatus::test,
                                     boost::cref(portFilter.filterStatus()),
                                     FilterStatusBit::systemExclusiveEvents);
  setMap[SystemExclusiveCell] = bind(
      &MIDIPortFilter::FilterStatus::set, boost::ref(portFilter.filterStatus()),
      FilterStatusBit::systemExclusiveEvents, _1);

  title = [super title];

  assert(sender.device);
  const auto &portInfo = sender.device->get<MIDIPortInfo>(portID);
  title = [title stringByAppendingString:@(portInfo.portName().c_str())];
}

- (NSString *)title {
  return title;
}

- (NSString *)providerName {
  return ((isInput) ? (@"PortFiltersInputFilterSelection")
                    : (@"PortFiltersOutputFilterSelection"));
}

- (NSUInteger)numberOfColumns {
  return 2;
}

- (NSUInteger)numberOfRows {
  return 7;
}

- (void)providerWillAppear:(ICViewController *)sender {
  for (const auto &getPair : getMap) {
    const auto &selected = !getPair.second();
    [(sender.providerButtons)[getPair.first] setSelected:selected];
  }

  if ([[UIDevice currentDevice] userInterfaceIdiom] !=
      UIUserInterfaceIdiomPhone) {
    for (int i = PitchBendCell; i <= NoteCell; ++i) {
      UIButton *const button = sender.providerButtons[i];
      UIButton *const indicatorButton =
          [UIButton buttonWithType:UIButtonTypeCustom];

      UIImage *image = [UIImage imageNamed:@"ChannelButton.png"];
      [indicatorButton setImage:image forState:UIControlStateNormal];
      [indicatorButton setFrame:CGRectMake(0, 0, 30, 30)];

      [button addSubview:indicatorButton];

      indicatorButton.center = CGPointMake(
          [button frame].size.width - indicatorButton.frame.size.width * 1.5f,
          [button frame].size.height * 0.5f);
      [indicatorButton
          setAutoresizingMask:UIViewAutoresizingFlexibleLeftMargin |
                              UIViewAutoresizingFlexibleTopMargin |
                              UIViewAutoresizingFlexibleBottomMargin];
      [indicatorButton addTarget:self
                          action:@selector(onDetailedDisclosurePressed:)
                forControlEvents:UIControlEventTouchUpInside];
      [indicatorButton setTag:i];
    }
  }
}

- (void)onDetailedDisclosurePressed:(UIButton *)button {
  NSInteger buttonIndex = [button tag];

  assert(parent);
  [parent pushToProvider:[[ICPortFiltersChannelProvider alloc]
                              initForFilterID:filterID
                                       portID:portID
                             channelStatusBit:(ChannelFilterStatusBitEnum)(
                                                  5 - buttonIndex)
                                        title:[self buttonNames][buttonIndex]]
                animated:YES];
  [self startUpdateTimer];
}

- (void)onButtonDown:(ICViewController *)sender index:(NSInteger)buttonIndex {
  if (([[UIDevice currentDevice] userInterfaceIdiom] ==
       UIUserInterfaceIdiomPhone) &&
      (buttonIndex <= NoteCell)) {
    [sender pushToProvider:[[ICPortFiltersChannelProvider alloc]
                                initForFilterID:filterID
                                         portID:portID
                               channelStatusBit:static_cast<
                                                    ChannelFilterStatusBitEnum>(
                                                    NoteCell - buttonIndex)
                                          title:[self buttonNames][buttonIndex]]
                  animated:YES];
  } else {
    if (setMap.count(buttonIndex) > 0) {
      setMap[buttonIndex]([(sender.providerButtons)[buttonIndex] isSelected]);
    }
    [(sender.providerButtons)[buttonIndex]
        setSelected:![(sender.providerButtons)[buttonIndex] isSelected]];
  }

  [self startUpdateTimer];
}

- (void)onUpdate:(DeviceID)deviceID transID:(Word)transID {
  assert(self.parent);
  assert(self.parent.device);
  const auto &portFilter = self.parent.device->midiPortFilter(portID, filterID);
  self.parent.device->send<SetMIDIPortFilterCommand>(portFilter);
}

@end
