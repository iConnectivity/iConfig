/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICDefaultProvider.h"
#import "MIDIPortRemap.h"
#import "RemapID.h"

#include <boost/function.hpp>
#include <map>

@interface ICChannelRemapProvider : ICDefaultProvider {
  BOOL isInput;
  int inChannel;
  Word portID;
  GeneSysLib::RemapTypeEnum remapID;

  NSString *title;
}

- (id)initForInput:(BOOL)inputType
            portID:(Word)portID
        forChannel:(NSInteger)channel;

@end
