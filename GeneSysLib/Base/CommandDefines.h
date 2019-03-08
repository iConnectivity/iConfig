/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __COMMANDDEFINES_H__
#define __COMMANDDEFINES_H__

#include <set>
#include <vector>
#include "property.h"

namespace GeneSysLib {

#define ANSWR_BIT 0x0000
#define WRITE_BIT 0x2000
#define QUERY_BIT 0x2000

namespace Command {
typedef enum Enum {
  GetDevice = QUERY_BIT | 0x0001,
  RetDevice = ANSWR_BIT | 0x0002,

  GetCommandList = QUERY_BIT | 0x0003,
  RetCommandList = ANSWR_BIT | 0x0004,

  GetInfoList = QUERY_BIT | 0x0005,
  RetInfoList = ANSWR_BIT | 0x0006,

  GetInfo = QUERY_BIT | 0x0007,
  RetInfo = ANSWR_BIT | 0x0008,
  SetInfo = WRITE_BIT | 0x0008,

  GetResetList = QUERY_BIT | 0x0009,
  RetResetList = ANSWR_BIT | 0x000A,

  GetSaveRestoreList = QUERY_BIT | 0x000B,
  RetSaveRestoreList = ANSWR_BIT | 0x000C,

  GetEthernetPortInfo = QUERY_BIT | 0x000D,
  RetEthernetPortInfo = ANSWR_BIT | 0x000E,
  SetEthernetPortInfo = WRITE_BIT | 0x000E,

  ACK = ANSWR_BIT | 0x000F,

  Reset = WRITE_BIT | 0x0010,

  SaveRestore = WRITE_BIT | 0x0011,

  GetGizmoCount = QUERY_BIT | 0x0012,
  RetGizmoCount = ANSWR_BIT | 0x0013,

  GetGizmoInfo = QUERY_BIT | 0x0014,
  RetGizmoInfo = ANSWR_BIT | 0x0015,


  // MIDI related
  GetMIDIInfo = QUERY_BIT | 0x0020,
  RetMIDIInfo = ANSWR_BIT | 0x0021,
  SetMIDIInfo = WRITE_BIT | 0x0021,

  GetMIDIPortInfo = QUERY_BIT | 0x0022,
  RetMIDIPortInfo = ANSWR_BIT | 0x0023,
  SetMIDIPortInfo = WRITE_BIT | 0x0023,

  GetMIDIPortFilter = QUERY_BIT | 0x0024,
  RetMIDIPortFilter = ANSWR_BIT | 0x0025,
  SetMIDIPortFilter = WRITE_BIT | 0x0025,

  GetMIDIPortRemap = QUERY_BIT | 0x0026,
  RetMIDIPortRemap = ANSWR_BIT | 0x0027,
  SetMIDIPortRemap = WRITE_BIT | 0x0027,

  GetMIDIPortRoute = QUERY_BIT | 0x0028,
  RetMIDIPortRoute = ANSWR_BIT | 0x0029,
  SetMIDIPortRoute = WRITE_BIT | 0x0029,

  GetMIDIPortDetail = QUERY_BIT | 0x002A,
  RetMIDIPortDetail = ANSWR_BIT | 0x002B,
  SetMIDIPortDetail = WRITE_BIT | 0x002B,

  GetRTPMIDIConnectionDetail = QUERY_BIT | 0x002C,
  RetRTPMIDIConnectionDetail = ANSWR_BIT | 0x002D,

  GetUSBHostMIDIDeviceDetail = QUERY_BIT | 0x002E,
  RetUSBHostMIDIDeviceDetail = ANSWR_BIT | 0x002F,


  // audio related
  GetAudioInfo = QUERY_BIT | 0x0030,
  RetAudioInfo = ANSWR_BIT | 0x0031,

  GetAudioCfgInfo = QUERY_BIT | 0x0032,
  RetAudioCfgInfo = ANSWR_BIT | 0x0033,
  SetAudioCfgInfo = WRITE_BIT | 0x0033,

  GetAudioPortInfo = QUERY_BIT | 0x0034,
  RetAudioPortInfo = ANSWR_BIT | 0x0035,
  SetAudioPortInfo = WRITE_BIT | 0x0035,

  GetAudioPortCfgInfo = QUERY_BIT | 0x0036,
  RetAudioPortCfgInfo = ANSWR_BIT | 0x0037,
  SetAudioPortCfgInfo = WRITE_BIT | 0x0037,

  GetAudioPortPatchbay = QUERY_BIT | 0x0038,
  RetAudioPortPatchbay = ANSWR_BIT | 0x0039,
  SetAudioPortPatchbay = WRITE_BIT | 0x0039,

  GetAudioClockInfo = QUERY_BIT | 0x003A,
  RetAudioClockInfo = ANSWR_BIT | 0x003B,
  SetAudioClockInfo = WRITE_BIT | 0x003B,


  // audio 2 related
  GetAudioGlobalParm = QUERY_BIT | 0x40,
  RetAudioGlobalParm = ANSWR_BIT | 0x41,
  SetAudioGlobalParm = WRITE_BIT | 0x41,

  GetAudioPortParm = QUERY_BIT | 0x42,
  RetAudioPortParm = ANSWR_BIT | 0x43,
  SetAudioPortParm = WRITE_BIT | 0x43,

  GetAudioDeviceParm = QUERY_BIT | 0x44,
  RetAudioDeviceParm = ANSWR_BIT | 0x45,
  SetAudioDeviceParm = WRITE_BIT | 0x45,

  GetAudioControlParm = QUERY_BIT | 0x46,
  RetAudioControlParm = ANSWR_BIT | 0x47,
  SetAudioControlParm = WRITE_BIT | 0x47,

  GetAudioControlDetail = QUERY_BIT | 0x48,
  RetAudioControlDetail = ANSWR_BIT | 0x49,
  SetAudioControlDetail = WRITE_BIT | 0x49,

  GetAudioControlDetailValue = QUERY_BIT | 0x4A,
  RetAudioControlDetailValue = ANSWR_BIT | 0x4B,
  SetAudioControlDetailValue = WRITE_BIT | 0x4B,

  GetAudioClockParm = QUERY_BIT | 0x4C,
  RetAudioClockParm = ANSWR_BIT | 0x4D,
  SetAudioClockParm = WRITE_BIT | 0x4D,

  GetAudioPatchbayParm = QUERY_BIT | 0x4E,
  RetAudioPatchbayParm = ANSWR_BIT | 0x4F,
  SetAudioPatchbayParm = WRITE_BIT | 0x4F,

  GetAudioChannelName = QUERY_BIT | 0x3C,
  RetAudioChannelName = ANSWR_BIT | 0x3D,
  SetAudioChannelName = WRITE_BIT | 0x3D,

  GetAudioPortMeterValue = QUERY_BIT | 0x3E,
  RetAudioPortMeterValue = ANSWR_BIT | 0x3F,
  SetAudioPortMeterValue = WRITE_BIT | 0x3F,

  GetMixerParm = QUERY_BIT | 0x50,
  RetMixerParm = ANSWR_BIT | 0x51,
  SetMixerParm = WRITE_BIT | 0x51,

  GetMixerPortParm = QUERY_BIT | 0x52,
  RetMixerPortParm = ANSWR_BIT | 0x53,
  SetMixerPortParm = WRITE_BIT | 0x53,

  GetMixerInputParm = QUERY_BIT | 0x54,
  RetMixerInputParm = ANSWR_BIT | 0x55,
  SetMixerInputParm = WRITE_BIT | 0x55,

  GetMixerOutputParm = QUERY_BIT | 0x56,
  RetMixerOutputParm = ANSWR_BIT | 0x57,
  SetMixerOutputParm = WRITE_BIT | 0x57,

  GetMixerInputControl = QUERY_BIT | 0x58,
  RetMixerInputControl = ANSWR_BIT | 0x59,
  SetMixerInputControl = WRITE_BIT | 0x59,

  GetMixerOutputControl = QUERY_BIT | 0x5A,
  RetMixerOutputControl = ANSWR_BIT | 0x5B,
  SetMixerOutputControl = WRITE_BIT | 0x5B,

  GetMixerInputControlValue = QUERY_BIT | 0x5C,
  RetMixerInputControlValue = ANSWR_BIT | 0x5D,
  SetMixerInputControlValue = WRITE_BIT | 0x5D,

  GetMixerOutputControlValue = QUERY_BIT | 0x5E,
  RetMixerOutputControlValue = ANSWR_BIT | 0x5F,
  SetMixerOutputControlValue = WRITE_BIT | 0x5F,

  GetMixerMeterValue = QUERY_BIT | 0x60,
  RetMixerMeterValue = ANSWR_BIT | 0x61,
  SetMixerMeterValue = WRITE_BIT | 0x61,

  Unknown = 0xFF
} Enum;
}  // namespace Command

typedef Command::Enum CmdEnum;

typedef std::vector<CmdEnum> CmdEnumVector;

std::set<CmdEnum> commandDependancy(Command::Enum command);

typedef readonly_property<CmdEnum> roCmdEnum;
typedef readwrite_property<CmdEnum> rwCmdEnum;

}  // namespace GeneSysLib

#endif  // __COMMANDDEFINES_H__
