/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "USBHostMIDIDeviceDetail.h"
#include "Generator.h"

#include <limits>

using namespace std;
using namespace boost;

namespace GeneSysLib {

commandDataKey_t USBHostMIDIDeviceDetail::minKey() {
  return generateKey(Command::RetUSBHostMIDIDeviceDetail);
}

commandDataKey_t USBHostMIDIDeviceDetail::maxKey() {
  return USBHostMIDIDeviceDetail::queryKey(std::numeric_limits<Byte>::max(),
                                           std::numeric_limits<Byte>::max());
}

commandDataKey_t USBHostMIDIDeviceDetail::queryKey(Byte usbHostJack,
                                                   Byte usbHostID) {
  return generateKey(Command::RetUSBHostMIDIDeviceDetail, usbHostJack,
                     usbHostID);
}

CmdEnum USBHostMIDIDeviceDetail::retCommand() {
  return Command::RetUSBHostMIDIDeviceDetail;
}

USBHostMIDIDeviceDetail::USBHostMIDIDeviceDetail(void)
    : usbHostJack(),
      usbHostID(),
      numMIDIIn(),
      numMIDIOut(),
      hostedUSBVendorID(),
      hostedUSBProductID(),
      vendorName(),
      productName() {}

const commandDataKey_t USBHostMIDIDeviceDetail::key() const {
  return generateKey(Command::RetUSBHostMIDIDeviceDetail, usbHostJack(),
                     usbHostID());
}

Bytes USBHostMIDIDeviceDetail::generate() const {
  Bytes result;

  result += versionNumber() & 0x7F;
  result += usbHostJack();
  result += usbHostID();

  result += numMIDIIn();
  result += numMIDIOut();

  result += (hostedUSBVendorID() >> 14) & 0x7F;
  result += (hostedUSBVendorID() >> 7) & 0x7F;
  result += (hostedUSBVendorID()) & 0x7F;

  result += (hostedUSBProductID() >> 14) & 0x7F;
  result += (hostedUSBProductID() >> 7) & 0x7F;
  result += (hostedUSBProductID()) & 0x7F;

  result += (Byte)(vendorName().size() & 0x7F);
  copy(vendorName().begin(), vendorName().end(), std::back_inserter(result));

  result += (Byte)(productName().size() & 0x7F);
  copy(productName().begin(), productName().end(), std::back_inserter(result));

  return result;
}

void USBHostMIDIDeviceDetail::parse(BytesIter& beginIter, BytesIter& endIter) {
  auto version = nextMidiByte(beginIter, endIter);

  if (version == versionNumber()) {
    usbHostJack = nextROByte(beginIter, endIter);
    usbHostID = nextROByte(beginIter, endIter);

    numMIDIIn = nextROByte(beginIter, endIter);
    numMIDIOut = nextROByte(beginIter, endIter);

    Word tmpVendorID = nextMidiByte(beginIter, endIter);
    tmpVendorID = ((tmpVendorID << 7) | (nextMidiByte(beginIter, endIter)));
    tmpVendorID = ((tmpVendorID << 7) | (nextMidiByte(beginIter, endIter)));
    hostedUSBVendorID = roWord(tmpVendorID);

    Word tmpProductID = nextMidiByte(beginIter, endIter);
    tmpProductID = ((tmpProductID << 7) | (nextMidiByte(beginIter, endIter)));
    tmpProductID = ((tmpProductID << 7) | (nextMidiByte(beginIter, endIter)));
    hostedUSBProductID = roWord(tmpProductID);

    Byte length = nextMidiByte(beginIter, endIter);
    if (length > 0) {
      vendorName = roString(string(beginIter, (beginIter + length)));
      advance(beginIter, length);
    }

    length = nextMidiByte(beginIter, endIter);
    if (length > 0) {
      productName = roString(string(beginIter, (beginIter + length)));
      advance(beginIter, length);
    }
  }
}

Byte USBHostMIDIDeviceDetail::versionNumber() const { return 0x01; }

}  // namespace GeneSysLib
