/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef REFRESHOBJECT_H
#define REFRESHOBJECT_H

#include <QWidget>

class RefreshObject : public QWidget {
  Q_OBJECT
 public:
  explicit RefreshObject(QWidget *parent = 0);
  virtual ~RefreshObject();

 signals:
  void requestRefresh();

 public slots:
  virtual void refreshWidget() {}
};

#endif  // REFRESHOBJECT_H
