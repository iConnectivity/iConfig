/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICIPAddressTextEditCellDelegate.h"
#import "EthernetPortInfo.h"
#import "MyConverters.h"

using namespace GeneSysLib;
using namespace NetAddrTools;

@interface ICIPAddressTextEditCellDelegate ()

@property(nonatomic, assign) DeviceInfoPtr device;
@property(nonatomic, assign) Word portID;

@end

@implementation ICIPAddressTextEditCellDelegate

- (id)initWithDevice:(DeviceInfoPtr)device portID:(Word)portID {
  self = [super init];

  if (self) {
    NSParameterAssert(device);
    NSParameterAssert(portID >= 1);
    self.device = device;
    self.portID = portID;
  }

  return self;
}

- (NSString *)title {
  return @"Static IP";
}

- (NSString *)getValue {
  assert(self.device->contains<EthernetPortInfo>(self.portID));
  const EthernetPortInfo &ethPortInfo =
      self.device->get<EthernetPortInfo>(self.portID);

  return fromNetAddr(ethPortInfo.staticIPAddress());
}

- (void)setValue:(NSString *)value {
  EthernetPortInfo &ethPortInfo =
      self.device->get<EthernetPortInfo>(self.portID);

  ethPortInfo.staticIPAddress(toNetAddr(value));

  self.device->send<SetEthernetPortInfoCommand>(ethPortInfo);
}

- (NSInteger)maxLength {
  return 15;
}

- (NSString *)charSet {
  return @"0123456789.";
}

- (NSString *)validator {
  return ipRegEx();
}

@end
