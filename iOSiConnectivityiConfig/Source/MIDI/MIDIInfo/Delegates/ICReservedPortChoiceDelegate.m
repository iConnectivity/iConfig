/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICReservedPortChoiceDelegate.h"
#import "MIDIPortDetail.h"

@interface ICReservedPortChoiceDelegate ()

@property(nonatomic) DeviceInfoPtr device;
@property(nonatomic) Word portID;

@end

@implementation ICReservedPortChoiceDelegate

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

- (NSString*)title {
  return @"Reserved";
}

- (NSArray*)options {
  using namespace GeneSysLib;
  MIDIPortDetail portDetail = self.device->get<MIDIPortDetail>(self.portID);
  MIDIPortDetailTypes::USBHost usbHost = portDetail.getUSBHost();

  NSMutableArray* reservedOptions = [NSMutableArray array];
  [reservedOptions addObject:@"None"];

  bool reservedInList = false;

  for (const auto& option : self.device->usbHostMIDIDeviceDetails) {
    if ((option.numMIDIIn() > 0) || (option.numMIDIOut() > 0)) {

      const auto maxPortID = MAX(option.numMIDIIn(), option.numMIDIOut());
      for (Word portID = 1; portID <= maxPortID; ++portID) {
        NSString* portName = [NSString
            stringWithFormat:@"%s %s (Port %d)", option.vendorName().c_str(),
                             option.productName().c_str(), portID];
        [reservedOptions addObject:portName];

        if ((usbHost.hostedUSBProductID() == option.hostedUSBProductID()) &&
            (usbHost.hostedUSBVendorID() == option.hostedUSBVendorID())) {
          reservedInList = true;
        }
      }
    }
  }

  if (usbHost.isReserved() && !reservedInList) {
    NSString* option =
        [NSString stringWithFormat:@"%s %s (Port %d) [Not attached]",
                                   usbHost.vendorName().c_str(),
                                   usbHost.productName().c_str(),
                                   usbHost.hostedDeviceMIDIPort()];
    [reservedOptions addObject:option];
  }

  return reservedOptions;
}

- (NSInteger)optionCount {
  return [self.options count];
}

- (NSInteger)getChoice {
  using namespace GeneSysLib;
  MIDIPortDetail portDetail = self.device->get<MIDIPortDetail>(self.portID);
  MIDIPortDetailTypes::USBHost usbHost = portDetail.getUSBHost();

  NSInteger value = 0;

    if (usbHost.isReserved()) {
      ++value;
    }

  for (const auto& option : self.device->usbHostMIDIDeviceDetails) {
    if ((option.hostedUSBVendorID() == usbHost.hostedUSBVendorID()) &&
        (option.hostedUSBProductID() == usbHost.hostedUSBProductID())) {
      value += usbHost.hostedDeviceMIDIPort();
    } else {
      value += MAX(option.numMIDIIn(), option.numMIDIOut());
    }
  }

  return value;
}

- (void)setChoice:(NSInteger)value {
  using namespace GeneSysLib;
  MIDIPortDetail& portDetail = self.device->get<MIDIPortDetail>(self.portID);
  MIDIPortDetailTypes::USBHost& usbHost = portDetail.getUSBHost();

  if (value <= 0) {
    usbHost.notReserved();
    self.device->send<SetMIDIPortDetailCommand>(portDetail);
  } else {
    auto option = self.device->usbHostMIDIDeviceDetails.begin();
    Word portID = 0;
    for (;
         (option != self.device->usbHostMIDIDeviceDetails.end()) && (value > 0);
         ++option) {
      portID = MIN(value, MAX(option->numMIDIIn(), option->numMIDIOut()));
      value -= MIN(value, MAX(option->numMIDIIn(), option->numMIDIOut()));
    }
    if (option != self.device->usbHostMIDIDeviceDetails.end()) {
      // reserve selected port
      usbHost.reserved(*option, portID);

      self.device->send<SetMIDIPortDetailCommand>(portDetail);
    } else {
      usbHost.notReserved();
      self.device->send<SetMIDIPortDetailCommand>(portDetail);
    }
  }
}

@end
