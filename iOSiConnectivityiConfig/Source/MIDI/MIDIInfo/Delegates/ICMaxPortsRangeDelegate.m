/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICMaxPortsRangeDelegate.h"
#import "MIDIInfo.h"

using namespace GeneSysLib;

@implementation ICMaxPortsRangeDelegate

- (id)initWithDevice:(DeviceInfoPtr)device {
  self = [self init];

  if (self) {
    NSParameterAssert(device);
    self.device = device;
  }

  return self;
}

- (NSString *)title {
  const auto &isiPhone = ([[UIDevice currentDevice] userInterfaceIdiom] ==
                          UIUserInterfaceIdiomPhone);
  return ((isiPhone) ? (@"Ports / Multiport Device")
                     : (@"Maximum Ports on " @"Multiport USB Devices"));
}

- (NSInteger)getValue {
  const auto &midiInfo = self.device->get<MIDIInfo>();
  return midiInfo.maxPortsOnMultiPortUSBDevice();
}

- (void)setValue:(NSInteger)value {
  auto &midiInfo = self.device->get<MIDIInfo>();
  midiInfo.maxPortsOnMultiPortUSBDevice((Byte)(value & 0x7F));
  self.device->send<SetMIDIInfoCommand>(midiInfo);
}

- (NSInteger)getMin {
  return 1;
}

- (NSInteger)getMax {
  const auto &midiInfo = self.device->get<MIDIInfo>();
  return (NSInteger)midiInfo.numUSBMIDIPortPerHostJack();
}

- (NSInteger)getStride {
  return 1;
}

@end
