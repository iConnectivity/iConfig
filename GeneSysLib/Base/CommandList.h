/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __COMMANDLIST_H__
#define __COMMANDLIST_H__

#include "LibTypes.h"
#include "SysexCommand.h"
#include "CommandDataKey.h"

namespace GeneSysLib {

struct DeviceID;

struct CommandList {
  static CmdEnum retCommand();

  CommandList(void);
  CommandList(CmdEnumVector cmdVector);

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &begin, BytesIter &end);

  bool contains(CmdEnum command) const;

  // properties
  CmdEnumVector commandList;
};  // struct CommandList

typedef SysexCommand<Command::GetCommandList, EmptyCommandData>
    GetCommandListCommand;

}  // namespace GeneSysLib

#endif  // __COMMANDLIST_H__
