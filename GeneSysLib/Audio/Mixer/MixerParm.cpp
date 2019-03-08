/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "MixerParm.h"
#include <limits>

using namespace boost;
using namespace std;

namespace GeneSysLib {

commandDataKey_t MixerParm::minKey() {
  return generateKey(Command::RetMixerParm);
}

commandDataKey_t MixerParm::maxKey() {
  return MixerParm::queryKey(std::numeric_limits<Byte>::max());
}

commandDataKey_t MixerParm::queryKey(Byte audioConfigurationNumber) {
  return generateKey(Command::RetMixerParm, audioConfigurationNumber);
}

MixerParm::MixerParm() {}

const commandDataKey_t MixerParm::key() const {
  return generateKey(Command::RetMixerParm, audioConfigurationNumber());
}

CmdEnum MixerParm::retCommand() { return Command::RetMixerParm; }

Bytes MixerParm::generate() const {
  Bytes result;

  result += versionNumber();
  appendMidiByte(result, activeMixerConfigurationBlock());
  appendMidiByte(result, audioConfigurationNumber());
  appendMidiByte(result, mixerBlockCount());
  for (int i = 0; i < mixerBlockCount(); i++) {
    MixerBlock mb = mixerBlocks.at(i);
    appendMidiByte(result, mb.mixerConfigurationNumber());
    appendMidiByte(result, mb.maximumInputs());
    appendMidiByte(result, mb.maximumOutputs());
  }

  return result;
}

void MixerParm::parse(BytesIter &beginIter, BytesIter &endIter) {
  Byte version = nextMidiByte(beginIter, endIter);

  if (version == versionNumber()) {
    activeMixerConfigurationBlock = rwByte(nextMidiByte(beginIter, endIter));
    audioConfigurationNumber = roByte(nextMidiByte(beginIter, endIter));
    mixerBlockCount = roByte(nextMidiByte(beginIter, endIter));
    mixerBlocks.clear();
    for (int i = 0; i < mixerBlockCount(); i++) {
      MixerBlock mb;
      mb.mixerConfigurationNumber = roByte(nextMidiByte(beginIter, endIter));
      mb.maximumInputs = roByte(nextMidiByte(beginIter, endIter));
      mb.maximumOutputs = roByte(nextMidiByte(beginIter,endIter));
      mixerBlocks.push_back(mb);
    }
  }
}

Byte MixerParm::versionNumber() const { return 0x01; }

}  // namespace GeneSysLib

