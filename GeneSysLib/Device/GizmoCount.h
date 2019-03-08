/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __GIZMOCOUNT_H__
#define __GIZMOCOUNT_H__

#include "CommandDataKey.h"
#include "LibTypes.h"
#include "SysexCommand.h"
#include "property.h"

namespace GeneSysLib {

struct GizmoCount {
  static CmdEnum retCommand();

  GizmoCount(void);

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &beginIter, BytesIter &endIter);

  // properties
  roWord gizmoCount;
};  // struct GizmoCount

typedef SysexCommand<Command::GetGizmoCount, EmptyCommandData>
    GetGizmoCountCommand;

}  // namespace GeneSysLib

#endif  // __GIZMOCOUNT_H__
