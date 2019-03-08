/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef QCLICKYDIAL_H
#define QCLICKYDIAL_H

#include <QDial>
#include <QMouseEvent>


class QClickyDial : public QDial
{
  Q_OBJECT
public:
  explicit QClickyDial(QWidget *parent = 0);
  void setDefaultValue(int def);

signals:

public slots:

protected:
  void mouseDoubleClickEvent(QMouseEvent *e);
  void mousePressEvent(QMouseEvent *e);
  void mouseReleaseEvent(QMouseEvent *e);

private:
  int defaultValue;
};

#endif // QCLICKYDIAL_H
