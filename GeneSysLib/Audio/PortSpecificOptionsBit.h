/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __PORTSPECIFICOPTIONSBIT_H__
#define __PORTSPECIFICOPTIONSBIT_H__

namespace GeneSysLib {

namespace PortSpecificOptionsBit {
typedef enum Enum {
  enabledAudioForiOS = 3,
  enabledAudioForPC = 2,
  portSupportsiOSAudio = 1,
  portSupportsPCAudio = 0,
  Unknown = 0xFF
} Enum;
}  // namespace PortSpecificOptionsBit

}  // namespace GeneSysLib

#endif  // __PORTSPECIFICOPTIONSBIT_H__
