/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "MixerInputControlValue.h"
#include <limits>

using namespace boost;
using namespace std;

namespace GeneSysLib {

commandDataKey_t MixerInputControlValue::minKey() {
  return generateKey(Command::RetMixerInputControlValue);
}

commandDataKey_t MixerInputControlValue::maxKey() {
  return MixerInputControlValue::queryKey(std::numeric_limits<Word>::max(), std::numeric_limits<Byte>::max(), std::numeric_limits<Byte>::max());
}

commandDataKey_t MixerInputControlValue::queryKey(Word audioPortID, Byte mixerOutputNumber, Byte mixerInputNumber) {
  return generateKey(Command::RetMixerInputControlValue, audioPortID, mixerOutputNumber, mixerInputNumber);
}

MixerInputControlValue::MixerInputControlValue() {}

const commandDataKey_t MixerInputControlValue::key() const {
  return generateKey(Command::RetMixerInputControlValue, audioPortID(), mixerOutputNumber(), mixerInputNumber());
}

CmdEnum MixerInputControlValue::retCommand() { return Command::RetMixerInputControlValue; }

Bytes MixerInputControlValue::generate() const {
  Bytes result;

  result += versionNumber();
  appendMidiWord(result, audioPortID());
  appendMidiByte(result, mixerOutputNumber());
  appendMidiByte(result, mixerInputNumber());
  appendMidiByte(result, includedFlags.to_ulong());
  if (includedFlags.test(0))
    appendMidiWord3Byte(result, volumeControl());
  if (includedFlags.test(1))
    appendMidiByte(result, muteControl());
  if (includedFlags.test(2))
    appendMidiByte(result, soloControl());
  if (includedFlags.test(3))
    appendMidiByte(result, soloPFLControl());
  if (includedFlags.test(4))
    appendMidiByte(result, stereoLinkControl());
  if (includedFlags.test(5))
    appendMidiByte(result, invertControl());
  if (includedFlags.test(6)) {
    appendMidiWord3Byte(result, panControl());
    appendMidiByte(result, panCurveLaw());
  }

  return result;
}

void MixerInputControlValue::parse(BytesIter &beginIter, BytesIter &endIter) {
  Byte version = nextMidiByte(beginIter, endIter);

  if (version == versionNumber()) {
    audioPortID = nextROWord(beginIter, endIter);
    mixerOutputNumber = nextROByte(beginIter, endIter);
    mixerInputNumber = nextROByte(beginIter, endIter);
    includedFlags = std::bitset<8>(nextMidiByte(beginIter, endIter));
    if (includedFlags.test(0))
      volumeControl = nextRWWord3Byte(beginIter, endIter);
    if (includedFlags.test(1))
      muteControl = nextRWByte(beginIter, endIter);
    if (includedFlags.test(2))
      soloControl = nextRWByte(beginIter, endIter);
    if (includedFlags.test(3))
      soloPFLControl = nextRWByte(beginIter, endIter);
    if (includedFlags.test(4))
      stereoLinkControl = nextRWByte(beginIter, endIter);
    if (includedFlags.test(5))
      invertControl = nextRWByte(beginIter, endIter);
    if (includedFlags.test(6)) {
      panControl = nextRWWord3Byte(beginIter, endIter);
      panCurveLaw = nextRWByte(beginIter, endIter);
    }
  }
}

Byte MixerInputControlValue::versionNumber() const { return 0x01; }

}  // namespace GeneSysLib

