/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __FILTERID_H__
#define __FILTERID_H__

#include "LibTypes.h"
#include "property.h"

namespace GeneSysLib {

namespace FilterID {
typedef enum Enum {
  InputFilter = 0x01,
  OutputFilter = 0x02,
  Unknown = 0xFF
} Enum;
}  // namespace FilterID
typedef FilterID::Enum FilterIDEnum;
typedef readonly_property<FilterIDEnum> roFilterIDEnum;
typedef readwrite_property<FilterIDEnum> rwFilterIDEnum;

}  // namespace GeneSysLib

#endif  // __FILTERID_H__
