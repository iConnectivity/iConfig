/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "EthernetPortInfo.h"
#include "Generator.h"

#include <cassert>
#include <limits>

using namespace std;
using namespace boost::assign;

namespace GeneSysLib {

commandDataKey_t EthernetPortInfo::minKey() {
  return generateKey(Command::RetEthernetPortInfo);
}

commandDataKey_t EthernetPortInfo::maxKey() {
  return EthernetPortInfo::queryKey(std::numeric_limits<Word>::max());
}

commandDataKey_t EthernetPortInfo::queryKey(Word portID) {
  return generateKey(Command::RetEthernetPortInfo, portID);
}

CmdEnum EthernetPortInfo::retCommand() { return Command::RetEthernetPortInfo; }

CmdEnum EthernetPortInfo::setCommand() { return Command::SetEthernetPortInfo; }

EthernetPortInfo::EthernetPortInfo(void)
    : portJackID(),
      ipMode(),
      staticIPAddress(),
      staticSubnetMask(),
      staticGateway(),
      currentIPAddress(),
      currentSubnetMask(),
      currentGateway(),
      deviceName() {}

const commandDataKey_t EthernetPortInfo::key() const {
  return generateKey(Command::RetEthernetPortInfo, portJackID());
}

Bytes EthernetPortInfo::generate() const {
  auto addAddr = [](NetAddr addr, Bytes &output) {
    output += (addr[0] >> 4) & 0x7F;
    output += ((addr[0] << 3) | (addr[1] >> 5)) & 0x7F;
    output += ((addr[1] << 2) | (addr[2] >> 6)) & 0x7F;
    output += ((addr[2] << 1) | (addr[3] >> 7)) & 0x7F;
    output += addr[3] & 0x7F;
  };
  Bytes result;

  result += versionNumber();
  appendMidiWord(result, portJackID());
  result += static_cast<Byte>(ipMode());

  addAddr(staticIPAddress(), result);
  addAddr(staticSubnetMask(), result);
  addAddr(staticGateway(), result);

  addAddr(currentIPAddress(), result);
  addAddr(currentSubnetMask(), result);
  addAddr(currentGateway(), result);

  copy(macAddress().begin(), macAddress().end(), back_inserter(result));
  result += (Byte)(deviceName().size() & 0x7F);
  copy(deviceName().begin(), deviceName().end(), back_inserter(result));

  return result;
}

void EthernetPortInfo::parse(BytesIter &beginIter, BytesIter &endIter) {
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
    portJackID = roWord(nextMidiWord(beginIter, endIter));
    ipMode =
        rwIPModeEnum(static_cast<IPModeEnum>(nextMidiByte(beginIter, endIter)));

    staticIPAddress = rwNetAddr(addrFromStream(beginIter, endIter));
    staticSubnetMask = rwNetAddr(addrFromStream(beginIter, endIter));
    staticGateway = rwNetAddr(addrFromStream(beginIter, endIter));

    currentIPAddress = roNetAddr(addrFromStream(beginIter, endIter));
    currentSubnetMask = roNetAddr(addrFromStream(beginIter, endIter));
    currentGateway = roNetAddr(addrFromStream(beginIter, endIter));

    // for compatibility with older demo units this check can't be asserted
    if (distance(beginIter, endIter) >= 12) {
      macAddress = roString(string(beginIter, beginIter + 12));
      advance(beginIter, 12);
    }

    size_t deviceLength = static_cast<size_t>(nextMidiByte(beginIter, endIter));
    deviceName = rwString(string(beginIter, endIter));

    assert(deviceName().size() == deviceLength);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// Properties
////////////////////////////////////////////////////////////////////////////////
Byte EthernetPortInfo::versionNumber() const { return 0x01; }

}  // namespace GeneSysLib
