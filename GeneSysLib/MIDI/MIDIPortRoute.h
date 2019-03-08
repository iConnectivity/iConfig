/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __MIDIPORTROUTE_H__
#define __MIDIPORTROUTE_H__

#include "LibTypes.h"
#include "SysexCommand.h"
#include "BytesCommandData.h"
#include "CommandDataKey.h"
#include "property.h"

namespace GeneSysLib {

struct MIDIPortRoute {
  static commandDataKey_t minKey();
  static commandDataKey_t maxKey();
  static commandDataKey_t queryKey(Word portID);
  static CmdEnum retCommand();
  static CmdEnum setCommand();

  MIDIPortRoute(void);
  MIDIPortRoute(Word portID, Bytes portRouting);

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &begin, BytesIter &end);

  // properties
  Byte versionNumber() const;
  roWord portID;
  bool isRoutedTo(Byte destPortID) const;
  void setRoutedTo(Byte destPortID, bool value);

 private:
  Bytes m_portRouting;
};  // struct MIDIPortRoute

struct GetMIDIPortRouteCommand
    : public BytesSysexCommand<Command::GetMIDIPortRoute> {
  GetMIDIPortRouteCommand(DeviceID deviceID, Word transID, Word portID)
      : BytesSysexCommand(deviceID, transID) {
    data.append(portID);
  }
};

typedef SysexCommand<Command::SetMIDIPortRoute, MIDIPortRoute>
    SetMIDIPortRouteCommand;

}  // namespace GeneSysLib

#endif  // __MIDIPORTROUTE_H__
