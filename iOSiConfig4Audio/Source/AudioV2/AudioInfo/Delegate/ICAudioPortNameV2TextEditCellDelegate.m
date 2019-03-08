/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICAudioPortNameV2TextEditCellDelegate.h"
#import "AudioPortParm.h"
#import <string>

using namespace GeneSysLib;
using namespace std;

@interface ICAudioPortNameV2TextEditCellDelegate ()

@property(nonatomic, assign) DeviceInfoPtr device;
@property(nonatomic, assign) Word portID;

@end

@implementation ICAudioPortNameV2TextEditCellDelegate

- (id)initWithDevice:(DeviceInfoPtr)device portID:(Word)portID {
  self = [super init];

  if (self) {
    NSParameterAssert(device);
    self.device = device;
    self.portID = portID;
  }

  return self;
}

- (NSString *)title {
  return @"Name";
}

- (NSString *)getValue {
  const AudioPortParm &audioPortParm =
      self.device->get<AudioPortParm>(self.portID);
  return ((audioPortParm.portName().size() > 0)
              ? @(audioPortParm.portName().c_str())
              : @"");
}

- (void)setValue:(NSString *)value {
  AudioPortParm &audioPortParm = self.device->get<AudioPortParm>(self.portID);

  audioPortParm.portName(string(
      [value cStringUsingEncoding:NSASCIIStringEncoding], [value length]));

  self.device->send<SetAudioPortParmCommand>(audioPortParm);
}

- (NSInteger)maxLength {
  AudioPortParm &audioPortParm = self.device->get<AudioPortParm>(self.portID);
  return (NSInteger)audioPortParm.maxPortName();
}

@end
