/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICAudioPortNameV1TextEditCellDelegate.h"
#import "AudioPortInfo.h"
#import <string>

using namespace GeneSysLib;
using namespace std;

@interface ICAudioPortNameV1TextEditCellDelegate ()

@property(nonatomic, assign) DeviceInfoPtr device;
@property(nonatomic, assign) Word portID;

@end

@implementation ICAudioPortNameV1TextEditCellDelegate

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
  const AudioPortInfo &audioPortInfo =
      self.device->get<AudioPortInfo>(self.portID);
  return ((audioPortInfo.portName().size() > 0)
              ? @(audioPortInfo.portName().c_str())
              : @"");
}

- (void)setValue:(NSString *)value {
  AudioPortInfo &audioPortInfo = self.device->get<AudioPortInfo>(self.portID);

  audioPortInfo.portName(string(
      [value cStringUsingEncoding:NSASCIIStringEncoding], [value length]));

  self.device->send<SetAudioPortInfoCommand>(audioPortInfo);
}

- (NSInteger)maxLength {
  const AudioPortInfo &audioPortInfo =
      self.device->get<AudioPortInfo>(self.portID);
  return audioPortInfo.maxPortName();
}

@end
