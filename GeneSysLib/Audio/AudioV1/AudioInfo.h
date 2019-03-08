/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __AUDIOINFO_H__
#define __AUDIOINFO_H__

#include "CommandDataKey.h"
#include "LibTypes.h"
#include "SysexCommand.h"
#include "property.h"

namespace GeneSysLib {

struct AudioInfo {
  static CmdEnum retCommand();

  AudioInfo(void);

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &begin, BytesIter &end);

  // variables
  Byte versionNumber() const;
  roWord numberOfAudioPorts;
  roByte numberOfUSBDeviceJacks;
  roByte numberOfUSBHostJacks;
  roByte numberOfEthernetJacks;
  roByte numberOfPortsPerUSBHostJack;
  roByte numberOfPortsPerEthernetJack;
};  // struct AudioInfo

typedef SysexCommand<Command::GetAudioInfo, EmptyCommandData>
    GetAudioInfoCommand;

}  // namespace GeneSysLib

#endif  // __AUDIOINFO_H__
