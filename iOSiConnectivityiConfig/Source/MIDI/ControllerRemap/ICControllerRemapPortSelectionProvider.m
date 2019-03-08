/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICControllerRemapPortSelectionProvider.h"
#import "ICControllerRemapIDProvider.h"
#import "ICRunOnMain.h"

using namespace GeneSysLib;

@implementation ICControllerRemapPortSelectionProvider

- (id)initForInput:(BOOL)inputType {
  self = [super initForInput:inputType];

  if (self) {
    isInputType = inputType;
    providerName = (inputType) ? (@"ControllerRemapInputPortSelection")
                               : (@"ControllerRemapOutputPortSelection");
    handlerID = -1;
  }

  return self;
}

- (void)onButtonDown:(ICViewController *)sender index:(NSInteger)buttonIndex {
  NSParameterAssert(sender);
  if (handlerID >= 0) {
    sender.comm->unRegisterHandler(Command::RetMIDIPortRemap, handlerID);
    handlerID = -1;
  }

  handlerID = sender.comm->registerHandler(
      Command::RetMIDIPortRemap, [=](CmdEnum, DeviceID, Word, commandData_t) {
        runOnMain(^{
            [sender pushToProvider:[[ICControllerRemapIDProvider alloc]
                                       initForInput:isInputType
                                             portID:(Word)(buttonIndex + 1)]
                          animated:YES];
        });
      });

  const auto &remapID =
      ((isInputType) ? (RemapID::InputRemap) : (RemapID::OutputRemap));
  sender.device->send<GetMIDIPortRemapCommand>((Word)(buttonIndex + 1),
                                               remapID);
}

- (void)providerWillDissappear:(ICViewController *)sender {
  if (handlerID >= 0) {
    sender.comm->unRegisterHandler(Command::RetMIDIPortRemap, handlerID);
    handlerID = -1;
  }
}
@end
