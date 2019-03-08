/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "MySpinBox.h"

MySpinBox::MySpinBox(QWidget* _parent)
    : QSpinBox(_parent), getFunc(), setFunc() {
  connect(this, SIGNAL(valueChanged(int)), this, SLOT(valueChanged(int)));
}

MySpinBox::MySpinBox(const GetIntFunctor& _getFunc,
                     const SetIntFunctor& _setFunc,
                     QWidget* _parent)
    : QSpinBox(_parent), getFunc(_getFunc), setFunc(_setFunc) {
  connect(this, SIGNAL(valueChanged(int)), this, SLOT(valueChanged(int)));
  refreshData();
}

void MySpinBox::refreshData() {
  if (getFunc) {
    auto oldState = this->blockSignals(true);
    this->setValue(getFunc());
    this->blockSignals(oldState);
  }
}

void MySpinBox::valueChanged(int value) {
  if (setFunc) {
    setFunc(value);
  }
}
