/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICFilterPortSelection.h"
#import "ICPortFiltersProvider.h"
#import "ICRunOnMain.h"

using namespace GeneSysLib;

@implementation ICFilterPortSelection

- (id)initForInput:(BOOL)inputType {
  self = [super initForInput:inputType];
  if (self) {
    providerName = (inputType) ? (@"PortFiltersInputPortSelection")
                               : (@"PortFiltersOutputPortSelection");
    handlerID = -1;
  }
  return self;
}

- (void)onButtonDown:(ICViewController *)sender index:(NSInteger)buttonIndex {
  NSParameterAssert(sender);
  if (handlerID >= 0) {
    sender.comm->unRegisterHandler(Command::RetMIDIPortFilter, handlerID);
    handlerID = -1;
  }

  handlerID = sender.comm->registerHandler(Command::RetMIDIPortFilter,
                                           [=](CmdEnum, DeviceID, Word,
                                               commandData_t) {
    runOnMain(^{
        [sender pushToProvider:[[ICPortFiltersProvider alloc]
                                   initWithInputType:isInputType
                                          withPortID:(Word)(buttonIndex + 1)]
                      animated:YES];
    });
  });
  const auto &filterID =
      ((isInputType) ? (FilterID::InputFilter) : (FilterID::OutputFilter));
  sender.device->send<GetMIDIPortFilterCommand>((Word)(buttonIndex + 1),
                                                filterID);
}

- (void)providerWillDissappear:(ICViewController *)sender {
  NSParameterAssert(sender);
  if (handlerID >= 0) {
    sender.comm->unRegisterHandler(Command::RetMIDIPortFilter, handlerID);
    handlerID = -1;
  }
}

@end
