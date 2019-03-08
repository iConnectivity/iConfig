/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __DEVICE_H__
#define __DEVICE_H__

#include "LibTypes.h"
#include "DevicePID.h"
#include "SysexCommand.h"
#include "CommandDataKey.h"

namespace GeneSysLib {

struct Device {
  static CmdEnum retCommand();

  Device(void);
  Device(Byte protocol, Byte mode, Word maxLength);

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &begin, BytesIter &end);

  // properties
  Byte protocol() const;
  Byte mode() const;
  Word maxLength() const;

 private:
  Byte m_protocol;
  Byte m_mode;
  Word m_maxLength;
};  // struct Device

typedef SysexCommand<Command::GetDevice, EmptyCommandData> GetDeviceCommand;

}  // namespace GeneSysLib

#endif  // __DEVICE_H__
