/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __PORTMIDIFLAGS_H__
#define __PORTMIDIFLAGS_H__

#include "LibTypes.h"

namespace GeneSysLib {

namespace PortMIDIFlags {

typedef enum Enum {
  InputEnabled = 0x01,
  OutputEnabled = 0x02,
  Unknown = 0xFF
} Enum;

}  // namespace PortMIDIFlags

}  // namespace GeneSysLib

#endif  // __PORTMIDIFLAGS_H__
