/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "MyLineEdit.h"

#ifndef Q_MOC_RUN
#include <boost/move/move.hpp>
#endif

#include <QEvent>

MyLineEdit::MyLineEdit(QWidget* _parent) : QLineEdit(_parent) {
  connect(this, SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)));
  refreshData();
}

MyLineEdit::MyLineEdit(const GetQStringFunctor& _getFunc,
                       const SetQStringFunctor& _setFunc,
                       QWidget* _parent)
    : QLineEdit(_parent),
      getFunc(boost::move(_getFunc)),
      setFunc(boost::move(_setFunc)) {
  connect(this, SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)));
  refreshData();
}

void MyLineEdit::refreshData() {
  if (getFunc) {
    auto oldState = this->blockSignals(true);
    auto pos = this->cursorPosition();
    this->setText(getFunc());
    this->setCursorPosition(pos);
    this->blockSignals(oldState);
  }
}

void MyLineEdit::textChanged(QString text) {
  if (setFunc) {
    setFunc(text);
  }
}
