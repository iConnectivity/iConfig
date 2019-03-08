/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __SYSEXPARSER_H__
#define __SYSEXPARSER_H__

#include "LibTypes.h"

#ifndef Q_MOC_RUN
#include <boost/optional.hpp>
#endif
#include <map>

namespace GeneSysLib {

struct SysexParser {
  SysexParser(void);
  ~SysexParser(void);

  long registerHandler(CmdEnum command, Handler &handler);
  void unRegisterHandler(CmdEnum command);
  void unRegisterHandler(CmdEnum command, long id);
  void unRegisterAll();

  void registerExclusiveHandler(CmdEnum command, Handler &handler);
  void unRegisterExclusiveHandler();

  bool parse(Bytes sysex) const;

 private:
  std::map<CmdEnum, std::map<long, Handler> > m_handlers;

  boost::optional<CmdEnum> m_exclusiveHandlerCommand;
  boost::optional<Handler> m_exclusiveHandler;

  commandData_t createCommandDataObject(CmdEnum command) const;

  static long nextID;
};  // struct SysexParser

}  // namespace GeneSysLib

#endif  // __SYSEXPARSER_H__
