/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __ETHERNETPORTINFO_H__
#define __ETHERNETPORTINFO_H__

#include "LibTypes.h"
#include "IPMode.h"
#include "SysexCommand.h"
#include "BytesCommandData.h"
#include "CommandDataKey.h"
#include "property.h"

namespace GeneSysLib {

struct EthernetPortInfo {
  static commandDataKey_t minKey();
  static commandDataKey_t maxKey();
  static commandDataKey_t queryKey(Word portID);
  static CmdEnum retCommand();
  static CmdEnum setCommand();

  EthernetPortInfo(void);

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &begin, BytesIter &end);

  // properties
  Byte versionNumber() const;

  roWord portJackID;

  rwIPModeEnum ipMode;

  rwNetAddr staticIPAddress;
  rwNetAddr staticSubnetMask;
  rwNetAddr staticGateway;

  roNetAddr currentIPAddress;
  roNetAddr currentSubnetMask;
  roNetAddr currentGateway;

  roString macAddress;
  rwString deviceName;
};  // struct EthernetPortInfo

struct GetEthernetPortInfoCommand
    : public BytesSysexCommand<Command::GetEthernetPortInfo> {
  GetEthernetPortInfoCommand(DeviceID deviceID, Word transID, Word portJackID)
      : BytesSysexCommand(deviceID, transID) {
    data.append(portJackID);
  }
};

typedef SysexCommand<Command::SetEthernetPortInfo, EthernetPortInfo>
    SetEthernetPortInfoCommand;

}  // namespace GeneSysLib

#endif  // __ETHERNETPORTINFO_H__
