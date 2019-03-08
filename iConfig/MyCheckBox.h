/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef MYCHECKBOX_H
#define MYCHECKBOX_H

#include "GetSetFunc.h"
#include "IRefreshData.h"

#include <QCheckBox>
#include <QWidget>

class MyCheckBox : public QCheckBox, public IRefreshData {
  Q_OBJECT
 public:
  MyCheckBox(QWidget *parent = 0);
  MyCheckBox(const GetBoolFunctor &_getFunc, const SetBoolFunctor &_setFunc,
             QWidget *_parent = 0);
  MyCheckBox(const GetBoolFunctor &_getFunc, const SetBoolFunctor &_setFunc,
             const QString &_text, QWidget *_parent = 0);
  MyCheckBox(const GetBoolFunctor &_getFunc, const SetBoolFunctor &_setFunc, const GetBoolFunctor &enabledFunc,
             QWidget *_parent = 0);
  MyCheckBox(const GetBoolFunctor &_getFunc, const SetBoolFunctor &_setFunc, const GetBoolFunctor &enabledFunc,
             const QString &_text, QWidget *_parent = 0);

  GetBoolFunctor getFunc;
  SetBoolFunctor setFunc;
  GetBoolFunctor enabledFunc;

  void refreshData();

 private slots:
  void stateChanged(int state);
};  // MyCheckBox

#endif  // MYCHECKBOX_H
