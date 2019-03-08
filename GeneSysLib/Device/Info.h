/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __INFO_H__
#define __INFO_H__

#include <string>
#include "LibTypes.h"
#include "InfoID.h"
#include "ByteCommandData.h"
#include "SysexCommand.h"
#include "CommandDataKey.h"

namespace GeneSysLib {

// forward declarations
struct DeviceID;

struct Info {
  static commandDataKey_t minKey();
  static commandDataKey_t maxKey();
  static commandDataKey_t queryKey(InfoIDEnum infoID);
  static CmdEnum retCommand();
  static CmdEnum setCommand();

  Info(void);
  Info(InfoIDEnum infoID, const std::string &infoString);

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &begin, BytesIter &end);

  // properties
  roInfoIDEnum infoID;
  rwString infoString;
};  // struct Info

typedef ByteCommandData<Command::GetInfo> GetInfoData;
typedef Info SetInfoData;

struct GetInfoCommand : public SysexCommand<Command::GetInfo, GetInfoData> {
  GetInfoCommand(DeviceID deviceID, Word transID, InfoIDEnum infoID)
      : SysexCommand(deviceID, transID, GetInfoData(infoID)) {}
};

struct SetInfoCommand : public SysexCommand<Command::SetInfo, SetInfoData> {
  SetInfoCommand(DeviceID deviceID, Word transID, Info info)
      : SysexCommand(deviceID, transID, info) {}
  SetInfoCommand(DeviceID deviceID, Word transID, InfoIDEnum infoID,
                 std::string infoString)
      : SysexCommand(deviceID, transID, SetInfoData(infoID, infoString)) {}
};

}  // namespace GeneSysLib

#endif  // __INFO_H__
