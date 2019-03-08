/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

// stdafx.cpp : source file that includes just the standard includes
// GeneSysLib.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#ifdef WIN32

#include <chrono>
#include <thread>

void usleep(__int64 usec) {
  std::this_thread::sleep_for(std::chrono::microseconds(usec));
}

#endif

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
