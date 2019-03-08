/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "MyConverters.h"

#include <QHostAddress>

namespace NetAddrTools {

QString fromNetAddr(const NetAddr &netAddr) {
  return QString("%1.%2.%3.%4").arg(QString::number(netAddr[0]))
      .arg(QString::number(netAddr[1])).arg(QString::number(netAddr[2]))
      .arg(QString::number(netAddr[3]));
}

NetAddr toNetAddr(QString value) {
  auto hostAddr = QHostAddress(value).toIPv4Address();
  NetAddr addr;
  addr[0] = (hostAddr >> (8 * 3)) & 0xFF;
  addr[1] = (hostAddr >> (8 * 2)) & 0xFF;
  addr[2] = (hostAddr >> (8 * 1)) & 0xFF;
  addr[3] = (hostAddr) & 0xFF;
  return addr;
}

QRegExp ipRegEx() {
  static QString octet = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
  static QRegExp ipRegEx = QRegExp("^" + octet + "\\." + octet + "\\." + octet +
                                   "\\." + octet + "$");
  return ipRegEx;
}

}  // namespace NetAddrTools
