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
@property(nonatomic) NSInteger currentChoice;

@end

@implementation ICReservedPortChoiceDelegate

- (id)initWithDevice:(DeviceInfoPtr)device portID:(Word)portID {
  self = [super init];
  
  self.currentChoice = 0;

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

  for (const auto& option : self.device->usbHostMIDIDeviceDetails)
  {
    if ((option.numMIDIIn() > 0) || (option.numMIDIOut() > 0))
    {
      const auto maxPortID = MAX(option.numMIDIIn(), option.numMIDIOut());
      for (Word portID = 1; portID <= maxPortID; ++portID)
      {
        NSString* portName = [NSString
            stringWithFormat:@"%s %s (Port %d)", option.vendorName().c_str(),
                             option.productName().c_str(), portID];
        [reservedOptions addObject:portName];

        if ((usbHost.hostedUSBProductID() == option.hostedUSBProductID()) &&
            (usbHost.hostedUSBVendorID() == option.hostedUSBVendorID()) &&
            (usbHost.hostedDeviceMIDIPort() == portID))
        {
          reservedInList = true;
          self.currentChoice = reservedOptions.count - 1;
        }
      }
    }
  }

  if (usbHost.isReserved() && !reservedInList)
  {
    NSString* option =
        [NSString stringWithFormat:@"%s %s (Port %d) [Not attached]",
                                   usbHost.vendorName().c_str(),
                                   usbHost.productName().c_str(),
                                   usbHost.hostedDeviceMIDIPort()];
    [reservedOptions addObject:option];
    self.currentChoice = reservedOptions.count - 1;
  }

  return reservedOptions;
}

- (NSInteger)optionCount {
  return [self.options count];
}

- (NSInteger)getChoice {
  return self.currentChoice;
}

- (void)setChoice:(NSInteger)value {
  using namespace GeneSysLib;
  MIDIPortDetail& portDetail = self.device->get<MIDIPortDetail>(self.portID);
  MIDIPortDetailTypes::USBHost& usbHost = portDetail.getUSBHost();

  if (value <= 0)
  {
    // none selected
    self.currentChoice = 0;
    usbHost.notReserved();
    self.device->send<SetMIDIPortDetailCommand>(portDetail);
  }
  else
  {
    auto possiblePort = 1;  // to find the one in usbHostMIDIDeviceDetails the index needs to be corrected
              // ('none' is not in the collection)
    for (const auto& option : self.device->usbHostMIDIDeviceDetails)
    {
      if ((option.numMIDIIn() > 0) || (option.numMIDIOut() > 0))
      {
        const auto maxPortID = MAX(option.numMIDIIn(), option.numMIDIOut());
        for (Word portID = 1; portID <= maxPortID; ++portID, ++possiblePort)
        {
          if (possiblePort == value)
          {
            usbHost.reserved(option, portID);
            self.device->send<SetMIDIPortDetailCommand>(portDetail);
            break;
          }
        }
      }
    }
  }
}

@end
