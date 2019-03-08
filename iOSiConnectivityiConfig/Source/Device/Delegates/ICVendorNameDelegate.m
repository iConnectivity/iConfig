/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICVendorNameDelegate.h"
#import "MIDIPortDetail.h"

@implementation ICVendorNameDelegate

- (id)initWithDevice:(DeviceInfoPtr)device portID:(Word)_portID {
  self = [super init];

  if (self) {
    NSParameterAssert(device);
    NSParameterAssert(_portID >= 1);
    self.device = device;
    portID = _portID;
  }

  return self;
}

- (NSString *)title {
  return @"Vendor Name";
}

- (NSString *)value {
  using namespace std;
  using namespace GeneSysLib;
  MIDIPortDetail midiDetail = self.device->get<MIDIPortDetail>(portID);

  const auto &usbHost = midiDetail.getUSBHost();
  NSString *result = @"None";

  if (usbHost.vendorName().size() > 0) {
    result = @(usbHost.vendorName().c_str());
  } else if (usbHost.hostedUSBVendorID() != 0) {
    result =
        [NSString stringWithFormat:@"ID:%04X", usbHost.hostedUSBVendorID()];
  }

  return result;
}

@end
