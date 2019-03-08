/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICControllerRemapIDProvider.h"
#import "ICControllerRemapProvider.h"
#import "MyAlgorithms.h"
#include "MIDIPortInfo.h"

using namespace MyAlgorithms;
using namespace GeneSysLib;

@implementation ICControllerRemapIDProvider

- (id)initForInput:(BOOL)inputType portID:(Word)_portID {
  self = [super init];

  if (self) {
    isInput = inputType;
    portID = _portID;
  }

  return self;
}

- (NSString *)providerName {
  return ((isInput) ? (@"ControllerRemapInputIDSelection")
                    : (@"ControllerRemapOutputIDSelection"));
}

- (void)initializeProviderButtons:(ICViewController *)sender {
  [super initializeProviderButtons:sender];

  const auto &remapType =
      ((isInput) ? (RemapID::InputRemap) : (RemapID::OutputRemap));

  NSParameterAssert(sender);
  assert(sender.device);
  assert(sender.device->contains<MIDIPortRemap>(portID, remapType));

  auto *const tempButtons = [NSMutableArray array];

  auto portRemap = sender.device->midiPortRemap(portID, remapType);
  for (int i = 1; i <= (int)portRemap.maxControllerSupported(); ++i) {
    [tempButtons
        addObject:[NSString stringWithFormat:@"Controller Remap %d", i]];
  }

  buttonNames = tempButtons;

  title = [super title];

  const auto &portInfo = sender.device->get<MIDIPortInfo>(portID);
  title = [title stringByAppendingString:@(portInfo.portName().c_str())];
}

- (NSString *)title {
  return title;
}

- (NSArray *)buttonNames {
  return buttonNames;
}

- (void)onButtonDown:(ICViewController *)sender index:(NSInteger)buttonIndex {
  [sender pushToProvider:[[ICControllerRemapProvider alloc]
                             initForInput:isInput
                                   portID:portID
                              withRemapID:buttonIndex]
                animated:YES];
}

@end
