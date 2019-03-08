/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICiOSEnabledV1SwitchDelegate.h"
#import "AudioPortInfo.h"

using namespace GeneSysLib;

@implementation ICiOSEnabledV1SwitchDelegate

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
  return @"iOS Audio Enabled";
}

- (BOOL)getValue {
  AudioPortInfo &audioPortInfo = self.device->get<AudioPortInfo>(self.portID);
  return (BOOL)audioPortInfo.isIOSAudioEnabled();
}

- (void)setValue:(BOOL)value {
  AudioPortInfo &audioPortInfo = self.device->get<AudioPortInfo>(self.portID);
  audioPortInfo.setIOSAudioEnabled(value);
  self.device->send<SetAudioPortInfoCommand>(audioPortInfo);
}

@end
