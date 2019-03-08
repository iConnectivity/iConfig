/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICControllerFilterIDProvider.h"
#import "ICControllerFilterPortSelectionProvider.h"
#import "ICRunOnMain.h"

using namespace GeneSysLib;

@implementation ICControllerFilterPortSelectionProvider

- (id)initForInput:(BOOL)inputType {
  self = [super initForInput:inputType];
  if (self) {
    providerName = (inputType) ? (@"ControllerFiltersInputPortSelection")
                               : (@"ControllerFiltersOutputPortSelection");
    handlerID = -1;
    isInputType = inputType;
  }
  return self;
}

- (void)onButtonDown:(ICViewController *)sender index:(NSInteger)buttonIndex {
  if (handlerID >= 0) {
    sender.comm->unRegisterHandler(Command::RetMIDIPortFilter, handlerID);
    handlerID = -1;
  }

  // TPK - should remove reference to comm and use the method in deviceInfo
  handlerID = sender.comm->registerHandler(
      Command::RetMIDIPortFilter, [=](CmdEnum, DeviceID, Word, commandData_t) {
        runOnMain(^{
            [sender pushToProvider:[[ICControllerFilterIDProvider alloc]
                                       initForInput:isInputType
                                             portID:(buttonIndex + 1)]
                          animated:YES];
        });
      });

  const auto &filterID =
      ((isInputType) ? (FilterID::InputFilter) : (FilterID::OutputFilter));
  sender.device->send<GetMIDIPortFilterCommand>((Word)(buttonIndex + 1),
                                                filterID);
}

- (void)providerWillDissappear:(ICViewController *)sender {
  if (handlerID >= 0) {
    sender.comm->unRegisterHandler(Command::RetMIDIPortFilter, handlerID);
    handlerID = -1;
  }
}

@end
