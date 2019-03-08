/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef QCLICKYSLIDER_H
#define QCLICKYSLIDER_H

#include <QSlider>
#include <QTimer>
#include <QMouseEvent>

class QClickySlider : public QSlider
{
  Q_OBJECT
public:
  explicit QClickySlider(QWidget *parent = 0);
  void setDefaultValue(int def);

signals:

public slots:

protected:
  void mouseDoubleClickEvent(QMouseEvent *e);
//  void mousePressEvent(QMouseEvent *e);
//  void mouseReleaseEvent(QMouseEvent *e);

private:
  int defaultValue;
};

#endif // QCLICKYSLIDER_H
