/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef BOOTMODE_H
#define BOOTMODE_H

namespace GeneSysLib {

namespace BootMode {
typedef enum Enum {
  AppMode = 0x01,
  BootLoaderMode = 0x02,
  TestMode = 0x03
} Enum;
}  // namespace BootMode
typedef BootMode::Enum BootModeEnum;

}  // namespace GeneSysLib

#endif  // BOOTMODE_H
