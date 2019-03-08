/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "Lookup.h"

#ifndef Q_MOC_RUN
#include <boost/tr1/unordered_map.hpp>
#include <boost/range.hpp>
#endif

using std::tr1::unordered_map;
using namespace boost;

namespace GeneSysLib {

std::string cmdString(CmdEnum command) {
  std::string result = "Unknown Command";
  static unordered_map<Byte, std::string> stringMap;

  if (stringMap.size() == 0) {
    stringMap[(Byte)(Command::GetDevice) & 0x7F] = "GetDevice";
    stringMap[(Byte)(Command::RetDevice) & 0x7F] = "RetDevice";
    stringMap[(Byte)(Command::GetCommandList) & 0x7F] = "GetCommandList";
    stringMap[(Byte)(Command::RetCommandList) & 0x7F] = "RetCommandList";
    stringMap[(Byte)(Command::GetInfoList) & 0x7F] = "GetInfoList";
    stringMap[(Byte)(Command::RetInfoList) & 0x7F] = "RetInfoList";
    stringMap[(Byte)(Command::GetInfo) & 0x7F] = "GetInfo";
    stringMap[(Byte)(Command::RetInfo) & 0x7F] = "RetInfo";
    stringMap[(Byte)(Command::SetInfo) & 0x7F] = "SetInfo";
    stringMap[(Byte)(Command::GetResetList) & 0x7F] = "GetResetList";
    stringMap[(Byte)(Command::RetResetList) & 0x7F] = "RetResetList";
    stringMap[(Byte)(Command::GetSaveRestoreList) & 0x7F] =
        "GetSaveRestoreList";
    stringMap[(Byte)(Command::RetSaveRestoreList) & 0x7F] =
        "RetSaveRestoreList";
    stringMap[(Byte)(Command::GetEthernetPortInfo) & 0x7F] =
        "GetEthernetPortInfo";
    stringMap[(Byte)(Command::RetEthernetPortInfo) & 0x7F] =
        "RetEthernetPortInfo";
    stringMap[(Byte)(Command::SetEthernetPortInfo) & 0x7F] =
        "SetEthernetPortInfo";
    stringMap[(Byte)(Command::ACK) & 0x7F] = "ACK";
    stringMap[(Byte)(Command::Reset) & 0x7F] = "Reset";
    stringMap[(Byte)(Command::SaveRestore) & 0x7F] = "Save/Restore";
    stringMap[(Byte)(Command::GetGizmoCount) & 0x7F] = "GetGizmoCount";
    stringMap[(Byte)(Command::RetGizmoCount) & 0x7F] = "RetGizmoCount";
    stringMap[(Byte)(Command::GetGizmoInfo) & 0x7F] = "GetGizmoInfo";
    stringMap[(Byte)(Command::RetGizmoInfo) & 0x7F] = "RetGizmoInfo";
    stringMap[(Byte)(Command::GetMIDIInfo) & 0x7F] = "GetMIDIInfo";
    stringMap[(Byte)(Command::RetMIDIInfo) & 0x7F] = "RetMIDIInfo";
    stringMap[(Byte)(Command::GetMIDIPortInfo) & 0x7F] = "GetMIDIPortInfo";
    stringMap[(Byte)(Command::RetMIDIPortInfo) & 0x7F] = "RetMIDIPortInfo";
    stringMap[(Byte)(Command::GetMIDIPortFilter) & 0x7F] = "GetMIDIPortFilter";
    stringMap[(Byte)(Command::RetMIDIPortFilter) & 0x7F] = "RetMIDIPortFilter";
    stringMap[(Byte)(Command::GetMIDIPortRemap) & 0x7F] = "GetMIDIPortRemap";
    stringMap[(Byte)(Command::RetMIDIPortRemap) & 0x7F] = "RetMIDIPortRemap";
    stringMap[(Byte)(Command::GetMIDIPortRoute) & 0x7F] = "GetMIDIPortRoute";
    stringMap[(Byte)(Command::RetMIDIPortRoute) & 0x7F] = "RetMIDIPortRoute";
    stringMap[(Byte)(Command::GetAudioInfo) & 0x7F] = "GetAudioInfo";
    stringMap[(Byte)(Command::RetAudioInfo) & 0x7F] = "RetAudioInfo";
    stringMap[(Byte)(Command::GetAudioCfgInfo) & 0x7F] = "GetAudioCfgInfo";
    stringMap[(Byte)(Command::RetAudioCfgInfo) & 0x7F] = "RetAudioCfgInfo";
  }

  if (stringMap.find((Byte)command & 0xFF) != stringMap.end()) {
    result = stringMap.at((Byte)command & 0xFF);
  }

  return result;
}

}  // namespace GeneSysLib
