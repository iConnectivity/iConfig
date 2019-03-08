/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "MixerInputParm.h"
#include <limits>

using namespace boost;
using namespace std;

namespace GeneSysLib {

commandDataKey_t MixerInputParm::minKey() {
  return generateKey(Command::RetMixerInputParm);
}

commandDataKey_t MixerInputParm::maxKey() {
  return MixerInputParm::queryKey(std::numeric_limits<Word>::max(), std::numeric_limits<Byte>::max());
}

commandDataKey_t MixerInputParm::queryKey(Word audioPortID, Byte mixerInputNumber) {
  return generateKey(Command::RetMixerInputParm, audioPortID, mixerInputNumber);
}

MixerInputParm::MixerInputParm() {}

const commandDataKey_t MixerInputParm::key() const {
  return generateKey(Command::RetMixerInputParm, audioPortID(), mixerInputNumber());
}

CmdEnum MixerInputParm::retCommand() { return Command::RetMixerInputParm; }

Bytes MixerInputParm::generate() const {
  Bytes result;

  result += versionNumber();
  appendMidiWord(result, audioPortID());
  appendMidiByte(result, mixerInputNumber());
  appendMidiWord(result, audioSourceAudioPortID());
  appendMidiByte(result, audioSourceChannelID());

  return result;
}

void MixerInputParm::parse(BytesIter &beginIter, BytesIter &endIter) {
  Byte version = nextMidiByte(beginIter, endIter);

  if (version == versionNumber()) {
    audioPortID = nextROWord(beginIter, endIter);
    mixerInputNumber = nextROByte(beginIter, endIter);
    audioSourceAudioPortID = nextRWWord(beginIter, endIter);
    audioSourceChannelID = nextRWByte(beginIter, endIter);
  }
}

Byte MixerInputParm::versionNumber() const { return 0x01; }

}  // namespace GeneSysLib

