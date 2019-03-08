/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef IREFRESHDATA_H
#define IREFRESHDATA_H

struct IRefreshData {
  virtual ~IRefreshData() {}
  virtual void refreshData() = 0;
};  // struct IRefreshData

#endif  // IREFRESHDATA_H
