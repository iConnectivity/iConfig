/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __AUDIODEVICEPARAM_H__
#define __AUDIODEVICEPARAM_H__

#include "BytesCommandData.h"
#include "CommandDataKey.h"
#include "CommandDefines.h"
#include "LibTypes.h"
#include "SysexCommand.h"
#include "PortType.h"
#include "HostType.h"
#include "AudioDeviceTypes.h"
#include "property.h"

namespace GeneSysLib {

struct AudioDeviceParm {
  static commandDataKey_t minKey();
  static commandDataKey_t maxKey();
  static commandDataKey_t queryKey(Word portID);
  static CmdEnum retCommand();
  static CmdEnum setCommand();

  AudioDeviceParm();

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &begin, BytesIter &end);

  // properties
  Byte versionNumber() const;
  roWord audioPortID;
  roPortTypeEnum portType;

  roByte maxControllers;

  const AudioDeviceTypes::USBDevice &usbDevice() const;
  AudioDeviceTypes::USBHost &usbHost();
  const AudioDeviceTypes::USBHost &usbHost() const;

 private:
  AudioDeviceTypes::Variants m_details;
};

struct GetAudioDeviceParmCommand
    : public BytesSysexCommand<Command::GetAudioDeviceParm> {
  GetAudioDeviceParmCommand(DeviceID deviceID, Word transID, Word audioPortID)
      : BytesSysexCommand(deviceID, transID) {
    data.append(audioPortID);
  }
};

typedef SysexCommand<Command::SetAudioDeviceParm, AudioDeviceParm>
    SetAudioDeviceParmCommand;

}  // namespace GeneSysLib

#endif  // __AUDIODEVICEPARAM_H__
