/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICDefaultProvider.h"
#include "MIDIPortRoute.h"

@interface ICPortRoutingProvider : ICDefaultProvider {
  GeneSysLib::MIDIPortRoute portRoute;

  Word selectedPort;

  std::map<int, boost::function<void(BOOL)> > setMap;
  std::map<int, boost::function<BOOL(void)> > getMap;

  NSArray *buttonNames;
  NSString *title;

  NSArray *isStandardArray;
  NSArray *isEnabledArray;
}

- (id)initWithSelectedPort:(Word)selectedPort;

@end
