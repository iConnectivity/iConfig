/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICChannelRemapProvider.h"
#include "MyAlgorithms.h"

enum RemapCell {
  Channel1Cell = 0,
  Channel2Cell,
  Channel3Cell,
  Channel4Cell,
  Channel5Cell,
  Channel6Cell,
  Channel7Cell,
  Channel8Cell,
  Channel9Cell,
  Channel10Cell,
  Channel11Cell,
  Channel12Cell,
  Channel13Cell,
  Channel14Cell,
  Channel15Cell,
  Channel16Cell,
  PitchBendCell,
  ChannelPressureCell,
  ProgramChangeCell,
  ControlChangeCell,
  PolyKeyPressureCell,
  NoteCell
};

using namespace MyAlgorithms;
using namespace GeneSysLib;

@implementation ICChannelRemapProvider

- (id)initForInput:(BOOL)inputType
            portID:(Word)_portID
        forChannel:(NSInteger)channel {
  self = [super init];
  if (self) {
    isInput = inputType;
    inChannel = channel;
    portID = _portID;

    remapID = ((isInput) ? (RemapID::InputRemap) : (RemapID::OutputRemap));

    title = [NSString stringWithFormat:@"Channel %d", (inChannel + 1)];
  }
  return self;
}

- (void)initializeProviderButtons:(ICViewController *)sender {
  [super initializeProviderButtons:sender];
}

- (NSString *)title {
  return title;
}

- (NSString *)providerName {
  return ((isInput) ? (@"ChannelRemapInputSetup")
                    : (@"ChannelRemapOutputSetup"));
}

- (NSUInteger)numberOfColumns {
  return 4;
}

- (NSUInteger)numberOfRows {
  return 7;
}

- (void)providerWillAppear:(ICViewController *)sender {
  assert(sender.device);
  assert(sender.device->contains<MIDIPortRemap>(portID, remapID));
  const auto &portRemap = sender.device->midiPortRemap(portID, remapID);
  const auto &chRemap = portRemap.remapStatus_at(inChannel);

  for (size_t i = Channel1Cell; i <= Channel16Cell; ++i) {
    [(sender.providerButtons)[i] setSelected:(i == chRemap.channelNumber)];
  }

  // Pitch Bend Events
  [(sender.providerButtons)[PitchBendCell] setSelected:chRemap.pitchBendEvents];

  // Channel Pressure Events
  [(sender.providerButtons)[ChannelPressureCell]
      setSelected:chRemap.channelPressureEvents];

  // Program Change Events
  [(sender.providerButtons)[ProgramChangeCell]
      setSelected:chRemap.programChangeEvents];

  // Control Change Events
  [(sender.providerButtons)[ControlChangeCell]
      setSelected:chRemap.controlChangeEvents];

  // PolyKey Pressure Events
  [(sender.providerButtons)[PolyKeyPressureCell]
      setSelected:chRemap.polyKeyPressureEvents];

  // Note Events
  [(sender.providerButtons)[NoteCell] setSelected:chRemap.noteEvents];
}

- (NSUInteger)spanForIndex:(NSUInteger)index {
  return ((index <= Channel16Cell) ? (1) : (2));
}

- (void)onButtonDown:(ICViewController *)sender index:(NSInteger)buttonIndex {
  assert(sender.device);
  assert(sender.device->contains<MIDIPortRemap>(portID, remapID));

  auto &portRemap = sender.device->midiPortRemap(portID, remapID);
  auto &chRemap = portRemap.remapStatus_at(inChannel);

  if (buttonIndex <= Channel16Cell) {
    chRemap.channelNumber = buttonIndex;
    for (size_t i = Channel1Cell; i <= Channel16Cell; ++i) {
      [(sender.providerButtons)[i] setSelected:(i == chRemap.channelNumber)];
    }
  } else {
    // flip the value of selected
    const auto &selected = ![(sender.providerButtons)[buttonIndex] isSelected];
    [(sender.providerButtons)[buttonIndex] setSelected:selected];

    switch (buttonIndex) {
      case PitchBendCell:
        chRemap.pitchBendEvents = selected;
        break;

      case ChannelPressureCell:
        chRemap.channelPressureEvents = selected;
        break;

      case ProgramChangeCell:
        chRemap.programChangeEvents = selected;
        break;

      case ControlChangeCell:
        chRemap.controlChangeEvents = selected;
        break;

      case PolyKeyPressureCell:
        chRemap.polyKeyPressureEvents = selected;
        break;

      case NoteCell:
        chRemap.noteEvents = selected;
        break;

      default:
        break;
    }
  }
  [self startUpdateTimer];
}

- (void)onUpdate:(DeviceID)deviceID transID:(Word)transID {
  const auto &portRemap = self.parent.device->midiPortRemap(portID, remapID);
  self.parent.device->send<SetMIDIPortRemapCommand>(portRemap);
}

@end
