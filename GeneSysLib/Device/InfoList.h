/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __INFOLIST_H__
#define __INFOLIST_H__

#include "LibTypes.h"
#include "InfoID.h"
#include "LibTypes.h"
#include "SysexCommand.h"
#include "CommandDataKey.h"
#ifndef Q_MOC_RUN
#include <boost/function.hpp>
#include <boost/range/adaptors.hpp>
#endif

using boost::adaptors::map_values;

namespace GeneSysLib {

struct InfoList {
  static CmdEnum retCommand();

  struct InfoRecord {
    Bytes generate() const;
    void parse(BytesIter &begin, BytesIter &end);

    roInfoIDEnum infoID;
    roByte maxLength;
  };  // struct InfoRecord

  typedef std::map<InfoIDEnum, InfoRecord> InfoRecords;

  InfoList(void);
  InfoList(InfoRecords records);

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &begin, BytesIter &end);

  bool contains(InfoIDEnum) const;

  // properties
  const InfoRecord &record_at(InfoIDEnum infoID) const;

  template <typename Action>
  void for_each(Action action) const {
    for (const auto &infoRecord : records | map_values) {
      action(infoRecord);
    }
  }

 private:
  InfoRecords records;
};  // struct InfoList

typedef SysexCommand<Command::GetInfoList, EmptyCommandData> GetInfoListCommand;

}  // namespace GeneSysLib

#endif  // __INFOLIST_H__
