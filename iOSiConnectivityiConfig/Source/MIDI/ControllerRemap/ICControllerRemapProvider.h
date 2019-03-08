/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICDefaultProvider.h"
#include "MIDIPortRemap.h"

#include <boost/function.hpp>
#include <map>

@interface ICControllerRemapProvider : ICDefaultProvider {
  BOOL isInput;
  Word portID;
  int remapID;
  GeneSysLib::RemapTypeEnum remapType;
  NSArray *buttonNames;

  std::map<int, boost::function<void(BOOL)> > setMap;
  std::map<int, boost::function<BOOL(void)> > getMap;

  NSString *title;
}

- (id)initForInput:(BOOL)inputType portID:(Word)portID withRemapID:(int)remapID;

@end
