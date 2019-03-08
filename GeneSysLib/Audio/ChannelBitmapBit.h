/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef CHANNELBITMAPBIT_H
#define CHANNELBITMAPBIT_H

namespace GeneSysLib {

namespace ChannelBitmapBit {
typedef enum Enum {
  channel16 = 27,
  channel15 = 26,
  channel14 = 25,
  channel13 = 24,
  channel12 = 19,
  channel11 = 18,
  channel10 = 17,
  channel9 = 16,
  channel8 = 11,
  channel7 = 10,
  channel6 = 9,
  channel5 = 8,
  channel4 = 3,
  channel3 = 2,
  channel2 = 1,
  channel1 = 0
} Enum;
}  // namespace ChannelBitmapBit
typedef ChannelBitmapBit::Enum ChannelBitmapBitEnum;

}  // namespace GeneSysLib

#endif  // CHANNELBITMAPBIT_H
