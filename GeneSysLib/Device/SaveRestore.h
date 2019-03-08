/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __SAVERESTORE_H__
#define __SAVERESTORE_H__

#include "SysexCommand.h"
#include "SaveRestoreID.h"
#include "ByteCommandData.h"

namespace GeneSysLib {

struct SaveRestoreCommand
    : public SysexCommand<Command::SaveRestore,
                          ByteCommandData<Command::SaveRestore>> {
  SaveRestoreCommand(DeviceID deviceID, Word transID,
                     SaveRestoreIDEnum saveRestoreID)
      : SysexCommand(deviceID, transID,
                     ByteCommandData<Command::SaveRestore>(
                         static_cast<SaveRestoreIDEnum>(saveRestoreID))) {}
};

}  // namespace GeneSysLib

#endif  // __SAVERESTORE_H__
