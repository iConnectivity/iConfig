/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "RotatedLabel.h"

#include <QPainter>

#include <cmath>

RotatedLabel::RotatedLabel(QWidget *parent) : QLabel(parent), angle(0.f) {}

void RotatedLabel::setAngle(float _angle) {
  angle = fmod(_angle, 360.0f);
  if (angle < 0) {
    angle += 360.0f;
  }
}

void RotatedLabel::paintEvent(QPaintEvent *) {
  QPainter p;
  p.begin(this);
  p.setFont(font());

  p.save();
  p.translate(width() * 0.7f, height() * 0.9f);
  p.rotate(angle);
  p.drawText(0, 0, text());
  p.restore();
  p.end();
}
