/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef MIXERMETERVALUE_H
#define MIXERMETERVALUE_H


#include "BytesCommandData.h"
#include "CommandDataKey.h"
#include "CommandDefines.h"
#include "LibTypes.h"
#include "SysexCommand.h"
#include "AudioDeviceTypes.h"
#include "property.h"
#include "ControllerType.h"
#include "StreamHelpers.h"

namespace GeneSysLib {

struct MixerMeterValue {
  typedef std::bitset<8> MeterTypeBitmap;

  struct MeterBlock {
    MeterTypeBitmap meterType;
    roByte meterValueCount = roByte(0);
    roWord* meterValues = 0;
  };

  static commandDataKey_t minKey();
  static commandDataKey_t maxKey();
  static commandDataKey_t queryKey(Word audioPortID, Byte mixerOutputNumber);
  static CmdEnum retCommand();

  MixerMeterValue();

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &beginIter, BytesIter &endIter);

  // properties
  Byte versionNumber() const;
  roWord audioPortID;
  roByte mixerOutputNumber;
  MeterTypeBitmap meterType;

  roByte meterBlockCount = roByte(0);
  MeterBlock* meterBlocks = 0;

  Word outputMeter() const;
  Word inputMeter(Byte mixerInputNumber) const;

};  // struct MixerMeterValue

struct GetMixerMeterValueCommand
    : public BytesSysexCommand<Command::GetMixerMeterValue> {
  GetMixerMeterValueCommand(DeviceID deviceID, Word transID,
                                    Word audioPortID, Byte mixerOutputNumber)
      : BytesSysexCommand(deviceID, transID) {
    data.append(audioPortID);
    data.append(mixerOutputNumber);
    data.append((Byte)3);
  }
};  // GetMixerMeterValueCommand

typedef SysexCommand<Command::SetMixerMeterValue,
                     MixerMeterValue> SetMixerMeterValueCommand;

}  // namespace GeneSysLib

#endif // MIXERMETERVALUE_H
