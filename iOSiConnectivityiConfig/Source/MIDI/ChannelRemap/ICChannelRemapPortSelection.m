/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICChannelRemapPortSelection.h"
#import "ICChannelRemapSelectionProvider.h"
#import "ICRunOnMain.h"

using namespace GeneSysLib;

@implementation ICChannelRemapPortSelection

- (id)initForInput:(BOOL)inputType {
  self = [super initForInput:inputType];

  if (self) {
    providerName = (inputType) ? (@"ChannelRemapInputPortSelection")
                               : (@"ChannelRemapOutputPortSelection");
    handlerID = -1;
  }

  return self;
}

- (void)onButtonDown:(ICViewController *)sender index:(NSInteger)buttonIndex {
  if (handlerID >= 0) {
    sender.comm->unRegisterHandler(Command::RetMIDIPortRemap, handlerID);
    handlerID = -1;
  }

  handlerID = sender.comm->registerHandler(
      Command::RetMIDIPortRemap, [=](CmdEnum, DeviceID, Word, commandData_t) {
        runOnMain(^{
            auto *const provider = [[ICChannelRemapSelectionProvider alloc]
                initForInput:isInputType
                      portID:(Word)(buttonIndex + 1)];
            [sender pushToProvider:provider animated:YES];
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
