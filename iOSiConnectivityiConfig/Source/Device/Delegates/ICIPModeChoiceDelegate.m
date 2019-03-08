/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "EthernetPortInfo.h"
#import "ICIPModeChoiceDelegate.h"

@interface ICIPModeChoiceDelegate ()
@property(nonatomic) Word portID;

@end

@implementation ICIPModeChoiceDelegate

- (id)initWithDevice:(DeviceInfoPtr)device portID:(Word)portID {
  self = [super init];

  if (self) {
    self.device = device;
    self.portID = portID;
    self.options = @[ @"Static", @"Dynamic" ];
  }

  return self;
}

- (NSString *)title {
  return @"IP Mode";
}

- (NSInteger)optionCount {
  return [self.options count];
}

- (NSInteger)getChoice {
  using namespace GeneSysLib;
  const EthernetPortInfo &ethPort =
      self.device->get<EthernetPortInfo>(self.portID);
  return (NSInteger)ethPort.ipMode();
}

- (void)setChoice:(NSInteger)value {
  using namespace GeneSysLib;
  EthernetPortInfo &ethPort = self.device->get<EthernetPortInfo>(self.portID);
  ethPort.ipMode(static_cast<IPModeEnum>(value));
  self.device->send<SetEthernetPortInfoCommand>(ethPort);
}

@end
