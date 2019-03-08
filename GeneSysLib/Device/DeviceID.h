/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef DEVICEID_H
#define DEVICEID_H

#include "LibTypes.h"

namespace GeneSysLib {

struct DeviceID {
  DeviceID();
  DeviceID(const Word &pid);
  DeviceID(const Word &pid, const SerialNumber &serialNumber);
  DeviceID(const DeviceID &other);

  DeviceID &operator=(const DeviceID &other);

  Word pid() const;
  SerialNumber serialNumber() const;

 private:
  Word m_pid;
  SerialNumber m_serialNumber;
};  // struct DeviceID

bool operator==(const DeviceID &a, const DeviceID &b);
bool operator==(const DeviceID &a, Word &b);
bool operator==(Word &a, const DeviceID &b);

}  // namespace GeneSysLib

#endif  // DEVICEID_H
