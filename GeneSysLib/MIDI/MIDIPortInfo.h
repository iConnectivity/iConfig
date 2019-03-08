/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __MIDIPORTINFO_H__
#define __MIDIPORTINFO_H__

#include "LibTypes.h"
#include "PortType.h"
#include "PortMIDIFlags.h"
#include "SysexCommand.h"
#include "BytesCommandData.h"
#include "CommandDataKey.h"
#include "property.h"

namespace GeneSysLib {

struct MIDIPortInfo {
  static commandDataKey_t minKey();
  static commandDataKey_t maxKey();
  static commandDataKey_t queryKey(Word portID);
  static CmdEnum retCommand();
  static CmdEnum setCommand();

  typedef union PortInfo {
    struct Common {
      Byte jack;
    } common;
    struct DIN {
      Byte jack;
    } din;
    struct USBDevice {
      Byte jack;
      Byte devicePort;
    } usbDevice;
    struct USBHost {
      Byte jack;
      Byte hostPort;
    } usbHost;
    struct Ethernet {
      Byte jack;
      Byte session;
    } ethernet;
    Byte raw[4];
  } PortInfo;

  MIDIPortInfo(void);

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &begin, BytesIter &end);

  // properties
  Byte versionNumber() const;
  roWord portID;
  roPortTypeEnum portType;
  PortInfo portInfo() const;

  bool isInputEnabled() const;
  void setInputEnabled(bool enabled);

  bool isOutputEnabled() const;
  void setOutputEnabled(bool enabled);

  roByte maxPortName;

  rwString portName;

  bool isOfType(PortTypeEnum portType) const;

 private:
  PortInfo m_portInfo;
  Byte m_portMIDIFlags;
};  // struct MIDIPortInfo

bool isOfType(const MIDIPortInfo &midiPortInfo, PortTypeEnum portType);

// GetMIDIPortInfoCommand SYSEX command struct
struct GetMIDIPortInfoCommand
    : public BytesSysexCommand<Command::GetMIDIPortInfo> {
  GetMIDIPortInfoCommand(DeviceID deviceID, Word transID, Word portID)
      : BytesSysexCommand(deviceID, transID) {
    data.append(portID);
  }
};

// SetMIDIPortInfoCommand SYSEX command struct
typedef SysexCommand<Command::SetMIDIPortInfo, MIDIPortInfo>
    SetMIDIPortInfoCommand;

}  // namespace GeneSysLib

#endif  // __MIDIPORTINFO_H__
