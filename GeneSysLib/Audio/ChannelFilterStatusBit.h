/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef CHANNELFILTERSTATUSBIT_H
#define CHANNELFILTERSTATUSBIT_H

#include "LibTypes.h"

namespace GeneSysLib {

namespace ChannelFilterStatusBit {
typedef enum Enum {
  pitchBendEvents = 5,
  channelPressureEvents = 4,
  programChangeEvents = 3,
  controlChangeEvents = 2,
  polyKeyPressureEvents = 1,  // monotouch
  noteEvents = 0
} Enum;
}  // namespace ChannelFilterStatusBit

typedef ChannelFilterStatusBit::Enum ChannelFilterStatusBitEnum;

}  // namespace GeneSysLib

#endif  // CHANNELFILTERSTATUSBIT_H
