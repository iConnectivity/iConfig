/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __RTPMIDICONNECTIONDETAIL_H__
#define __RTPMIDICONNECTIONDETAIL_H__

#include "SysexCommand.h"
#include "BytesCommandData.h"
#include "CommandDataKey.h"
#include "property.h"

namespace GeneSysLib {

struct RTPMIDIConnectionDetail {
  static commandDataKey_t minKey();
  static commandDataKey_t maxKey();
  static commandDataKey_t queryKey(Word portID);
  static CmdEnum retCommand();

  RTPMIDIConnectionDetail(void);

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &begin, BytesIter &end);

  // properties
  Byte versionNumber() const;
  roWord portID;
  roByte rtpMIDIConnectionNumber;
  roNetAddr connectedIPAddress;
  roWord rtpMIDIPortNumber;
  roString sessionName;
};  // struct RTPMIDIConnectionDetail

struct GetRTPMIDIConnectionDetailCommand
    : public BytesSysexCommand<Command::GetRTPMIDIConnectionDetail> {
  GetRTPMIDIConnectionDetailCommand(DeviceID deviceID, Word transID,
                                    Word portID, Byte connectionNumber)
      : BytesSysexCommand(deviceID, transID) {
    // data is zeroed in the SysexCommand constructor

    // create data package
    data.append(portID);
    data.append(connectionNumber);
  }
};

}  // namespace GeneSysLib

#endif  // __RTPMIDICONNECTIONDETAIL_H__
