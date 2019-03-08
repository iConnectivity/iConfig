/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef QCLICKYDBLABEL_H
#define QCLICKYDBLABEL_H

#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include "IQClickyDbLabelAcceptor.h"

class QClickyDbLabel : public QWidget
{
  Q_OBJECT
public:
  explicit QClickyDbLabel(IQClickyDbLabelAcceptor *parent = 0);

  void setValue(double newValue);
  void mousePressEvent(QMouseEvent *mouseEvent);
  void mouseReleaseEvent(QMouseEvent *mouseEvent);
  void finishEditing();
signals:

public slots:

protected:
  bool eventFilter(QObject *obj, QEvent *event);
private:
  double dbValue;
  QLabel* label;
  QTextEdit* textEdit;
  IQClickyDbLabelAcceptor* parent;
};

#endif // QCLICKYDBLABEL_H
