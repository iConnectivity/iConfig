/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __ACK_H__
#define __ACK_H__

#include "LibTypes.h"
#include "ErrorCode.h"
#include "CommandData.h"
#include "CommandDataKey.h"
#include "CommandDefines.h"
#include "property.h"

namespace GeneSysLib {
struct ACK {
  // constructors
  ACK(void);

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &beginIter, BytesIter &endIter);

  // properties
  roCmdEnum commandID;
  roErrorCodeEnum errorCode;
};
}  // namespace GeneSysLib

#endif  // __ACK_H__
