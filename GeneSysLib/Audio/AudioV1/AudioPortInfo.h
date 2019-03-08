/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __AUDIOPORTINFO_H__
#define __AUDIOPORTINFO_H__

#include "CommandDataKey.h"
#include "LibTypes.h"
#include "PortType.h"
#include "PortSpecificOptionsBit.h"
#include "SysexCommand.h"
#include "BytesCommandData.h"
#include "property.h"
#include <bitset>

namespace GeneSysLib {

struct AudioPortInfo {
  static commandDataKey_t minKey();
  static commandDataKey_t maxKey();
  static commandDataKey_t queryKey(Word portID);
  static CmdEnum retCommand();
  static CmdEnum setCommand();

  typedef union PortInfo {
    struct Common {
      Byte jack;
    } common;

    struct USBDevice {
      Byte jack;
    } usbDevice;

    struct USBHost {
      Byte jack;
      Byte hostPort;
    } usbHost;

    struct Ethernet {
      Byte jack;
      Byte portNum;
    } ethernet;

    Byte raw[4];
  } PortInfo;

  typedef std::bitset<32> PortSpecificOptions;

  AudioPortInfo(void);

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &beginIter, BytesIter &endIter);

  // properties
  Byte versionNumber() const;
  roWord portID;
  roPortTypeEnum portType;
  PortInfo portInfo() const;
  roByte maxPortName;

  bool canEditPortName() const;

  bool supportsPCAudio() const;
  bool isPCAudioEnabled() const;
  void setPCAudioEnabled(bool isSet);

  bool supportsIOSAudio() const;
  bool isIOSAudioEnabled() const;
  void setIOSAudioEnabled(bool isSet);

  rwString portName;

 private:
  PortInfo m_portInfo;
  PortSpecificOptions m_portSpecificOptions;
};  // struct AudioPortInfo

struct GetAudioPortInfoCommand
    : public BytesSysexCommand<Command::GetAudioPortInfo> {
  GetAudioPortInfoCommand(DeviceID deviceID, Word transID, Word portID)
      : BytesSysexCommand(deviceID, transID) {
    data.append(portID);
  }
};

typedef SysexCommand<Command::SetAudioPortInfo, AudioPortInfo>
    SetAudioPortInfoCommand;

}  // namespace GeneSysLib

#endif  // __AUDIOPORTINFO_H__
