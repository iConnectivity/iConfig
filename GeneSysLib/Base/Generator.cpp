/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include <numeric>
#include "Generator.h"
#include "CommandData.h"
#include "Info.h"
#include "ACK.h"

#ifndef Q_MOC_RUN
#include <boost/range.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#endif

using namespace std;
using namespace boost;
using boost::range::push_back;

namespace GeneSysLib {

Bytes generate(DeviceID deviceID, Word transID, CmdEnum commandID,
               const commandData_t &data) {
  Bytes sysex;
  sysex += 0xF0, 0x00, 0x01, 0x73, 0x7E;  // header

  appendMidiWord(sysex, deviceID.pid());
  push_back(sysex, deviceID.serialNumber());
  appendMidiWord(sysex, transID);
  appendMidiWord(sysex, static_cast<Word>(commandID));

  const auto &generatedData = data.generate();
  appendMidiWord(sysex, (Word)(generatedData.size()));
  push_back(sysex, generatedData);

  // checksum
  sysex += (~(accumulate(sysex.begin() + 5, sysex.end(), 0x00)) + 1) & 0x7F;

  sysex += 0xF7;
  return sysex;
}

}  // namespace GeneSysLib
