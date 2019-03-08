/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __STREAMHELPERS_H__
#define __STREAMHELPERS_H__

#include "LibTypes.h"
#ifndef Q_MOC_RUN
#include <boost/assign.hpp>
#endif
#include "property.h"

using namespace boost::assign;
namespace GeneSysLib {

inline Byte nextMidiByte(BytesIter &beginIter, const BytesIter &endIter) {
  return ((beginIter != endIter) ? *beginIter++ : 0x00);
}

inline roByte nextROByte(BytesIter &beginIter, const BytesIter &endIter) {
  return roByte(((beginIter != endIter) ? *beginIter++ : 0x00));
}

inline rwByte nextRWByte(BytesIter &beginIter, BytesIter endIter) {
  return rwByte(((beginIter != endIter) ? *beginIter++ : 0x00));
}

inline void appendMidiWord(Bytes &buffer, Word wd) {
  buffer += ((wd >> 7) & 0x7F);
  buffer += ((wd) & 0x7F);
}

inline void appendMidiByte(Bytes &buffer, Byte b) { buffer += (b & 0x7F); }

inline Word nextMidiWord(BytesIter &begin, const BytesIter &end) {
  Word wd = 0x00;

  if (begin != end) {
    wd = (*begin << 7) & 0x3F80;
    ++begin;
  }

  if (begin != end) {
    wd |= *begin & 0x7F;
    ++begin;
  }
  return wd;
}

inline rwWord nextRWWord(BytesIter &beginIter, BytesIter const &endIter) {
  return rwWord(nextMidiWord(beginIter, endIter));
}

inline roWord nextROWord(BytesIter &beginIter, BytesIter const &endIter) {
  return roWord(nextMidiWord(beginIter, endIter));
}

inline Word fromMidiWordIn3Bytes(BytesIter &beginIter,
                                 const BytesIter &endIter) {
  Word wd = 0x00;
  if (beginIter != endIter) {
    wd = *beginIter;
    ++beginIter;
  }
  if (beginIter != endIter) {
    wd = (wd << 7) | (*beginIter & 0x7F);
    ++beginIter;
  }

  if (beginIter != endIter) {
    wd = (wd << 7) | (*beginIter & 0x7F);
    ++beginIter;
  }

  return wd;
}

inline roWord nextROWord3Byte(BytesIter &beginIter, const BytesIter &endIter) {
  return roWord(fromMidiWordIn3Bytes(beginIter, endIter));
}

inline rwWord nextRWWord3Byte(BytesIter &beginIter, const BytesIter &endIter) {
  return rwWord(fromMidiWordIn3Bytes(beginIter, endIter));
}

inline void appendMidiWord3Byte(Bytes &result, Word wd) {
  result += (wd >> 14) & 0x7F;
  result += (wd >> 7) & 0x7F;
  result += wd & 0x7F;
}

inline void appendString(Bytes &result, const std::string &str) {
  using namespace std;
  copy(str.begin(), str.end(), back_inserter(result));
}

inline void appendBytes(Bytes &result, const Bytes &rhs) {
  using namespace std;
  copy(rhs.begin(), rhs.end(), back_inserter(result));
}

inline std::string nextMidiString(Byte length, BytesIter &beginIter,
                                  const BytesIter &endIter) {
  using namespace std;
  string result;
  if (distance(beginIter, endIter) >= length) {
    result = string(beginIter, (beginIter + length));
    advance(beginIter, length);
  }
  return result;
}

inline roString nextROString(Byte length, BytesIter &beginIter,
                             const BytesIter &endIter) {
  return roString(nextMidiString(length, beginIter, endIter));
}

inline rwString nextRWString(Byte length, BytesIter &beginIter,
                             const BytesIter &endIter) {
  return rwString(nextMidiString(length, beginIter, endIter));
}

}  // namespace GeneSysLib

#endif  // __STREAMHELPERS_H__
