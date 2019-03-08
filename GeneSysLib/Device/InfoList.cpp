/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "InfoList.h"
#include "Generator.h"

#include <algorithm>

#ifndef Q_MOC_RUN
#include <boost/range/algorithm_ext.hpp>
#include <boost/range/algorithm.hpp>
#endif

using boost::range::push_back;

namespace GeneSysLib {

CmdEnum InfoList::retCommand() { return Command::RetInfoList; }

Bytes InfoList::InfoRecord::generate() const {
  Bytes result;
  result += static_cast<Byte>(infoID());
  result += static_cast<Byte>(maxLength());
  return result;
}

void InfoList::InfoRecord::parse(BytesIter &begin, BytesIter &end) {
  infoID = roInfoIDEnum(static_cast<InfoIDEnum>(nextMidiByte(begin, end)));
  maxLength = roByte(nextMidiByte(begin, end));
}

InfoList::InfoList(void) : records() {}

InfoList::InfoList(InfoRecords _records) : records(_records) {}

const commandDataKey_t InfoList::key() const {
  return generateKey(Command::RetInfoList);
}

Bytes InfoList::generate() const {
  Bytes result;
  for (const auto &rec : records | map_values) {
    push_back(result, rec.generate());
  }
  return result;
}

void InfoList::parse(BytesIter &begin, BytesIter &end) {
  records.clear();
  while (begin != end) {
    InfoRecord record;
    record.parse(begin, end);
    records[record.infoID()] = record;
  }
}

bool InfoList::contains(InfoIDEnum infoID) const {
  const auto &recordRange = boost::adaptors::values(records);
  return (find_if(recordRange, [=](InfoList::InfoRecord record) {
            return (record.infoID() == infoID);
          }) != recordRange.end());
}

const InfoList::InfoRecord &InfoList::record_at(InfoIDEnum infoID) const {
  assert(contains(infoID));
  const auto &recordRange = boost::adaptors::values(records);
  return *(find_if(recordRange, [=](InfoList::InfoRecord record) {
    return (record.infoID() == infoID);
  }));
}

}  // namespace GeneSysLib
