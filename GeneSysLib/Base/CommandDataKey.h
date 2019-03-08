/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __COMMANDDATAKEY_H__
#define __COMMANDDATAKEY_H__

#include "LibTypes.h"
#include "CommandDefines.h"
#include "StreamHelpers.h"

namespace GeneSysLib {
typedef Bytes commandDataKey_t;

template <typename T>
inline Bytes &appendBytes(Bytes &result, T t) {
  result.push_back(static_cast<Byte>(t & 0xFF));
  return result;
}

template <>
inline Bytes &appendBytes<Byte>(Bytes &result, Byte byte) {
  result.push_back(byte);
  return result;
}

template <>
inline Bytes &appendBytes<Word>(Bytes &result, Word word) {
  appendMidiWord(result, word);
  return result;
}

inline commandDataKey_t generateKeyHelper(Bytes &result) { return result; }

template <typename T, typename... Ts>
inline commandDataKey_t generateKeyHelper(Bytes &result, T t, Ts... ts) {
  appendBytes(result, t);
  return generateKeyHelper(result, ts...);
}

template <typename... Ts>
inline commandDataKey_t generateKey(Command::Enum cmd, Ts... ts) {
  Bytes result;
  appendBytes(result, static_cast<Byte>(cmd & 0x7F));
  return generateKeyHelper(result, ts...);
}

inline Command::Enum keyToCommand(const commandDataKey_t &key) {
  assert(key.size() > 0);
  return static_cast<Command::Enum>(ANSWR_BIT | key.at(0));
}

inline Byte keyToCommandID(const commandDataKey_t &key) {
  assert(key.size() > 0);
  return key.at(0);
}

}  // namespace GeneSysLib
#endif  // __COMMANDDATAKEY_H__
