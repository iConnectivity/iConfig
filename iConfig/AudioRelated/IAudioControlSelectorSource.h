/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __IAUDIOCONTROLSELECTORSOURCE_H__
#define __IAUDIOCONTROLSELECTORSOURCE_H__

#include "LibTypes.h"

/* This is a interface that can be used to help develop the audio controls
 * views. It is a placeholder currently until there are selectors implemented
 * in the firmware.
 */
struct IAudioControlSelectorSource {

  virtual Byte currentInput() const = 0;
  virtual void currentInput(Byte value) = 0;

  virtual Byte numSelectors() const = 0;

  virtual std::string controllerName() const = 0;
  virtual std::string selectorName(Byte selectorID) const = 0;

};  // IAudioControlSelectorSource

#endif  // __IAUDIOCONTROLSELECTORSOURCE_H__
