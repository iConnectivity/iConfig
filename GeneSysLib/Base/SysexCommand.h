/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __SYSEXCOMMAND_H__
#define __SYSEXCOMMAND_H__

#include "LibTypes.h"
#include "Communicator.h"

namespace GeneSysLib {

template <CmdEnum CMD, class DATA_T>
struct SysexCommand {
  SysexCommand(DeviceID _deviceID, Word _transID)
      : deviceID(_deviceID), transID(_transID), data() {}
  SysexCommand(DeviceID _deviceID, Word _transID, const DATA_T &_data)
      : deviceID(_deviceID), transID(_transID), data(_data) {}

  Bytes sysex() const { return generate(deviceID, transID, CMD, data); }

  void send(const CommPtr &comm) const {
    comm->sendSysex(sysex());
  }

 protected:
  const DeviceID deviceID;
  const Word transID;
  DATA_T data;
};

template <class T>
Bytes sysex(const T &x) {
  return x.sysex();
}

template <class DATA_T>
void send(const DATA_T &x, const CommPtr &comm) {
  x.send(comm);
}

}  // namespace GeneSysLib

#endif  // __SYSEXCOMMAND_H__
