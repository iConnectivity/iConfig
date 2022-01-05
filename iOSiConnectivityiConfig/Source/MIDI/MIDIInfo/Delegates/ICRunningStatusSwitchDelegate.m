/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <UIKit/UIKit.h>

#import "ICRunningStatusSwitchDelegate.h"
#import "MIDIInfo.h"

@implementation ICRunningStatusSwitchDelegate

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
  return ((isiPhone) ? (@"Running Status on DIN")
                     : (@"Running Status on DIN ports"));
}

- (BOOL)getValue {
  using namespace GeneSysLib;
  const MIDIInfo &midiInfo = self.device->get<MIDIInfo>();
  return (BOOL)midiInfo.isSet(GlobalMIDIFlags::RunningStatusOnDINs);
}

- (void)setValue:(BOOL)value {
  using namespace GeneSysLib;
  MIDIInfo &midiInfo = self.device->get<MIDIInfo>();
  midiInfo.setFlag(GlobalMIDIFlags::RunningStatusOnDINs, value);
  self.device->send<SetMIDIInfoCommand>(midiInfo);
}

@end
