/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "MixerOutputParm.h"
#include <limits>

using namespace boost;
using namespace std;

namespace GeneSysLib {

commandDataKey_t MixerOutputParm::minKey() {
  return generateKey(Command::RetMixerOutputParm);
}

commandDataKey_t MixerOutputParm::maxKey() {
  return MixerOutputParm::queryKey(std::numeric_limits<Word>::max(), std::numeric_limits<Byte>::max());
}

commandDataKey_t MixerOutputParm::queryKey(Word audioPortID, Byte mixerOutputNumber) {
  return generateKey(Command::RetMixerOutputParm, audioPortID, mixerOutputNumber);
}

MixerOutputParm::MixerOutputParm() {}

const commandDataKey_t MixerOutputParm::key() const {
  return generateKey(Command::RetMixerOutputParm, audioPortID(), mixerOutputNumber());
}

CmdEnum MixerOutputParm::retCommand() { return Command::RetMixerOutputParm; }

Bytes MixerOutputParm::generate() const {
  Bytes result;

  result += versionNumber();
  appendMidiWord(result, audioPortID());
  appendMidiByte(result, mixerOutputNumber());
  appendMidiByte(result, numberOfMixerOutputAssignments());
  for (int i = 0; i < numberOfMixerOutputAssignments(); i++)
    appendMidiByte(result, mixerOutputAssignments.at(i)());
  appendMidiByte(result, maximumMixNameLength());
  appendMidiByte(result, mixNameLength());
  for (int i = 0; i < mixNameLength(); i++)
    appendMidiByte(result, mixName.at(i)());

  return result;
}

void MixerOutputParm::parse(BytesIter &beginIter, BytesIter &endIter) {
  Byte version = nextMidiByte(beginIter, endIter);

  if (version == versionNumber()) {
    audioPortID = nextROWord(beginIter, endIter);
    mixerOutputNumber = nextROByte(beginIter, endIter);
    numberOfMixerOutputAssignments = nextRWByte(beginIter, endIter);
    mixerOutputAssignments.clear();
    for (int i = 0; i < numberOfMixerOutputAssignments(); i++) {
      mixerOutputAssignments.push_back(nextRWByte(beginIter, endIter));
    }
    maximumMixNameLength = nextROByte(beginIter, endIter);
    mixNameLength = nextRWByte(beginIter, endIter);
    mixName.clear();
    for (int i = 0; i < mixNameLength(); i++) {
      mixName.push_back(nextRWByte(beginIter, endIter));
    }
  }
}

Byte MixerOutputParm::versionNumber() const { return 0x01; }

}  // namespace GeneSysLib

