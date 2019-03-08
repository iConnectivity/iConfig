/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef MYSPINBOX_H
#define MYSPINBOX_H

#include "GetSetFunc.h"
#include "IRefreshData.h"

#include <QSpinBox>

class MySpinBox : public QSpinBox, public IRefreshData {
  Q_OBJECT
public:
  MySpinBox(QWidget* parent = 0);
  MySpinBox(const GetIntFunctor& getFunc,
            const SetIntFunctor& setFunc,
            QWidget* parent = 0);

  void refreshData();

  GetIntFunctor getFunc;
  SetIntFunctor setFunc;

private slots:
  void valueChanged(int value);
};

#endif  // MYSPINBOX_H
