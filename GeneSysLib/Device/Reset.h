/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef RESET_H
#define RESET_H

#include "SysexCommand.h"
#include "ByteCommandData.h"
#include "BootMode.h"

namespace GeneSysLib {

struct ResetCommand
    : public SysexCommand<Command::Reset, ByteCommandData<Command::Reset>> {
  ResetCommand(DeviceID deviceID, Word transID, BootModeEnum bootMode)
      : SysexCommand(deviceID, transID, ByteCommandData<Command::Reset>(
                                            static_cast<Byte>(bootMode))) {}
};

}  // namespace GeneSysLib

#endif  // RESET_H
