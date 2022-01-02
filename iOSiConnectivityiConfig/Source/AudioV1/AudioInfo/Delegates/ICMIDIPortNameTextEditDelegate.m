/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <UIKit/UIKit.h>

#import "ICMIDIPortNameTextEditDelegate.h"
#import "MIDIPortInfo.h"
#import <string>

using namespace std;

@interface ICMIDIPortNameTextEditDelegate ()
@property(nonatomic, assign) DeviceInfoPtr device;
@property(nonatomic, assign) Word portID;
@end

@implementation ICMIDIPortNameTextEditDelegate

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

- (NSString *)title {
  bool isiPhone = ([[UIDevice currentDevice] userInterfaceIdiom] ==
                   UIUserInterfaceIdiomPhone);
  return ((isiPhone) ? (@"Name") : (@"Port Name"));
}

- (NSString *)getValue {
  const MIDIPortInfo &portInfo = self.device->get<MIDIPortInfo>(self.portID);
  return @(portInfo.portName().c_str());
}

- (void)setValue:(NSString *)value {
  MIDIPortInfo &portInfo = self.device->get<MIDIPortInfo>(self.portID);
  portInfo.portName(string([value cStringUsingEncoding:NSASCIIStringEncoding],
                           [value length]));
  self.device->send<SetMIDIPortInfoCommand>(portInfo);
}

- (NSInteger)maxLength {
  const MIDIPortInfo &portInfo = self.device->get<MIDIPortInfo>(self.portID);
  return portInfo.maxPortName();
}

@end
