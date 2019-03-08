/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __SAVERESTORELIST_H__
#define __SAVERESTORELIST_H__

#include "LibTypes.h"
#include "SaveRestoreList.h"
#include "SaveRestoreID.h"
#include "SysexCommand.h"
#include "CommandDataKey.h"

namespace GeneSysLib {

struct SaveRestoreList {
  static CmdEnum retCommand();

  SaveRestoreList(void);

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &begin, BytesIter &end);

  bool contains(SaveRestoreIDEnum saveRestore) const;

 private:
  std::vector<SaveRestoreIDEnum> m_saveRestoreList;
};  // struct SaveRestoreList

typedef SysexCommand<Command::GetSaveRestoreList, EmptyCommandData>
    GetSaveRestoreListCommand;

}  // namespace GeneSysLib

#endif  // __SAVERESTORELIST_H__
