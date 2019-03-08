/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICDefaultProvider.h"

#include "MIDIPortFilter.h"

#include <boost/function.hpp>
#include <map>

@interface ICPortFiltersChannelProvider : ICDefaultProvider {
  std::map<int, boost::function<void(BOOL)> > setMap;
  std::map<int, boost::function<BOOL(void)> > getMap;

  GeneSysLib::FilterIDEnum filterID;
  Word portID;
  GeneSysLib::ChannelFilterStatusBitEnum channelFilter;

  NSString *title;
}

- (id)initForFilterID:(GeneSysLib::FilterIDEnum)filterID
               portID:(Word)portID
     channelStatusBit:(GeneSysLib::ChannelFilterStatusBitEnum)channelFilter
                title:(NSString *)title;

@end
