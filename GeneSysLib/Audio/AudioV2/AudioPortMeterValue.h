/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef AUDIOPORTMETERVALUE_H
#define AUDIOPORTMETERVALUE_H

#include "BytesCommandData.h"
#include "CommandDataKey.h"
#include "CommandDefines.h"
#include "LibTypes.h"
#include "SysexCommand.h"
#include "AudioDeviceTypes.h"
#include "property.h"
#include "ControllerType.h"

namespace GeneSysLib {

struct AudioPortMeterValue {
  typedef std::bitset<8> MeterTypeBitmap;

  struct MeterBlock {
    MeterTypeBitmap meterType;
    roByte meterValueCount = roByte(0);
    roWord *meterValues = 0;
  };

  static commandDataKey_t minKey();
  static commandDataKey_t maxKey();
  static commandDataKey_t queryKey(Word audioPortID);
  static CmdEnum retCommand();

  AudioPortMeterValue();

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &beginIter, BytesIter &endIter);
  Word meterValue(int blockIndex, int meterIndex) const;

  // properties
  Byte versionNumber() const;
  roWord audioPortID;
  roByte meterBlockCount = roByte(0);
  MeterBlock *meterBlocks = 0;

  MeterTypeBitmap meterType;

};  // struct AudioPortMeterValue

struct GetAudioPortMeterValueCommand
    : public BytesSysexCommand<Command::GetAudioPortMeterValue> {
  GetAudioPortMeterValueCommand(DeviceID deviceID, Word transID,
                                    Word audioPortID)
      : BytesSysexCommand(deviceID, transID) {
    data.append(audioPortID);
    data.append((Byte)7);
  }
};  // GetAudioPortMeterValueCommand

}  // namespace GeneSysLib

#endif // AUDIOPORTMETERVALUE_H
