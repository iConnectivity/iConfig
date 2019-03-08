/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef RESETLIST_H
#define RESETLIST_H

#include "SysexCommand.h"
#include "BytesCommandData.h"

namespace GeneSysLib {

typedef BytesCommandData<Command::RetResetList> ResetList;

typedef SysexCommand<Command::GetResetList, EmptyCommandData>
    GetResetListCommand;

}  // namespace GeneSysLib

#endif  // RESETLIST_H
