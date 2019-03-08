/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __AUDIOCONTROLPARAM_H__
#define __AUDIOCONTROLPARAM_H__

#include "BytesCommandData.h"
#include "CommandDataKey.h"
#include "CommandDefines.h"
#include "LibTypes.h"
#include "SysexCommand.h"
#include "AudioDeviceTypes.h"
#include "property.h"
#include "ControllerType.h"
#include "AudioControlTypes.h"

namespace GeneSysLib {

struct AudioControlParm {
  static commandDataKey_t minKey();
  static commandDataKey_t maxKey();
  static commandDataKey_t queryKey(Word audioPortID, Byte controllerNumber);
  static CmdEnum retCommand();
  static CmdEnum setCommand();

  AudioControlParm();

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &begin, BytesIter &end);

  // properties
  Byte versionNumber() const;
  roWord audioPortID;
  roByte controllerNumber;
  roControllerTypeEnum controllerType;

  bool isOfType(ControllerTypeEnum controllerType) const;

  Byte numDetails() const;

  AudioControllerType::Selector &selector();
  const AudioControllerType::Selector &selector() const;

  const AudioControllerType::Feature &feature() const;

  AudioControllerType::ClockSource &clockSource();
  const AudioControllerType::ClockSource &clockSource() const;

  std::string controllerName() const;

 private:
  AudioControllerType::Variants m_controller;
};  // struct AudioControlParm

struct GetAudioControlParmCommand
    : public BytesSysexCommand<Command::GetAudioControlParm> {
  GetAudioControlParmCommand(DeviceID deviceID, Word transID, Word audioPortID,
                             Byte controllerNumber)
      : BytesSysexCommand(deviceID, transID) {
    // fill the bytes command data with the relavent data
    data.append(audioPortID);
    data.append(controllerNumber);
  }
};  // struct GetAudioControllerParmCommand

typedef SysexCommand<Command::SetAudioControlParm, AudioControlParm>
    SetAudioControlParmCommand;

}  // namespace GeneSysLib

#endif  // __AUDIOCONTROLPARAM_H__
