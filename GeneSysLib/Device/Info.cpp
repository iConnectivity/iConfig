/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "Info.h"
#include "Generator.h"
#ifndef Q_MOC_RUN
#include <boost/range.hpp>
#endif
#include <string>

using namespace boost;
using namespace std;

namespace GeneSysLib {

commandDataKey_t Info::minKey() { return generateKey(Command::RetInfo); }

commandDataKey_t Info::maxKey() { return Info::queryKey(InfoID::Unknown); }

commandDataKey_t Info::queryKey(InfoIDEnum infoID) {
  return generateKey(Command::RetInfo, infoID);
}

CmdEnum Info::retCommand() { return Command::RetInfo; }

CmdEnum Info::setCommand() { return Command::SetInfo; }

Info::Info(void) : infoID(), infoString() {}

Info::Info(InfoIDEnum _infoID, const std::string &_infoString)
    : infoID(_infoID), infoString(_infoString) {}

const commandDataKey_t Info::key() const {
  return generateKey(Command::RetInfo, infoID());
}

Bytes Info::generate() const {
  Bytes result;
  result += static_cast<Byte>(infoID());
  copy(infoString().begin(), infoString().end(), std::back_inserter(result));
  return result;
}

void Info::parse(BytesIter &beginIter, BytesIter &endIter) {
  infoID =
      roInfoIDEnum(static_cast<InfoIDEnum>(nextMidiByte(beginIter, endIter)));
  infoString = rwString(string(beginIter, endIter));
}

}  // namespace GeneSysLib
