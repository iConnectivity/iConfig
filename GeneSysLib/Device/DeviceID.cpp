/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "DeviceID.h"

namespace GeneSysLib {
DeviceID::DeviceID() : m_pid(), m_serialNumber() {}

DeviceID::DeviceID(const Word &pid) : m_pid(pid), m_serialNumber() {}

DeviceID::DeviceID(const Word &pid, const SerialNumber &serialNumber)
    : m_pid(pid), m_serialNumber(serialNumber) {}

DeviceID::DeviceID(const DeviceID &other) {
  this->m_pid = other.m_pid;
  this->m_serialNumber = other.m_serialNumber;
}

DeviceID &DeviceID::operator=(const DeviceID &other) {
  this->m_pid = other.m_pid;
  this->m_serialNumber = other.m_serialNumber;
  return *this;
}

Word DeviceID::pid() const { return m_pid; }

SerialNumber DeviceID::serialNumber() const { return m_serialNumber; }

bool operator==(const DeviceID &a, const DeviceID &b) {
  return (a.pid() == b.pid()) && (a.serialNumber() == b.serialNumber());
  //(a.pid() != 0x00);
}

bool operator==(const DeviceID &a, Word &b) { return (a.pid() == b); }

bool operator==(Word &a, const DeviceID &b) { return (b == a); }

}  // namespace GeneSysLib
