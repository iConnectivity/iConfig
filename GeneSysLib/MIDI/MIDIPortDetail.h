/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __MIDIPORTDETAILDATA_H__
#define __MIDIPORTDETAILDATA_H__

#include "LibTypes.h"
#include "USBHostMIDIDeviceDetail.h"
#include "MIDIPortDetailTypes.h"
#include "SysexCommand.h"
#include "BytesCommandData.h"
#include "CommandDataKey.h"
#include "property.h"

namespace GeneSysLib {

struct MIDIPortDetail {
  static commandDataKey_t minKey();
  static commandDataKey_t maxKey();
  static commandDataKey_t queryKey(Word portID);
  static CmdEnum retCommand();
  static CmdEnum setCommand();

  MIDIPortDetail(void);

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &beginIter, BytesIter &endIter);

  // properties
  Byte versionNumber() const;
  roWord portID;
  roPortTypeEnum portType;

  MIDIPortDetailTypes::USBDevice &getUSBDevice();
  const MIDIPortDetailTypes::USBDevice &getUSBDevice() const;

  MIDIPortDetailTypes::USBHost &getUSBHost();
  const MIDIPortDetailTypes::USBHost &getUSBHost() const;

  MIDIPortDetailTypes::Ethernet &getEthernet();
  const MIDIPortDetailTypes::Ethernet &getEthernet() const;

 private:
  MIDIPortDetailTypes::Variants m_portDetails;
};  // struct MIDIPortDetail

struct GetMIDIPortDetailCommand
    : public BytesSysexCommand<Command::GetMIDIPortDetail> {
  GetMIDIPortDetailCommand(DeviceID deviceID, Word transID, Word portID)
      : BytesSysexCommand(deviceID, transID) {
    data.append(portID);
  }
};

typedef SysexCommand<Command::SetMIDIPortDetail, MIDIPortDetail>
    SetMIDIPortDetailCommand;

}  // namespace GeneSysLib

#endif  //__MIDIPORTDETAILDATA_H__
