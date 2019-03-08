/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "QClickyDial.h"

QClickyDial::QClickyDial(QWidget *parent) :
  QDial(parent)
{
  defaultValue = 0;
}

void QClickyDial::setDefaultValue(int def) {
  defaultValue = def;
}

void QClickyDial::mouseDoubleClickEvent(QMouseEvent *e) {
  if ( e->button() == Qt::LeftButton )
  {
      this->setValue(defaultValue);
  }
}

void QClickyDial::mousePressEvent(QMouseEvent *e)
{
  return;
}

void QClickyDial::mouseReleaseEvent(QMouseEvent *e)
{
  return;
}
