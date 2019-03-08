/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "MySleep.h"

#include <QThread>

void MySleep(unsigned long ms) {
    struct  Thread : public QThread {
        static void msleep(unsigned long ms) {
            QThread::msleep(ms);
        }
    };

    Thread::msleep(ms);
}
