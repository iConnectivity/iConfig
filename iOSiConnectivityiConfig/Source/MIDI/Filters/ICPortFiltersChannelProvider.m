/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICPortFiltersChannelProvider.h"

#include "MyAlgorithms.h"
#include <boost/bind.hpp>

using namespace GeneSysLib;
using namespace MyAlgorithms;

@implementation ICPortFiltersChannelProvider

- (id)initForFilterID:(FilterIDEnum)_filterID
               portID:(Word)_portID
     channelStatusBit:(ChannelFilterStatusBitEnum)_channelFilter
                title:(NSString *)_title {
  self = [super init];
  if (self) {
    NSParameterAssert(_title);
    filterID = _filterID;
    portID = _portID;
    channelFilter = _channelFilter;
    title = _title;
  }

  return self;
}

- (NSString *)title {
  return title;
}

- (NSString *)providerName {
  return @"PortFiltersChannelFilterSelection";
}

- (void)providerWillAppear:(ICViewController *)sender {
  self.parent = sender;
  auto &portFilter = sender.device->midiPortFilter(portID, filterID);

  for (size_t i = 0; i < 16; ++i) {
    setMap[i] = bind(&MIDIPortFilter::ChannelFilterStatus::set,
                     boost::ref(portFilter.channelFilterStatus_at(i)),
                     channelFilter, _1);
    getMap[i] =
        bind(&MIDIPortFilter::ChannelFilterStatus::test,
             boost::ref(portFilter.channelFilterStatus_at(i)), channelFilter);
  }
  for (const auto &getPair : getMap) {
    const auto &selected = !getPair.second();
    [(sender.providerButtons)[getPair.first] setSelected:selected];
  }
}

- (void)onButtonDown:(ICViewController *)sender index:(NSInteger)buttonIndex {
  const auto &selected = ![(sender.providerButtons)[buttonIndex] isSelected];
  [(sender.providerButtons)[buttonIndex] setSelected:selected];

  if (setMap.count(buttonIndex) > 0) {
    setMap[buttonIndex](!selected);
  }

  [self startUpdateTimer];
}

- (void)onUpdate:(DeviceID)deviceID transID:(Word)transID {
  assert(self.parent);
  assert(self.parent.device);
  auto &portFilter = self.parent.device->midiPortFilter(portID, filterID);
  self.parent.device->send<SetMIDIPortFilterCommand>(portFilter);
}

@end
