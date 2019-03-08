/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef IREFRESHWIDGET_H
#define IREFRESHWIDGET_H

struct IRefreshWidget {
  virtual ~IRefreshWidget() {}
  virtual void refreshWidget() = 0;
  virtual void refreshMeters() = 0;
};  // struct IRefreshWidget

#endif  // IREFRESHWIDGET_H
