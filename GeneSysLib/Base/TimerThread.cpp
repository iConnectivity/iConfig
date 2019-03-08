/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "timerthread.h"

#ifndef __IOS__

TimerThread::TimerThread(QObject *parent) : QThread(parent) {
  connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

TimerThread::~TimerThread() {
  if (isRunning()) {
    quit();
    wait();
  }
}

void TimerThread::startTimer() { emit timerStarted(); }

void TimerThread::stopTimer() { emit timerStopped(); }

void TimerThread::run() {
  /* initialize */
  timer = new QTimer();
  connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
  connect(this, SIGNAL(timerStarted()), timer, SLOT(start()));
  connect(this, SIGNAL(timerStopped()), timer, SLOT(stop()));

  timer->setSingleShot(true);
  timer->setInterval(5000);

  exec();

  timer->deleteLater();
}

void TimerThread::timeout() { emit timedOut(); }

#endif  // __IOS__
