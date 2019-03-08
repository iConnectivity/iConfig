/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICMIDIInterPortRoutingSwitchDelegate.h"
#import "MIDIInfo.h"

@implementation ICMIDIInterPortRoutingSwitchDelegate

- (id)initWithDevice:(DeviceInfoPtr)device {
  self = [super init];

  if (self) {
    NSParameterAssert(device);
    self.device = device;
  }

  return self;
}

- (NSString *)title {
  BOOL isiPhone = ([[UIDevice currentDevice] userInterfaceIdiom] ==
                   UIUserInterfaceIdiomPhone);
  return ((isiPhone) ? (@"Routing between ports")
                     : (@"Routing between ports on Multiport USB Devices"));
}

- (BOOL)getValue {
  using namespace GeneSysLib;
  const MIDIInfo &midiInfo = self.device->get<MIDIInfo>();
  return (BOOL)midiInfo.isSet(GlobalMIDIFlags::RoutingBetweenMultiPort);
}

- (void)setValue:(BOOL)value {
  using namespace GeneSysLib;
  MIDIInfo &midiInfo = self.device->get<MIDIInfo>();
  midiInfo.setFlag(GlobalMIDIFlags::RoutingBetweenMultiPort, value);
  self.device->send<SetMIDIInfoCommand>(midiInfo);
}

@end
