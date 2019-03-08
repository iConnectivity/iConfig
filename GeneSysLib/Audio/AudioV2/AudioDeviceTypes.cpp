/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "AudioDeviceTypes.h"
#include "StreamHelpers.h"
#include <string>

namespace GeneSysLib {

namespace AudioDeviceTypes {

USBDevice USBDevice::parse(BytesIter &beginIter, const BytesIter &endIter) {
  USBDevice device;
  device.hostType = roHostTypeEnum(
      static_cast<HostTypeEnum>(nextMidiByte(beginIter, endIter)));
  auto length = nextMidiByte(beginIter, endIter);
  device.hostName = nextROString(length, beginIter, endIter);

  return device;
}

////////////////////////////////////////////////////////////////////////////////

USBHost USBHost::parse(BytesIter &beginIter, const BytesIter &endIter) {
  using namespace std;
  USBHost host;

  host.flags = std::bitset<8>(nextMidiByte(beginIter, endIter));
  host.maxInChannels = nextROByte(beginIter, endIter);
  host.maxOutChannels = nextROByte(beginIter, endIter);

  auto inCount = nextMidiByte(beginIter, endIter);
  host.inChannelMap.clear();
  for (auto i = 0; i < inCount; ++i) {
    auto src = nextMidiByte(beginIter, endIter);
    auto dst = nextMidiByte(beginIter, endIter);
    host.inChannelMap[src] = dst;
  }

  auto outCount = nextMidiByte(beginIter, endIter);
  host.outChannelMap.clear();
  for (auto i = 0; i < outCount; ++i) {
    auto src = nextMidiByte(beginIter, endIter);
    auto dst = nextMidiByte(beginIter, endIter);
    host.outChannelMap[src] = dst;
  }

  host.vendorID = nextROWord3Byte(beginIter, endIter);
  host.productID = nextROWord3Byte(beginIter, endIter);

  auto vendorNameLength = nextMidiByte(beginIter, endIter);
  host.vendorName = nextROString(vendorNameLength, beginIter, endIter);

  auto productNameLength = nextMidiByte(beginIter, endIter);
  host.productName = nextROString(productNameLength, beginIter, endIter);

  return host;
}

bool USBHost::connected() const { return flags[connectedFlag]; }

bool USBHost::reserved() const { return flags[isReservedFlag]; }

void USBHost::reserved(bool value) { flags[isReservedFlag] = value; }

////////////////////////////////////////////////////////////////////////////////

Ethernet Ethernet::parse(BytesIter &, const BytesIter &) { return Ethernet(); }

////////////////////////////////////////////////////////////////////////////////

Analogue Analogue::parse(BytesIter &, const BytesIter &) { return Analogue(); }

////////////////////////////////////////////////////////////////////////////////

Bytes generateVisitor::operator()(const USBDevice &usbDevice) const {
  using namespace std;
  Bytes result;
  appendMidiByte(result, static_cast<Byte>(usbDevice.hostType()));
  appendMidiByte(result, static_cast<Byte>(usbDevice.hostName().size() & 0x7F));
  appendString(result, usbDevice.hostName());
  return result;
}

Bytes generateVisitor::operator()(const USBHost &usbHost) const {
  Bytes result;

  appendMidiByte(result, static_cast<Byte>(usbHost.flags.to_ulong() & 0x7F));
  appendMidiByte(result, usbHost.maxInChannels());
  appendMidiByte(result, usbHost.maxOutChannels());
  appendMidiByte(result, static_cast<Byte>(usbHost.inChannelMap.size() & 0x7F));
  for (const auto &chMap : usbHost.inChannelMap) {
    appendMidiByte(result, chMap.first);
    appendMidiByte(result, chMap.second);
  }

  appendMidiByte(result,
                 static_cast<Byte>(usbHost.outChannelMap.size() & 0x7F));
  for (const auto &chMap : usbHost.outChannelMap) {
    appendMidiByte(result, chMap.first);
    appendMidiByte(result, chMap.second);
  }

  appendMidiWord3Byte(result, usbHost.vendorID());
  appendMidiWord3Byte(result, usbHost.productID());

  appendMidiByte(result, usbHost.vendorName().size() & 0x7F);
  appendString(result, usbHost.vendorName());

  appendMidiByte(result, usbHost.productName().size() & 0x7F);
  appendString(result, usbHost.productName());
  return result;
}

Bytes generateVisitor::operator()(const Ethernet &) const {
  Bytes result;
  return result;
}

Bytes generateVisitor::operator()(const Analogue &) const {
  Bytes result;
  return result;
}

}  // namespace AudioDeviceTypes

}  // namespace GeneSysLib
