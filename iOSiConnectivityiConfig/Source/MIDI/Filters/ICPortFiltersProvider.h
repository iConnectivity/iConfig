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

@interface ICPortFiltersProvider : ICDefaultProvider {
  std::map<int, boost::function<void(BOOL)> > setMap;
  std::map<int, boost::function<BOOL(void)> > getMap;

  BOOL isInput;
  GeneSysLib::FilterIDEnum filterID;
  Word portID;

  NSString *title;

  ICViewController *parent;
}

- (id)initWithInputType:(BOOL)inputType withPortID:(Word)portID;
- (void)onDetailedDisclosurePressed:(UIButton *)button;

@end
