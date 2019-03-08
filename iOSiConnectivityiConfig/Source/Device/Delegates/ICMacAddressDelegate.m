/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICMacAddressDelegate.h"
#import "EthernetPortInfo.h"
#import <string>

@implementation ICMacAddressDelegate
- (id)initWithDevice:(DeviceInfoPtr)device portID:(Word)_portID {
  self = [super init];
  if (self) {
    NSParameterAssert(device);
    NSParameterAssert(_portID >= 1);
    portID = _portID;
    self.device = device;
  }

  return self;
}

- (NSString *)title {
  return @"MAC Address";
}
- (NSString *)value {
  using namespace std;
  using namespace GeneSysLib;
  EthernetPortInfo &ethPort = self.device->get<EthernetPortInfo>(portID);
  string macAddress = ethPort.macAddress();

  return [NSString stringWithCString:macAddress.c_str()
                            encoding:NSASCIIStringEncoding];
}

@end
