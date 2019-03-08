/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __ERRORCODE_H__
#define __ERRORCODE_H__

#include "LibTypes.h"
#include "property.h"

namespace GeneSysLib {

namespace ErrorCode {
typedef enum Enum {
  NoError = 0x00,
  UnknownCommand = 0x01,
  MalformedMessage = 0x02,
  CommandFailed = 0x03
} Enum;
}  // namespace ErrorCode
typedef ErrorCode::Enum ErrorCodeEnum;

typedef readonly_property<ErrorCodeEnum> roErrorCodeEnum;
typedef readwrite_property<ErrorCodeEnum> rwErrorCodeEnum;

}  // namespace GeneSysLib

#endif  // __ERRORCODE_H__
