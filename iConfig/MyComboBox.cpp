/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "MyComboBox.h"

#include <QEvent>

#ifndef Q_MOC_RUN
#include <boost/move/move.hpp>
#endif

MyComboBox::MyComboBox(QWidget *_parent)
    : QComboBox(_parent), getOptions(), getFunc(), setFunc() {
  connect(this, SIGNAL(currentIndexChanged(int)), this,
          SLOT(currentIndexChanged(int)));
}

MyComboBox::MyComboBox(GetComboBoxOptions _getOptions,
                       GetComboBoxSelectionFunctor _getFunc,
                       SetComboBoxSelectionFunctor _setFunc, QWidget *_parent)
    : QComboBox(_parent),
      getOptions(_getOptions),
      getFunc(_getFunc),
      setFunc(_setFunc) {
  connect(this, SIGNAL(currentIndexChanged(int)), this,
          SLOT(currentIndexChanged(int)));

  refreshData();
}

void MyComboBox::wheelEvent(QWheelEvent *) {
  // Do nothing
}

void MyComboBox::refreshData() {
  auto oldState = this->blockSignals(true);
  if (getOptions) {
    this->clear();
    this->addItems(getOptions());
  }

  if (getFunc) {
    int newIndex = getFunc(this);
    if (newIndex > this->count()) {
      newIndex = 0;
      if (setFunc)
        setFunc(this, 0);
    }
    this->setCurrentIndex(newIndex);
  }
  this->blockSignals(oldState);
}

void MyComboBox::currentIndexChanged(int index) {
  if (setFunc) {
    setFunc(this, index);
  }
}
