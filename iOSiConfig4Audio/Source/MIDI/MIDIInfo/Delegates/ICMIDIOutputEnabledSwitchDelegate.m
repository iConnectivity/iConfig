/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICMIDIOutputEnabledSwitchDelegate.h"

#import "MIDIInfo.h"
#import "MIDIPortInfo.h"

using namespace GeneSysLib;

@implementation ICMIDIOutputEnabledSwitchDelegate

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
  return @"Output Enabled";
}

- (BOOL)getValue {
  const MIDIPortInfo& port = self.device->get<MIDIPortInfo>(self.portID);
  return port.isOutputEnabled();
}

- (void)setValue:(BOOL)value {
  MIDIPortInfo& port = self.device->get<MIDIPortInfo>(self.portID);
  port.setOutputEnabled(value);
  self.device->send<SetMIDIPortInfoCommand>(port);
}

@end
