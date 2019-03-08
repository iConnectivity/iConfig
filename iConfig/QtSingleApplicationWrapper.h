/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef QAPPLICATIONWRAPPER_H
#define QAPPLICATIONWRAPPER_H

#include <QtSingleApplication>

class QtSingleApplicationWrapper : public QtSingleApplication
{
  Q_OBJECT
public:
  explicit QtSingleApplicationWrapper(QObject *parent = 0);
  explicit QtSingleApplicationWrapper( int & argc, char ** argv, bool GUIenabled = true );
  explicit QtSingleApplicationWrapper( const QString & appId, int & argc, char ** argv );
  bool notify(QObject *receiver, QEvent *event);
signals:

public slots:

};

#endif // QAPPLICATIONWRAPPER_H
