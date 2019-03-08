/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICPCEnabledV2SwitchDelegate.h"
#import "AudioPortParm.h"

using namespace GeneSysLib;

@implementation ICPCEnabledV2SwitchDelegate

- (id)initWithDevice:(DeviceInfoPtr)device portID:(Word)portID {
  self = [super init];

  if (self) {
    NSParameterAssert(device);
    NSParameterAssert(portID);

    self.device = device;
    self.portID = portID;
  }

  return self;
}

- (NSString *)title {
  return @"PC/Mac Audio Enabled";
}

- (BOOL)getValue {
  const AudioPortParm &audioPortParm =
      self.device->get<AudioPortParm>(self.portID);
  const auto &usbDevice = audioPortParm.usbDevice();
  return usbDevice.isPCAudioEnabled();
}

- (void)setValue:(BOOL)value {
  AudioPortParm &audioPortParm = self.device->get<AudioPortParm>(self.portID);
  auto &usbDevice = audioPortParm.usbDevice();

  usbDevice.setPCAudioEnabled(value);
}

@end
