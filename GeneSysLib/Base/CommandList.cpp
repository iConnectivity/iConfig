/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "CommandList.h"
#include "Generator.h"
#include "MyAlgorithms.h"
#include <algorithm>

using namespace std;

namespace GeneSysLib {

CmdEnum CommandList::retCommand() { return Command::RetCommandList; }

CommandList::CommandList(void) : commandList() {}

CommandList::CommandList(vector<CmdEnum> _commandList)
    : commandList(_commandList) {}

const commandDataKey_t CommandList::key() const {
  return generateKey(Command::RetCommandList);
}

Bytes CommandList::generate() const {
  Bytes result;
  for (const auto &cmd : commandList) {
    appendMidiWord(result, static_cast<Word>(cmd));
  }
  return result;
}

void CommandList::parse(BytesIter &beginIter, BytesIter &endIter) {
  commandList.clear();

  auto d = std::distance(beginIter, endIter);
  while ((d > 0) && (beginIter != endIter)) {
    commandList += static_cast<CmdEnum>(nextMidiWord(beginIter, endIter));
  }
}

bool CommandList::contains(CmdEnum command) const {
  return MyAlgorithms::contains(commandList, (CmdEnum)(command & 0x7F));
}

}  // namespace GeneSysLib
