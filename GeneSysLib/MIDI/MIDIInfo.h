/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __MIDIINFO_H__
#define __MIDIINFO_H__

#include "LibTypes.h"
#include "SysexCommand.h"
#include "CommandDataKey.h"
#include "property.h"

namespace GeneSysLib {

namespace GlobalMIDIFlags {
typedef enum Enum {
  RunningStatusOnDINs = 0x01,
  RoutingBetweenMultiPort = 0x02
} Enum;
}  // namespace GlobalMIDIFlags

struct MIDIInfo {
  static CmdEnum retCommand();
  static CmdEnum setCommand();

  MIDIInfo(void);
  MIDIInfo(Byte globalMIDIFlags, Byte maxPortsOnMultiPortUSBDevice);
  MIDIInfo(Word numMIDIPorts, Word hostMIDIPort, Byte numDINPairs,
           Byte numUSBDeviceJacks, Byte numUSBHostJacks, Byte numEthernetJacks,
           Byte numUSBMIDIPortPerDeviceJack, Byte numUSBMIDIPortPerHostJack,
           Byte numRTPMIDISessionsPerEthernetJack,
           Byte numRTPMIDIConnectionsPerSession,
           Byte maxPortsOnMultiPortUSBDevice, Byte globalMIDIFlags);

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &begin, BytesIter &end);

  // properties
  Byte versionNumber() const;

  roWord numMIDIPorts;
  roWord hostMIDIPort;
  roByte numDINPairs;
  roByte numUSBDeviceJacks;
  roByte numUSBHostJacks;
  roByte numEthernetJacks;
  roByte numUSBMIDIPortPerDeviceJack;
  roByte numUSBMIDIPortPerHostJack;
  roByte numRTPMIDISessionsPerEthernetJack;
  roByte numRTPMIDIConnectionsPerSession;
  rwByte maxPortsOnMultiPortUSBDevice;

  Byte globalMIDIFlags() const;
  bool isSet(GlobalMIDIFlags::Enum globalFlag) const;
  void setFlag(GlobalMIDIFlags::Enum globalFlag, bool value);

 private:
  Word m_numMIDIPorts;
  Byte m_globalMIDIFlags;
};  // struct MIDIInfo

typedef SysexCommand<Command::GetMIDIInfo, EmptyCommandData> GetMIDIInfoCommand;

struct SetMIDIInfoCommand
    : public SysexCommand<Command::SetMIDIInfo, MIDIInfo> {
  SetMIDIInfoCommand(DeviceID deviceID, Word transID, MIDIInfo midiInfo)
      : SysexCommand(deviceID, transID, midiInfo) {}
  SetMIDIInfoCommand(DeviceID deviceID, Word transID, Byte globalMIDIFlags,
                     Byte maxPortsOnMultiPortUSBDevice)
      : SysexCommand(deviceID, transID,
                     MIDIInfo(globalMIDIFlags, maxPortsOnMultiPortUSBDevice)) {}
};

}  // namespace GeneSysLib

#endif  // __MIDIINFO_H__
