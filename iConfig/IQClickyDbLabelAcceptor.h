/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef IQCLICKYDBLABELACCEPTOR_H
#define IQCLICKYDBLABELACCEPTOR_H

struct IQClickyDbLabelAcceptor {
  virtual void labelDone(double value) = 0;
};

#endif // IQCLICKYDBLABELACCEPTOR_H
