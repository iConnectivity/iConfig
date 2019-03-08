/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __LIBTYPES_H__
#define __LIBTYPES_H__

#ifndef Q_MOC_RUN
#include <boost/array.hpp>
#include <boost/function.hpp>
#endif

#include <vector>

#include "property.h"

#define kMessageTimeout 2000

typedef unsigned char Byte;
typedef std::vector<Byte> Bytes;
typedef Bytes::iterator BytesIter;

typedef unsigned short Word;
typedef std::vector<Word> Words;
typedef Words::iterator WordsIter;

typedef boost::array<Byte, 5> SerialNumber;

typedef boost::array<Byte, 4> NetAddr;

namespace GeneSysLib {
struct DeviceID;
struct commandData_t;
}  // namespace GeneSysLib

#include "CommandDefines.h"
#include "DeviceID.h"

// function (Command, DeviceID, TransID, commandData_t*)
typedef boost::function<void(GeneSysLib::CmdEnum, GeneSysLib::DeviceID, Word,
                             GeneSysLib::commandData_t)> Handler;

#endif  // __LIBTYPES_H__
