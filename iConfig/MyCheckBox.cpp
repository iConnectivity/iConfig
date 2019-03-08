/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "MyCheckBox.h"

#include <QDebug>
#include <QEvent>

MyCheckBox::MyCheckBox(QWidget *_parent)
    : QCheckBox(_parent), getFunc(), setFunc() {}

MyCheckBox::MyCheckBox(const GetBoolFunctor &_getFunc,
                       const SetBoolFunctor &_setFunc, QWidget *_parent)
    : QCheckBox(_parent),
      getFunc(_getFunc),
      setFunc(_setFunc) {
  connect(this, SIGNAL(stateChanged(int)), this, SLOT(stateChanged(int)));
  refreshData();
}

MyCheckBox::MyCheckBox(const GetBoolFunctor &_getFunc,
                       const SetBoolFunctor &_setFunc, const QString &_text,
                       QWidget *_parent)
    : QCheckBox(_text, _parent),
      getFunc(_getFunc),
      setFunc(_setFunc) {
  connect(this, SIGNAL(stateChanged(int)), this, SLOT(stateChanged(int)));
  refreshData();
}

MyCheckBox::MyCheckBox(const GetBoolFunctor &_getFunc, const SetBoolFunctor &_setFunc, const GetBoolFunctor &_enabledFunc,
             QWidget *_parent)
    : QCheckBox(_parent),
      getFunc(_getFunc),
      setFunc(_setFunc),
      enabledFunc(_enabledFunc){
  connect(this, SIGNAL(stateChanged(int)), this, SLOT(stateChanged(int)));
  refreshData();
}

MyCheckBox::MyCheckBox(const GetBoolFunctor &_getFunc, const SetBoolFunctor &_setFunc, const GetBoolFunctor &_enabledFunc,
             const QString &_text, QWidget *_parent)
    : QCheckBox(_text, _parent),
      getFunc(_getFunc),
      setFunc(_setFunc),
      enabledFunc(_enabledFunc) {
  connect(this, SIGNAL(stateChanged(int)), this, SLOT(stateChanged(int)));
  refreshData();
}


void MyCheckBox::refreshData() {
  if (getFunc) {
    auto oldState = this->blockSignals(true);
    this->setChecked(getFunc());
    this->blockSignals(oldState);
  }
  if (enabledFunc) {
    this->setEnabled(enabledFunc());
  }
}

void MyCheckBox::stateChanged(int state) {
  if (setFunc) {
    setFunc(state == Qt::Checked);
  }
}
