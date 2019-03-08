/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef DEVICEPID_H
#define DEVICEPID_H

namespace GeneSysLib {

namespace DevicePID {
typedef enum Enum {
  MIO10 = 0x0002,
  MIO = 0x0003,
  iConnect1 = 0x0004,
  iConnect2Plus = 0x0005,
  iConnect4Plus = 0x0006,
  iConnect4Audio = 0x0007,
  iConnect2Audio = 0x0008,
  MIO2 = 0x0009,
  MIO4 = 10,
  PlayAudio12 = 11,
  //xxx = 12,
  ConnectAudio24 = 13
} Enum;
}  // namespace DevicePID

}  // namespace GeneSysLib

#endif  // DEVICEPID_H
