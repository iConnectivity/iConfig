/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "MixerPortParm.h"
#include <limits>

using namespace boost;
using namespace std;

namespace GeneSysLib {
/*
commandDataKey_t MixerPortParm::minKey() {
  return generateKey(Command::RetMixerPortParm);
}

commandDataKey_t MixerPortParm::maxKey() {
  return MixerPortParm::queryKey(Command::RetMixerPortParm);
}

commandDataKey_t MixerPortParm::queryKey() {
  return generateKey(Command::RetMixerPortParm);
}*/

MixerPortParm::MixerPortParm() {}

const commandDataKey_t MixerPortParm::key() const {
  return generateKey(Command::RetMixerPortParm);
}

CmdEnum MixerPortParm::retCommand() { return Command::RetMixerPortParm; }

Bytes MixerPortParm::generate() const {
  Bytes result;

  result += versionNumber();
  appendMidiByte(result, audioPortMixerBlockCount());
  for (int i = 0; i < audioPortMixerBlockCount(); i++) {
    AudioPortMixerBlock apmb = audioPortMixerBlocks.at(i);
    appendMidiWord(result, apmb.audioPortID());
    appendMidiByte(result, apmb.numInputs());
    appendMidiByte(result, apmb.numOutputs());
  }

  return result;
}

void MixerPortParm::parse(BytesIter &beginIter, BytesIter &endIter) {
  Byte version = nextMidiByte(beginIter, endIter);

  if (version == versionNumber()) {
    audioPortMixerBlockCount = nextROByte(beginIter, endIter);
    audioPortMixerBlocks.clear();
    for (int i = 0; i < audioPortMixerBlockCount(); i++) {
      AudioPortMixerBlock apmb;
      apmb.audioPortID = nextROWord(beginIter, endIter);
      apmb.numInputs = nextRWByte(beginIter, endIter);
      apmb.numOutputs = nextRWByte(beginIter, endIter);
      audioPortMixerBlocks.push_back(apmb);
    }
  }
}

Byte MixerPortParm::versionNumber() const { return 0x01; }

}  // namespace GeneSysLib

