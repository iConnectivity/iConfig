/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#import <UIKit/UIKit.h>

#import "AppDelegate.h"

#ifdef DEBUG
void eHandler(NSException *);
void handler(int);

void eHandler(NSException *exception) {
  NSLog(@"%@", exception);
  NSLog(@"%@", [exception callStackSymbols]);
}

void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}
#endif

int main(int argc, char *argv[]) {
#ifdef DEBUG
  signal(SIGABRT, handler);
  signal(SIGSEGV, handler);

  NSSetUncaughtExceptionHandler(&eHandler);
#endif
  @autoreleasepool {
    return UIApplicationMain(argc, argv, nil,
                             NSStringFromClass([AppDelegate class]));
  }
}
