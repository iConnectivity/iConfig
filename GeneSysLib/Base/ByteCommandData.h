/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __BYTECOMMANDDATA_H__
#define __BYTECOMMANDDATA_H__

#include "LibTypes.h"
#include "CommandDataKey.h"

namespace GeneSysLib {

template <Command::Enum CMD>
struct ByteCommandData {
  static CmdEnum retCommand() { return CMD; }

  ByteCommandData(void) : data() {}
  ByteCommandData(Byte _data) : data(_data) {}

  // overloaded methods
  const commandDataKey_t key() const { return generateKey(CMD); }
  Bytes generate() const {
    Bytes result;
    result += data;
    return result;
  }
  void parse(BytesIter &begin, BytesIter &end) {
    if (begin != end) data = *begin++;
  }

  Byte data;
};  // struct ByteCommandData

typedef ByteCommandData<Command::GetInfo> GetInfoData;

}  // namespace GeneSysLib

#endif  // __BYTECOMMANDDATA_H__
