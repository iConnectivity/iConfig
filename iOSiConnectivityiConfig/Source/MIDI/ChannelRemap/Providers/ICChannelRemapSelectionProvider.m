/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICChannelRemapSelectionProvider.h"
#import "ICChannelRemapProvider.h"
#include "MIDIPortInfo.h"

using namespace GeneSysLib;

@implementation ICChannelRemapSelectionProvider

- (id)initForInput:(BOOL)inputType portID:(Word)_portID {
  self = [super init];

  if (self) {
    isInput = inputType;
    portID = _portID;
  }

  return self;
}

- (void)initializeProviderButtons:(ICViewController *)sender {
  [super initializeProviderButtons:sender];
  title = [super title];

  if (sender.device) {
    const auto &portInfo = sender.device->get<MIDIPortInfo>(portID);
    title = @(portInfo.portName().c_str());
  }
}

- (NSString *)title {
  return title;
}

- (NSString *)providerName {
  return ((isInput) ? (@"ChannelRemapInputChannelSelection")
                    : (@"ChannelRemapOutputChannelSelection"));
}

- (void)onButtonDown:(ICViewController *)sender index:(NSInteger)buttonIndex {
  [sender
      pushToProvider:[[ICChannelRemapProvider alloc] initForInput:isInput
                                                            portID:portID
                                                        forChannel:buttonIndex]
            animated:TRUE];
}

@end
