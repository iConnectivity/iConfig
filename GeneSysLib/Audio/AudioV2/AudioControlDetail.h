/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __AUDIOCONTROLDETAIL_H__
#define __AUDIOCONTROLDETAIL_H__

#include "BytesCommandData.h"
#include "CommandDataKey.h"
#include "CommandDefines.h"
#include "LibTypes.h"
#include "SysexCommand.h"
#include "AudioDeviceTypes.h"
#include "property.h"
#include "ControllerType.h"
#include "AudioControlDetailTypes.h"

namespace GeneSysLib {

struct AudioControlDetail {
  static commandDataKey_t minKey();
  static commandDataKey_t maxKey();
  static commandDataKey_t queryKey(Word audioPortID, Byte controllerNumber,
                                   Byte detailNumber);
  static CmdEnum retCommand();
  static CmdEnum setCommand();

  AudioControlDetail();

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &begin, BytesIter &end);

  // properties
  Byte versionNumber() const;
  roWord audioPortID;
  roByte controllerNumber;
  roByte detailNumber;
  roControllerTypeEnum controllerType;

  const AudioControlDetailTypes::Selector &selector() const;

  AudioControlDetailTypes::Feature &feature();
  const AudioControlDetailTypes::Feature &feature() const;

  const AudioControlDetailTypes::ClockSource clockSource() const;

  std::string channelName() const;

 private:
  AudioControlDetailTypes::Variants m_details;
};  // struct AudioControlDetail

struct GetAudioControlDetailCommand
    : public BytesSysexCommand<Command::GetAudioControlDetail> {
  GetAudioControlDetailCommand(DeviceID deviceID, Word transID,
                               Word audioPortID, Byte controllerNumber,
                               Byte detailNumber)
      : BytesSysexCommand(deviceID, transID) {
    data.append(audioPortID);
    data.append(controllerNumber);
    data.append(detailNumber);
  }
};  // GetAudioControlDetailTypes

typedef SysexCommand<Command::SetAudioControlDetail, AudioControlDetail>
    SetAudioControlDetailCommand;

}  // namespace GeneSysLib

#endif  // __AUDIOCONTROLDETAIL_H__
