/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef SCREEN_H
#define SCREEN_H

typedef enum Screen {
  InformationScreen = 0,
  MIDIInformationScreen,
  PortRoutingScreen,
  PortFiltersScreen,
  ChannelRemapScreen,
  CCFiltersScreen,
  CCRemapScreen,
  AudioInformationFormScreen,
  AudioPatchbayScreen,
  AudioControlScreen,
  MixerScreen,
  SaveAsScreen,
  SaveScreen,
  OpenScreen,
  RereadAllScreen,
  RereadAudioControls,
  RereadMeters,
  FirmwareConfigScreen,
  UnknownScreen = 0xFF
} Screen;

#endif  // SCREEN_H
