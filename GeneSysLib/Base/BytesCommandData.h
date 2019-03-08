/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __BYTESCOMMANDDATA_H__
#define __BYTESCOMMANDDATA_H__

#include "CommandDataKey.h"
#include "LibTypes.h"
#include "MyAlgorithms.h"
#include "SysexCommand.h"
#include "StreamHelpers.h"

namespace GeneSysLib {

template <CmdEnum CMD>
struct BytesCommandData {
  static CmdEnum retCommand() { return CMD; }

  BytesCommandData(void) : data() {}
  BytesCommandData(Bytes _data) : data(_data) {}
  BytesCommandData(Word midiWord) : data() { appendMidiWord(data, midiWord); }

  // overloaded methods
  const commandDataKey_t key() const { return generateKey(CMD); }
  Bytes generate() const { return data; }
  void parse(BytesIter &begin, BytesIter &end) {
    data.clear();
    copy(begin, end, std::back_inserter(data));
  }

  // append methods
  void append(const Byte &midiByte) { appendMidiByte(data, midiByte); }
  void append(const Word &midiWord) { appendMidiWord(data, midiWord); }
  void append(const std::string &str) { appendString(data, str); }

  bool contains(const Byte &b) const { return MyAlgorithms::contains(data, b); }

  template <typename T>
  bool contains(const T &b) const {
    return contains(static_cast<Byte>(b));
  }

  // data
  Bytes data;
};  // struct BytesCommandData

template <CmdEnum CMD>
struct BytesSysexCommand : SysexCommand<CMD, BytesCommandData<CMD> > {
  BytesSysexCommand(DeviceID deviceID, Word transID)
      : SysexCommand<CMD, BytesCommandData<CMD> >(deviceID, transID) {}
};

}  // namespace GeneSysLib

#endif  // __BYTESCOMMANDDATA_H__
