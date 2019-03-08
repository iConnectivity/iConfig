/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef MYLINEEDIT_H
#define MYLINEEDIT_H

#include "GetSetFunc.h"
#include "IRefreshData.h"

#include <QLineEdit>

class MyLineEdit : public QLineEdit, public IRefreshData {
  Q_OBJECT
public:
  MyLineEdit(QWidget* parent = 0);
  MyLineEdit(const GetQStringFunctor& getFunc,
             const SetQStringFunctor& setFunc,
             QWidget* parent = 0);

  void refreshData();

  GetQStringFunctor getFunc;
  SetQStringFunctor setFunc;

 private slots:
  void textChanged(QString);
};  // class MyLineEdit

#endif  // MYLINEEDIT_H
