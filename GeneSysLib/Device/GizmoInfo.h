/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __GIZMOINFO_H__
#define __GIZMOINFO_H__

#include "LibTypes.h"
#include "SysexCommand.h"
#include "BytesCommandData.h"
#include "CommandDataKey.h"
#include "property.h"

namespace GeneSysLib {

namespace GizmoType {
typedef enum Enum { QueryWrite = 1, AnswerRead = 2 } Enum;
}  // namespace GizmoTypeCode
typedef GizmoType::Enum GizmoTypeEnum;
typedef readonly_property<GizmoTypeEnum> roGizmoTypeEnum;

struct GizmoInfo {
  static commandDataKey_t minKey();
  static commandDataKey_t maxKey();
  static commandDataKey_t queryKey(Word gizmoID);
  static CmdEnum retCommand();

  GizmoInfo(void);

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &beginIter, BytesIter &endIter);

  // properties
  Byte versionNumber() const;
  roWord gizmoID;
  roGizmoTypeEnum gizmoType;
  roWord midiPortID;
  roWord pid;
  roSerialNumber serialNumber;
};  // struct GizmoInfo

struct GetGizmoInfoCommand : public BytesSysexCommand<Command::GetGizmoInfo> {
  GetGizmoInfoCommand(DeviceID deviceID, Word transID, Word gizmoID)
      : BytesSysexCommand(deviceID, transID) {
    data.append(gizmoID);
  }
};

}  // namespace GeneSysLib

#endif  // __GIZMOINFO_H__
