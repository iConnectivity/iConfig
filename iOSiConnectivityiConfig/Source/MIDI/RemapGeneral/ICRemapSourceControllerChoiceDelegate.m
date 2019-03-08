/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICRemapSourceControllerChoiceDelegate.h"
#import "MIDIPortRemap.h"

using namespace GeneSysLib;

@interface ICRemapSourceControllerChoiceDelegate ()

@property(nonatomic) DeviceInfoPtr device;
@property(nonatomic) Word portID;
@property(nonatomic) RemapTypeEnum remapType;
@property(nonatomic) int remapID;

@end

@implementation ICRemapSourceControllerChoiceDelegate

- (id)initWithDevice:(DeviceInfoPtr)device
              portID:(Word)portID
           remapType:(GeneSysLib::RemapTypeEnum)remapType
             remapID:(int)remapID {
  self = [super init];

  if (self) {
    NSParameterAssert(device);
    NSParameterAssert(portID >= 1);
    NSParameterAssert(device->contains<MIDIPortRemap>(portID, remapType));
    auto &portRemap = device->midiPortRemap(portID, remapType);
    NSParameterAssert(remapID < portRemap.numControllers());

    self.device = device;
    self.portID = portID;
    self.remapType = remapType;
    self.remapID = remapID;
  }

  return self;
}

- (NSString *)title {
  return [NSString stringWithFormat:@"Source: %@",
                                    [[ICControllerChoiceDelegate contollerList]
                                        objectAtIndex:[self getChoice]]];
}

- (NSInteger)getChoice {
  const auto &portRemap =
      self.device->midiPortRemap(self.portID, self.remapType);

  const auto &controllerRemap = portRemap.controller_at(self.remapID);

  return controllerRemap.controllerSource;
}

- (void)setChoice:(NSInteger)value {
  auto &portRemap = self.device->midiPortRemap(self.portID, self.remapType);

  auto &controllerRemap = portRemap.controller_at(self.remapID);

  controllerRemap.controllerSource = value;

  self.device->send<SetMIDIPortRemapCommand>(portRemap);
}

@end
