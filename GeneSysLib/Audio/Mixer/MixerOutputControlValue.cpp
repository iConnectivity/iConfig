/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "MixerOutputControlValue.h"
#include <limits>

using namespace boost;
using namespace std;

namespace GeneSysLib {

commandDataKey_t MixerOutputControlValue::minKey() {
  return generateKey(Command::RetMixerOutputControlValue);
}

commandDataKey_t MixerOutputControlValue::maxKey() {
  return MixerOutputControlValue::queryKey(std::numeric_limits<Word>::max(), std::numeric_limits<Byte>::max());
}

commandDataKey_t MixerOutputControlValue::queryKey(Word audioPortID, Byte mixerOutputNumber) {
  return generateKey(Command::RetMixerOutputControlValue, audioPortID, mixerOutputNumber);
}

MixerOutputControlValue::MixerOutputControlValue() {}

const commandDataKey_t MixerOutputControlValue::key() const {
  return generateKey(Command::RetMixerOutputControlValue, audioPortID(), mixerOutputNumber());
}

CmdEnum MixerOutputControlValue::retCommand() { return Command::RetMixerOutputControlValue; }

Bytes MixerOutputControlValue::generate() const {
  Bytes result;

  result += versionNumber();
  appendMidiWord(result, audioPortID());
  appendMidiByte(result, mixerOutputNumber());
  appendMidiByte(result, includedFlags.to_ulong());
  if (includedFlags.test(0))
    appendMidiWord3Byte(result, volumeControl());
  if (includedFlags.test(1))
    appendMidiByte(result, muteControl());
  if (includedFlags.test(2))
    appendMidiWord3Byte(result, soloControl());
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

void MixerOutputControlValue::parse(BytesIter &beginIter, BytesIter &endIter) {
  Byte version = nextMidiByte(beginIter, endIter);

  if (version == versionNumber()) {
    audioPortID = nextROWord(beginIter, endIter);
    mixerOutputNumber = nextROByte(beginIter, endIter);
    includedFlags = std::bitset<8>(nextMidiByte(beginIter, endIter));
    if (includedFlags.test(0))
      volumeControl = nextRWWord3Byte(beginIter, endIter);
    if (includedFlags.test(1))
      muteControl = nextRWByte(beginIter, endIter);
    if (includedFlags.test(2))
      soloControl = nextRWWord3Byte(beginIter, endIter);
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

Byte MixerOutputControlValue::versionNumber() const { return 0x01; }

}  // namespace GeneSysLib
