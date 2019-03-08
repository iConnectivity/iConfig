/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "SaveRestoreList.h"
#include "stdafx.h"

#include "MyAlgorithms.h"

namespace GeneSysLib {

CmdEnum SaveRestoreList::retCommand() { return Command::RetSaveRestoreList; }

SaveRestoreList::SaveRestoreList(void) {}

const commandDataKey_t SaveRestoreList::key() const {
  return generateKey(Command::RetSaveRestoreList);
}

Bytes SaveRestoreList::generate() const {
  Bytes result;

  for (const auto &saveRestore : m_saveRestoreList) {
    result += static_cast<Byte>(saveRestore);
  }

  return result;
}

void SaveRestoreList::parse(BytesIter &beginIter, BytesIter &endIter) {
  m_saveRestoreList.clear();
  for (auto iter = beginIter; iter != endIter; ++iter) {
    m_saveRestoreList += static_cast<SaveRestoreIDEnum>(*iter);
  }
}

bool SaveRestoreList::contains(SaveRestoreIDEnum saveRestore) const {
  return MyAlgorithms::contains(m_saveRestoreList, saveRestore);
}

}  // namespace GeneSysLib
