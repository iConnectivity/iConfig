/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICPortRoutingPortSelectionProvider.h"
#import "ICPortRoutingProvider.h"
#import "ICRunOnMain.h"

using namespace GeneSysLib;

@implementation ICPortRoutingPortSelectionProvider

- (id)init {
  self = [super initForInput:YES];
  if (self) {
    providerName = @"PortRoutingPortSelection";
    handlerID = -1;
  }
  return self;
}

- (void)onButtonDown:(ICViewController *)sender index:(NSInteger)buttonIndex {
  if (handlerID >= 0) {
    sender.comm->unRegisterHandler(Command::RetMIDIPortRoute, handlerID);
    handlerID = -1;
  }

  handlerID = sender.comm->registerHandler(Command::RetMIDIPortRoute,
                                           [=](CmdEnum, DeviceID, Word,
                                               commandData_t) {
    runOnMain(^{
        [sender pushToProvider:[[ICPortRoutingProvider alloc]
                                   initWithSelectedPort:(Word)(buttonIndex + 1)]
                      animated:YES];
    });
  });
  sender.device->send<GetMIDIPortRouteCommand>((Word)(buttonIndex + 1));
}

- (void)providerWillDissappear:(ICViewController *)sender {
  if (handlerID >= 0) {
    sender.comm->unRegisterHandler(Command::RetMIDIPortRoute, handlerID);
    handlerID = -1;
  }
}

@end
