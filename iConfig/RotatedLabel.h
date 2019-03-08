/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef ROTATEDLABEL_H
#define ROTATEDLABEL_H

#include <QLabel>
#include <QWidget>
#include <QPaintEvent>

class RotatedLabel : public QLabel {
  Q_OBJECT
 public:
  explicit RotatedLabel(QWidget *parent = 0);

  void setAngle(float angle);

 private:
  float angle;

 protected:
  void paintEvent(QPaintEvent *event);
};

#endif  // ROTATEDLABEL_H
