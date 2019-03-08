/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __BLOCKNOTFOUND_H__
#define __BLOCKNOTFOUND_H__

struct blockNotFoundException : public std::exception {
  blockNotFoundException(std::string _info) : info(_info) {}
  std::string info;
};

#endif  // __BLOCKNOTFOUND_H__
