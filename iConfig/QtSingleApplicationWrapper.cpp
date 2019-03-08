/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "QtSingleApplicationWrapper.h"
#include <iostream>

QtSingleApplicationWrapper::QtSingleApplicationWrapper(int &argc, char **argv, bool GUIenabled) :
  QtSingleApplication(argc, argv, GUIenabled)
{
//  QtSingleApplication::;
}

QtSingleApplicationWrapper::QtSingleApplicationWrapper(const QString &appId, int &argc, char **argv) :
  QtSingleApplication(appId, argc, argv)
{

}

bool QtSingleApplicationWrapper::notify(QObject *receiver, QEvent *event) {
  try {
    return QtSingleApplication::notify( receiver, event );
  } catch ( std::exception& e ) {
    std::cout << "exception caught: " << e.what() << '\n';
    return false;
  }
}
