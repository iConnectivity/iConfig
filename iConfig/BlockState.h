/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef BLOCKSTATE_H
#define BLOCKSTATE_H

#define kBlockState "BlockState"

namespace BlockState {
typedef enum Enum {
  Empty = 0,
  Half = 1,
  Full = 2,
  UnknownBlockState = 3
} Enum;
}  // namespace BlockState

#endif  // BLOCKSTATE_H
