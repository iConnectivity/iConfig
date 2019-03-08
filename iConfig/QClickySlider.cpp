/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "QClickySlider.h"

QClickySlider::QClickySlider(QWidget *parent) :
  QSlider(parent)
{
  defaultValue = 0;
#ifdef _WIN32
  setStyleSheet("QClickySlider {margin-right:10px;} QClickySlider::groove:vertical { border: none; background-color: #999999; position: absolute; left: 8px; right: 8px;  } QClickySlider::handle:vertical {margin: 0 -6px; image: url(:/mixer_graphics/Images/mixer_graphics/handle.png);} QSlider::add-page:vertical {background: #46748B;} QSlider::sub-page:vertical {background: #262626;}");
#else
  setStyleSheet("QClickySlider {margin-right:10px;} QClickySlider::groove:vertical { border: none; background-color: #999999; position: absolute; left: 5px; right: 5px;  } QClickySlider::handle:vertical {margin: 0 -5px; image: url(:/mixer_graphics/Images/mixer_graphics/handle.png);} QSlider::add-page:vertical {background: #46748B;} QSlider::sub-page:vertical {background: #262626;}");
#endif
}


void QClickySlider::setDefaultValue(int def) {
  defaultValue = def;
}

void QClickySlider::mouseDoubleClickEvent(QMouseEvent *e) {
  if ( e->button() == Qt::LeftButton )
  {
      this->setValue(defaultValue);
  }
}

/*void QClickySlider::mousePressEvent(QMouseEvent *e)
{
  return;
}

void QClickySlider::mouseReleaseEvent(QMouseEvent *e)
{
  return;
}*/

