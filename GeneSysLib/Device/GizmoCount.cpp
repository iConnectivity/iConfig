/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "GizmoCount.h"

namespace GeneSysLib {

CmdEnum GizmoCount::retCommand() { return Command::RetGizmoCount; }

GizmoCount::GizmoCount(void) : gizmoCount() {}

const commandDataKey_t GizmoCount::key() const {
  return generateKey(Command::RetGizmoCount);
}

Bytes GizmoCount::generate() const {
  Bytes result;

  appendMidiWord(result, gizmoCount());

  return result;
}

void GizmoCount::parse(BytesIter &beginIter, BytesIter &endIter) {
  gizmoCount = roWord(nextMidiWord(beginIter, endIter));
}

}  // namespace GeneSysLib
