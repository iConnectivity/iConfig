/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "CommandDefines.h"

using namespace std;

namespace GeneSysLib {

set<CmdEnum> commandDependancy(CmdEnum command) {
  // D is depencency set for command
  set<CmdEnum> D;

  switch (command) {
  case Command::GetDevice:
  case Command::RetDevice:
  case Command::ACK:
    // NO DEPENDANCY
    break;

  case Command::GetCommandList:
  case Command::RetCommandList:
    D.insert(Command::RetDevice);
    break;

  case Command::GetInfoList:
  case Command::RetInfoList:
    D.insert(Command::RetDevice);
    D.insert(Command::RetCommandList);
    break;

  case Command::GetInfo:
  case Command::RetInfo:
  case Command::SetInfo:
    D.insert(Command::RetDevice);
    D.insert(Command::RetCommandList);
    D.insert(Command::RetInfoList);
    break;

  case Command::GetResetList:
  case Command::RetResetList:
  case Command::GetSaveRestoreList:
  case Command::RetSaveRestoreList:
    D.insert(Command::RetDevice);
    D.insert(Command::RetCommandList);
    break;

  case Command::Reset:
    D.insert(Command::RetDevice);
    D.insert(Command::RetCommandList);
    D.insert(Command::RetResetList);
    break;

  case Command::SaveRestore:
    D.insert(Command::RetDevice);
    D.insert(Command::RetCommandList);
    D.insert(Command::RetSaveRestoreList);
    break;

  case Command::GetGizmoCount:
  case Command::RetGizmoCount:
    // NO DEPENDANCIES
    break;

  case Command::GetGizmoInfo:
  case Command::RetGizmoInfo:
    D.insert(Command::RetDevice);
    D.insert(Command::RetCommandList);
    D.insert(Command::RetGizmoCount);
    break;

  case Command::GetEthernetPortInfo:
  case Command::RetEthernetPortInfo:
  case Command::SetEthernetPortInfo:
    D.insert(Command::RetDevice);
    D.insert(Command::RetCommandList);
    D.insert(Command::RetMIDIInfo);
    break;

    // MIDI Related
  case Command::GetMIDIPortInfo:
  case Command::RetMIDIPortInfo:
  case Command::SetMIDIPortInfo:
  case Command::GetMIDIPortFilter:
  case Command::RetMIDIPortFilter:
  case Command::SetMIDIPortFilter:
  case Command::GetMIDIPortRemap:
  case Command::RetMIDIPortRemap:
  case Command::SetMIDIPortRemap:
  case Command::GetMIDIPortRoute:
  case Command::RetMIDIPortRoute:
  case Command::SetMIDIPortRoute:
  case Command::GetMIDIInfo:
  case Command::RetMIDIInfo:
  case Command::SetMIDIInfo:
  case Command::GetRTPMIDIConnectionDetail:
  case Command::RetRTPMIDIConnectionDetail:
  case Command::GetUSBHostMIDIDeviceDetail:
  case Command::RetUSBHostMIDIDeviceDetail:
    D.insert(Command::RetDevice);
    D.insert(Command::RetCommandList);
    D.insert(Command::RetMIDIInfo);
    break;

  case Command::GetMIDIPortDetail:
  case Command::RetMIDIPortDetail:
  case Command::SetMIDIPortDetail:
    D.insert(Command::RetMIDIInfo);
    break;

    // Audio Related
  case Command::GetAudioInfo:
  case Command::RetAudioInfo:
  case Command::GetAudioClockInfo:
  case Command::RetAudioClockInfo:
  case Command::SetAudioClockInfo:
  case Command::GetAudioGlobalParm:
  case Command::RetAudioGlobalParm:
  case Command::GetAudioClockParm:
  case Command::RetAudioClockParm:
    D.insert(Command::RetDevice);
    D.insert(Command::RetCommandList);
    break;

  case Command::GetAudioCfgInfo:
  case Command::RetAudioCfgInfo:
  case Command::SetAudioCfgInfo:
  case Command::GetAudioPortInfo:
  case Command::RetAudioPortInfo:
  case Command::SetAudioPortInfo:
  case Command::GetAudioPortCfgInfo:
  case Command::RetAudioPortCfgInfo:
  case Command::SetAudioPortCfgInfo:
  case Command::GetAudioPortPatchbay:
  case Command::RetAudioPortPatchbay:
  case Command::SetAudioPortPatchbay:
    D.insert(Command::RetDevice);
    D.insert(Command::RetCommandList);
    D.insert(Command::RetAudioInfo);
    break;

  case Command::GetAudioPortParm:
  case Command::RetAudioPortParm:
  case Command::GetAudioDeviceParm:
  case Command::RetAudioDeviceParm:
  case Command::GetAudioPatchbayParm:
  case Command::RetAudioPatchbayParm:
    D.insert(Command::RetDevice);
    D.insert(Command::RetCommandList);
    D.insert(Command::RetAudioGlobalParm);
    break;

  case Command::GetAudioControlParm:
  case Command::RetAudioControlParm:
  case Command::GetMixerParm:
  case Command::RetMixerParm:
  case Command::GetMixerPortParm:
  case Command::RetMixerPortParm:
    D.insert(Command::RetDevice);
    D.insert(Command::RetCommandList);
    D.insert(Command::RetAudioGlobalParm);
    D.insert(Command::RetAudioDeviceParm);
/*    D.insert(Command::RetAudioControlParm);
    D.insert(Command::RetAudioControlDetail);*/
    break;

  case Command::GetMixerInputParm:
  case Command::RetMixerInputParm:
  case Command::GetMixerOutputParm:
  case Command::RetMixerOutputParm:
  case Command::GetMixerInputControl:
  case Command::RetMixerInputControl:
  case Command::GetMixerOutputControl:
  case Command::RetMixerOutputControl:
  case Command::GetMixerInputControlValue:
  case Command::RetMixerInputControlValue:
  case Command::GetMixerOutputControlValue:
  case Command::RetMixerOutputControlValue:
  case Command::GetMixerMeterValue:
  case Command::RetMixerMeterValue:
    D.insert(Command::RetDevice);
    D.insert(Command::RetCommandList);
    D.insert(Command::RetAudioGlobalParm);
    D.insert(Command::RetMixerParm);
    D.insert(Command::RetMixerPortParm);
    break;

  case Command::GetAudioControlDetail:
  case Command::RetAudioControlDetail:
  case Command::GetAudioControlDetailValue:
  case Command::RetAudioControlDetailValue:
    D.insert(Command::RetDevice);
    D.insert(Command::RetCommandList);
    D.insert(Command::RetAudioGlobalParm);
    D.insert(Command::RetAudioControlParm);
    break;

  default:
    // NO DEPENDANCY
    break;
  }

  return D;
}

}  // namespace GeneSysLib
