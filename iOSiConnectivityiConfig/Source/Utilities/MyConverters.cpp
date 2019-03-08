/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "MyConverters.h"

namespace NetAddrTools {

NSString* fromNetAddr(const NetAddr& netAddr) {
  return [NSString stringWithFormat:@"%d.%d.%d.%d", netAddr[0], netAddr[1],
                                    netAddr[2], netAddr[3]];
}

NetAddr toNetAddr(NSString* value) {
  NetAddr addr;
  NSArray* const components = [value componentsSeparatedByString:@"."];
  int i = 0;
  for (i = 0; i < [components count]; ++i) {
    addr[i] = [components[i] integerValue] & 0xFF;
  }
  for (int j = i; j < 4; ++j) {
    addr[j] = 0;
  }
  return addr;
};

NSString* ipRegEx() {
  return @"^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\\.(25[0-5]|2[0-4][0-9]|[01]"
          "?[0-9][0-9]?|$)){0,3}$";
}

}  // namespace NetAddrTools
