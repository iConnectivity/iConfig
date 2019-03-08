/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "MainWindow.h"

#include <QApplication>
#include "QtSingleApplicationWrapper.h"

int main(int argc, char *argv[]) {

#ifdef Q_OS_MAC
  if (QSysInfo::MacintoshVersion > QSysInfo::MV_10_8) {
    // fix Mac OS X 10.9 (mavericks) font issue
    // https://bugreports.qt-project.org/browse/QTBUG-32789
    QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
  }
#endif

  QtSingleApplicationWrapper instance("iConnectivity iConfig", argc, argv);
  QThread::currentThread()->setPriority(QThread::LowPriority);
  if (instance.sendMessage("Application already running")) {
    return 0;
  }

  QCoreApplication::setOrganizationName("iConnectivity");
  QCoreApplication::setOrganizationDomain("iConnectivity.com");
  QCoreApplication::setApplicationName("iConnectivity iConfig");
  //QCoreApplication::setApplicationVersion("4.1.3");
  //QCoreApplication::setApplicationVersion("4.2.0");
  //bugfxing: version change to "4.2.1"
  //--zx,2016-06-08
  //QCoreApplication::setApplicationVersion("4.2.1");
  //QCoreApplication::setApplicationVersion("4.2.3");
  //--zx-03-23
  //QCoreApplication::setApplicationVersion("4.2.4");
  //QCoreApplication::setApplicationVersion("4.2.5"); //zx, 2017-04-26
  //QCoreApplication::setApplicationVersion("4.2.6"); //zx, 2017-06-22
  QCoreApplication::setApplicationVersion("4.2.7"); //fix, 2017-11-15
  MainWindow w;
  w.show();

  return instance.exec();
}
