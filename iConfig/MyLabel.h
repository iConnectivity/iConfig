/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef MYLABEL_H
#define MYLABEL_H

#include "GetSetFunc.h"
#include "IRefreshData.h"

#include <QLabel>

class MyLabel : public QLabel, public IRefreshData {
  Q_OBJECT
 public:
  MyLabel(QWidget* parent = 0);
  MyLabel(const QString& text, QWidget* parent = 0);
  MyLabel(GetQStringFunctor getFunc, QWidget* parent = 0);

  void refreshData();

  GetQStringFunctor getFunc;
};  // MyLabel

#endif  // MYLABEL_H
