/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICPortSelectionProvider.h"

@interface ICChannelRemapPortSelection : ICPortSelectionProvider {
  long handlerID;
}

- (id)initForInput:(BOOL)inputType;

@end
