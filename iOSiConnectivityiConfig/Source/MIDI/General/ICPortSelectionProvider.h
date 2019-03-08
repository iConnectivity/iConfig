/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICDefaultProvider.h"

@interface ICPortSelectionProvider : ICDefaultProvider {
  NSString *providerName;

  NSArray *buttonNames;
  NSArray *isStandardArray;
  NSArray *isEnabledArray;

  id<SquareViewProvider> destinationProvider;
  BOOL isInputType;
}

- (id)initWithProviderName:(NSString *)_providerName
       destinationProvider:(id<SquareViewProvider>)destination
                  forInput:(BOOL)inputType;
- (id)initForInput:(BOOL)inputType;

- (NSString *)providerName;

@end
