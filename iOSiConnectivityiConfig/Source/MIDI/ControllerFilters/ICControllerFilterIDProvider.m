/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICControllerFilterIDProvider.h"
#import "ICControllerFilterProvider.h"
#import "MyAlgorithms.h"
#import "MIDIPortInfo.h"

using namespace MyAlgorithms;
using namespace GeneSysLib;

@implementation ICControllerFilterIDProvider

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
  const auto &filterType =
      ((isInput) ? (FilterID::InputFilter) : (FilterID::OutputFilter));

  NSParameterAssert(sender);
  assert(sender.device);

  auto *const tempButtons = [NSMutableArray array];

  auto controllerFilters =
      sender.device->midiPortFilter(portID, filterType).controllerFilters();
  for (int i = 1; i <= controllerFilters.size(); ++i) {
    [tempButtons
        addObject:[NSString stringWithFormat:@"Controller Filter %d", i]];
  }
  buttonNames = tempButtons;

  title = [super title];

  const auto &portInfo = sender.device->get<MIDIPortInfo>(portID);
  title = [title stringByAppendingString:@(portInfo.portName().c_str())];
}

- (NSString *)title {
  return title;
}

- (NSString *)providerName {
  return ((isInput) ? (@"ControllerFiltersInputIDSelection")
                    : (@"ControllerFiltersOutputIDSelection"));
}

- (NSArray *)buttonNames {
  return buttonNames;
}

- (void)onButtonDown:(ICViewController *)sender index:(NSInteger)buttonIndex {
  auto *const filterProvider =
      [[ICControllerFilterProvider alloc] initWithDevice:sender.device
                                                 forInput:isInput
                                                   portID:portID
                                              forFilterID:buttonIndex];
  [sender pushToProvider:filterProvider animated:TRUE];
}

@end
