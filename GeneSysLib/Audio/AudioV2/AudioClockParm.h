/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef AUDIOCLOCKPARAM_H
#define AUDIOCLOCKPARAM_H

#include "CommandDataKey.h"
#include "LibTypes.h"
#include "ClockSourceType.h"
#include "SysexCommand.h"
#include "BitDepthCode.h"
#include "property.h"
#ifndef Q_MOC_RUN
#include <boost/function.hpp>
#include <boost/range/adaptors.hpp>
#endif
#include <map>

using namespace boost::adaptors;

namespace GeneSysLib {

struct AudioClockParm {
  static CmdEnum retCommand();
  static CmdEnum setCommand();

  typedef struct SourceBlock {
    typedef union SourceInfo {
      struct Common {
        Byte number;
        Byte reserved;
      } common;
      struct Internal {
        Byte clockNumber;
      } internal;
      struct PortClock {
        Byte portNumber;
      } portClock;
    } SourceInfo;

    // overloaded methods
    Bytes generate() const;
    void parse(BytesIter &begin, BytesIter &end);

    // to string
    std::string toString() const;

    // variables
    roByte sourceNumber;
    roClockSourceEnum sourceType;
    SourceInfo sourceInfo;
  } SourceBlock;

  AudioClockParm(void);

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &begin, BytesIter &end);

  // properties
  Byte versionNumber() const;
  rwByte activeSourceBlock;

  template <typename Action>
  void for_each(Action action) const {
    for (const auto &block : m_sourceBlocks | map_values) {
      action(block);
    }
  }

 private:
  std::map<Byte, SourceBlock> m_sourceBlocks;
};  // struct AudioClockParm

typedef SysexCommand<Command::GetAudioClockParm, EmptyCommandData>
    GetAudioClockParmCommand;

typedef SysexCommand<Command::SetAudioClockParm, AudioClockParm>
    SetAudioClockParmCommand;

}  // namespace GeneSysLib

#endif  // AUDIOCLOCKPARAM_H
