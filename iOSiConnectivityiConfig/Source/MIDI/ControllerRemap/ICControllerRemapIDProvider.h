/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICDefaultProvider.h"

@interface ICControllerRemapIDProvider : ICDefaultProvider {
  BOOL isInput;
  NSArray *buttonNames;
  NSString *title;
  Word portID;
}

- (id)initForInput:(BOOL)inputType portID:(Word)portID;

@end
