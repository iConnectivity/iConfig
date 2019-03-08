/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __USBHOSTMIDIDEVICEDETAIL_H__
#define __USBHOSTMIDIDEVICEDETAIL_H__

#include "LibTypes.h"
#include "SysexCommand.h"
#include "BytesCommandData.h"
#include "CommandDataKey.h"
#include "property.h"

namespace GeneSysLib {

struct USBHostMIDIDeviceDetail {
  static commandDataKey_t minKey();
  static commandDataKey_t maxKey();
  static commandDataKey_t queryKey(Byte usbHostJack, Byte usbHostID);
  static CmdEnum retCommand();

  USBHostMIDIDeviceDetail(void);

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &beginIter, BytesIter &endIter);

  // properties
  Byte versionNumber() const;
  roByte usbHostJack;
  roByte usbHostID;
  roByte numMIDIIn;
  roByte numMIDIOut;
  roWord hostedUSBVendorID;
  roWord hostedUSBProductID;
  roString vendorName;
  roString productName;
};  // struct USBHostMIDIDeviceDetail

struct GetUSBHostMIDIDeviceDetailCommand
    : public BytesSysexCommand<Command::GetUSBHostMIDIDeviceDetail> {
  GetUSBHostMIDIDeviceDetailCommand(DeviceID deviceID, Word transID,
                                    Byte usbHostJack, Byte usbHostID)
      : BytesSysexCommand(deviceID, transID) {
    // data is zero filled in SysexCommand constructor

    // prepare the data package
    data.append(usbHostJack);
    data.append(usbHostID);
  }
};

}  // namespace GeneSysLib

#endif  // __USBHOSTMIDIDEVICEDETAIL_H__
