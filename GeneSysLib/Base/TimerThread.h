/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef TIMERTHREAD_H
#define TIMERTHREAD_H

#ifndef __IOS__

#include <QThread>
#include <QTimer>

class TimerThread : public QThread {
  Q_OBJECT
 public:
  explicit TimerThread(QObject *parent = 0);
  ~TimerThread();

signals:
  void timerStarted();
  void timerStopped();

  void timedOut();

 public slots:
  void startTimer();
  void stopTimer();

 protected:
  void run();

 private slots:
  void timeout();

 private:
  QTimer *timer;
};

#endif  // __IOS__

#endif  // TIMERTHREAD_H
