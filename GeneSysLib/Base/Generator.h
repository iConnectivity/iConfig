/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __GENERATOR_H__
#define __GENERATOR_H__

#include "LibTypes.h"
#include "CommandData.h"
#include "DeviceID.h"
#include "BootMode.h"
#include "SaveRestoreID.h"

namespace GeneSysLib {

Bytes generate(DeviceID deviceID, Word transID, CmdEnum commandID,
               const commandData_t &data);

}  // namespace GeneSysLib

// Audio Related

#endif  // __GENERATOR_H__
