/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef MYCONVERTERS_H
#define MYCONVERTERS_H

#include "LibTypes.h"

#include <QString>
#include <QRegExp>

namespace NetAddrTools {

QString fromNetAddr(const NetAddr &netAddr);

NetAddr toNetAddr(QString value);

QRegExp ipRegEx();

}  // namespace NetAddrTools

#endif  // MYCONVERTERS_H
