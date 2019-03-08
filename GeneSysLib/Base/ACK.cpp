/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "ACK.h"

namespace GeneSysLib {
ACK::ACK(void) : commandID(), errorCode() {}

const commandDataKey_t ACK::key() const { return generateKey(Command::ACK); }

Bytes ACK::generate() const {
  Bytes result;

  appendMidiWord(result, static_cast<Word>(commandID()));
  result += static_cast<Byte>(errorCode()) & 0x7F;

  return result;
}

void ACK::parse(BytesIter &beginIter, BytesIter &endIter) {
  commandID = roCmdEnum(static_cast<CmdEnum>(nextMidiWord(beginIter, endIter)));
  errorCode = roErrorCodeEnum(
      static_cast<ErrorCodeEnum>(nextMidiByte(beginIter, endIter)));
}
}  // namespace GeneSysLib
