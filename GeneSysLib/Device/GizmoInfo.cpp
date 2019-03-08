/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "GizmoInfo.h"
#include <limits>

using namespace boost;
using namespace std;

namespace GeneSysLib {

commandDataKey_t GizmoInfo::minKey() {
  return generateKey(Command::RetGizmoInfo);
}

commandDataKey_t GizmoInfo::maxKey() {
  return GizmoInfo::queryKey(std::numeric_limits<Word>::max());
}

commandDataKey_t GizmoInfo::queryKey(Word gizmoID) {
  return generateKey(Command::RetGizmoInfo, gizmoID);
}

CmdEnum GizmoInfo::retCommand() { return Command::RetGizmoInfo; }

GizmoInfo::GizmoInfo(void)
    : gizmoID(), gizmoType(), midiPortID(), pid(), serialNumber() {}

const commandDataKey_t GizmoInfo::key() const {
  return generateKey(Command::RetGizmoInfo);
}

Bytes GizmoInfo::generate() const {
  Bytes result;

  result += versionNumber();
  appendMidiWord(result, gizmoID());
  result += static_cast<Byte>(gizmoType());
  appendMidiWord(result, midiPortID());
  appendMidiWord(result, pid());
  copy(serialNumber().begin(), serialNumber().end(), back_inserter(result));

  return result;
}

void GizmoInfo::parse(BytesIter &beginIter, BytesIter &endIter) {
  Byte version = nextMidiByte(beginIter, endIter);

  if (version == versionNumber()) {
    gizmoID = roWord(nextMidiWord(beginIter, endIter));
    gizmoType = roGizmoTypeEnum(
        static_cast<GizmoTypeEnum>(nextMidiByte(beginIter, endIter)));
    midiPortID = roWord(nextMidiWord(beginIter, endIter));
    pid = roWord(nextMidiWord(beginIter, endIter));

    if ((size_t)distance(beginIter, endIter) >= (size_t)serialNumber().size()) {
      SerialNumber sn;
      copy(beginIter, (beginIter + sn.size()), sn.begin());
      serialNumber = roSerialNumber(sn);
    }
  }
}

Byte GizmoInfo::versionNumber() const { return 0x01; }

}  // namespace GeneSysLib
