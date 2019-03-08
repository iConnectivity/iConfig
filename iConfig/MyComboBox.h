/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef MYCOMBOBOX_H
#define MYCOMBOBOX_H

#include "GetSetFunc.h"
#include "IRefreshData.h"

#include <QComboBox>

class MyComboBox : public QComboBox, public IRefreshData {
  Q_OBJECT
 public:
  MyComboBox(QWidget *parent = 0);
  MyComboBox(GetComboBoxOptions getOptions, GetComboBoxSelectionFunctor getFunc,
             SetComboBoxSelectionFunctor setFunc, QWidget *parent = 0);

  void wheelEvent(QWheelEvent *e);
  void refreshData();

  GetComboBoxOptions getOptions;
  GetComboBoxSelectionFunctor getFunc;
  SetComboBoxSelectionFunctor setFunc;

 private slots:
  void currentIndexChanged(int index);
};  // class MyComboBox

#endif  // MYCOMBOBOX_H
