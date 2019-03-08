/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "MixerOutputControl.h"
#include <limits>

using namespace boost;
using namespace std;

namespace GeneSysLib {

commandDataKey_t MixerOutputControl::minKey() {
  return generateKey(Command::RetMixerOutputControl);
}

commandDataKey_t MixerOutputControl::maxKey() {
  return MixerOutputControl::queryKey(std::numeric_limits<Word>::max());
}

commandDataKey_t MixerOutputControl::queryKey(Word audioPortID) {
  return generateKey(Command::RetMixerOutputControl, audioPortID);
}

MixerOutputControl::MixerOutputControl() {}

const commandDataKey_t MixerOutputControl::key() const {
  return generateKey(Command::RetMixerOutputControl, audioPortID());
}

CmdEnum MixerOutputControl::retCommand() { return Command::RetMixerOutputControl; }

Bytes MixerOutputControl::generate() const {
  Bytes result;

  result += versionNumber();
  appendMidiWord(result, audioPortID());
  result += static_cast<Byte>(existFlags.to_ulong());
  result += static_cast<Byte>(editFlags.to_ulong());
  if (existFlags.test(6)) {
    appendMidiWord3Byte(result, maximumPanControl());
    appendMidiByte(result, numberPanCurveLaws());
    for (int i = 0; i < numberPanCurveLaws(); i++) {
      appendMidiByte(result, panCurveLaws.at(i)());
    }
  }
  if (existFlags.test(0)) {
    appendMidiWord3Byte(result, minimumVolumeControl());
    appendMidiWord3Byte(result, maximumVolumeControl());
    appendMidiWord3Byte(result, resolutionVolumeControl());
  }
  return result;
}

void MixerOutputControl::parse(BytesIter &beginIter, BytesIter &endIter) {
  Byte version = nextMidiByte(beginIter, endIter);

  if (version == versionNumber()) {
    audioPortID = nextROWord(beginIter, endIter);
    existFlags = std::bitset<8>(nextMidiByte(beginIter, endIter));
    editFlags = std::bitset<8>(nextMidiByte(beginIter, endIter));
    if (existFlags.test(6)) { // if pan exists
      maximumPanControl = nextROWord3Byte(beginIter, endIter);
      numberPanCurveLaws = nextROByte(beginIter, endIter);
      panCurveLaws.clear();
      for (int i = 0; i < numberPanCurveLaws(); i++) {
        roByte plc = nextROByte(beginIter, endIter);
        panCurveLaws.push_back(plc);
      }
    }
    if ((existFlags.test(0)) || (existFlags.test(2))) { //if volume or solo
      minimumVolumeControl = nextROWord3Byte(beginIter, endIter);
      maximumVolumeControl = nextROWord3Byte(beginIter, endIter);
      resolutionVolumeControl = nextROWord3Byte(beginIter, endIter);
    }
  }
}

Byte MixerOutputControl::versionNumber() const { return 0x01; }

}  // namespace GeneSysLib

