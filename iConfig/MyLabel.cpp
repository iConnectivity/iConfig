/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "MyLabel.h"

MyLabel::MyLabel(QWidget* _parent) : QLabel(_parent), getFunc() {
  setWordWrap(true);
}

MyLabel::MyLabel(GetQStringFunctor _getFunc, QWidget* _parent)
    : QLabel(_parent), getFunc(_getFunc) {
  refreshData();
  setWordWrap(true);
}

MyLabel::MyLabel(const QString& _text, QWidget* _parent)
    : QLabel(_text, _parent), getFunc() {
  setWordWrap(true);
}

void MyLabel::refreshData() {
  auto oldState = this->blockSignals(true);

  if (getFunc) {
    this->setText(getFunc());
  }

  this->blockSignals(oldState);
}
