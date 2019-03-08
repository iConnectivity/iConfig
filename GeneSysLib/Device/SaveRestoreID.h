/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef SAVERESTOREID_H
#define SAVERESTOREID_H

namespace GeneSysLib {

namespace SaveRestoreID {
typedef enum Enum {
  SaveToFlash = 0x01,
  RestoreFromFlash = 0x02,
  FactoryDefault = 0x03
} Enum;
}  // namespace SaveRestoreID
typedef SaveRestoreID::Enum SaveRestoreIDEnum;

}  // namespace GeneSysLib

#endif  // SAVERESTOREID_H
