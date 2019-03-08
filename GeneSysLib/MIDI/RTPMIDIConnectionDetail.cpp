/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "RTPMIDIConnectionDetail.h"
#include "Generator.h"

#include <limits>

using namespace boost::assign;
using namespace std;

namespace GeneSysLib {

commandDataKey_t RTPMIDIConnectionDetail::minKey() {
  return generateKey(Command::RetRTPMIDIConnectionDetail);
}

commandDataKey_t RTPMIDIConnectionDetail::maxKey() {
  return RTPMIDIConnectionDetail::queryKey(std::numeric_limits<Word>::max());
}

commandDataKey_t RTPMIDIConnectionDetail::queryKey(Word portID) {
  return generateKey(Command::RetRTPMIDIConnectionDetail, portID);
}

CmdEnum RTPMIDIConnectionDetail::retCommand() {
  return Command::RetRTPMIDIConnectionDetail;
}

RTPMIDIConnectionDetail::RTPMIDIConnectionDetail(void)
    : portID(),
      rtpMIDIConnectionNumber(),
      connectedIPAddress(),
      rtpMIDIPortNumber(),
      sessionName() {}

const commandDataKey_t RTPMIDIConnectionDetail::key() const {
  return generateKey(Command::RetRTPMIDIConnectionDetail, portID());
}

Bytes RTPMIDIConnectionDetail::generate() const {
  auto addAddr = [](NetAddr addr, Bytes &output) {
    output += (addr[0] >> 4) & 0x7F;
    output += ((addr[0] << 3) | (addr[1] >> 5)) & 0x7F;
    output += ((addr[1] << 2) | (addr[2] >> 6)) & 0x7F;
    output += ((addr[2] << 1) | (addr[3] >> 7)) & 0x7F;
    output += addr[3] & 0x7F;
  };
  Bytes result;

  result += versionNumber() & 0x7F;
  appendMidiWord(result, portID());
  result += rtpMIDIConnectionNumber();
  addAddr(connectedIPAddress(), result);

  result += (rtpMIDIPortNumber() >> 14) & 0x7F;
  result += (rtpMIDIPortNumber() >> 7) & 0x7F;
  result += (rtpMIDIPortNumber()) & 0x7F;

  result += sessionName().size() & 0x7F;

  copy(sessionName().begin(), sessionName().end(), std::back_inserter(result));

  return result;
}

void RTPMIDIConnectionDetail::parse(BytesIter &beginIter, BytesIter &endIter) {
  auto addrFromStream = [](BytesIter &b, BytesIter &e)->NetAddr {
    Byte t0, t1, t2, t3, t4;
    NetAddr addr;

    t0 = nextMidiByte(b, e);
    t1 = nextMidiByte(b, e);
    t2 = nextMidiByte(b, e);
    t3 = nextMidiByte(b, e);
    t4 = nextMidiByte(b, e);

    addr[0] = ((t1 >> 3) | (t0 << 4));
    addr[1] = ((t2 >> 2) | (t1 << 5));
    addr[2] = ((t3 >> 1) | (t2 << 6));
    addr[3] = ((t4) | (t3 << 7));

    return addr;
  };

  auto version = nextMidiByte(beginIter, endIter);
  if (version == versionNumber()) {
    portID = nextROWord(beginIter, endIter);
    rtpMIDIConnectionNumber = nextROByte(beginIter, endIter);
    connectedIPAddress = roNetAddr(addrFromStream(beginIter, endIter));
    rtpMIDIPortNumber = roWord(fromMidiWordIn3Bytes(beginIter, endIter));

    Byte length = nextMidiByte(beginIter, endIter);

    sessionName = roString(string(beginIter, (beginIter + length)));
  }
}

Byte RTPMIDIConnectionDetail::versionNumber() const { return 0x01; }

}  // namespace GeneSysLib
