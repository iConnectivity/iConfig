/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __AUDIOPORTPARAM_H__
#define __AUDIOPORTPARAM_H__

#include <vector>
#include "SysexCommand.h"
#include "LibTypes.h"
#include "BytesCommandData.h"
#include "CommandDataKey.h"
#include "PortType.h"
#include "property.h"
#include "AudioPortParmTypes.h"

namespace GeneSysLib {

/* AudioPortParm
 *  Byte #1: version number = 1
 *  Byte #2-3: portID
 *  Byte #4: portType
 *  Byte #5: numInputChannels
 *  Byte #6: numOutputChannels
 *  Byte #7: numConfigBlocks
 *  Byte #8-N: m_configBlocks
 *  Byte #N: maxPortName
 *  Byte #N+1: portNameLength
 *  Byte #N+2-M: portName
 *  Byte #M-M+1: portInfo
 */
struct AudioPortParm {
  static commandDataKey_t minKey();
  static commandDataKey_t maxKey();
  static commandDataKey_t queryKey(Word portID);
  static CmdEnum retCommand();
  static CmdEnum setCommand();

  struct ConfigBlock {
    ConfigBlock(void);

    // overloaded methods
    void parse(BytesIter &begin, BytesIter &end);
    static ConfigBlock parseConfigBlock(BytesIter &begin, BytesIter &end);

    // properties
    roByte audioConfigNumber;
    roByte maxAudioChannels;
    roByte minInputChannels;
    roByte maxInputChannels;
    roByte minOutputChannels;
    roByte maxOutputChannels;

    bool isInputSelectable() const;
    bool isOutputSelectable() const;
  };  // struct ConfigBlock

  AudioPortParm(void);

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &begin, BytesIter &end);

  // properties
  Byte versionNumber() const;
  roWord audioPortID;
  roPortTypeEnum portType;
  rwByte numInputChannels;
  rwByte numOutputChannels;

  Byte jack() const;

  Byte numConfigBlocks() const;
  const ConfigBlock &block_at(Byte audioConfigNumber) const;

  roByte maxPortName;
  rwString portName;

  bool canEditPortName() const;

  // ALCT: boost::bind is getting confused with member function of same name.
//  static bool isOfType(const AudioPortParm &audioPortParm,
//                       PortTypeEnum portType) {
//    return audioPortParm.portType() == portType;
//  }

  bool isOfType(PortTypeEnum portType) const;

  AudioPortParmTypes::USBDevice &usbDevice();
  const AudioPortParmTypes::USBDevice &usbDevice() const;
  const AudioPortParmTypes::USBHost &usbHost() const;
  const AudioPortParmTypes::Ethernet &ethernet() const;
  const AudioPortParmTypes::Analogue &analogue() const;

 private:
  std::vector<ConfigBlock> m_configBlocks;
  AudioPortParmTypes::Variants m_details;
};  // struct AudioPortParm

struct GetAudioPortParmCommand
    : public BytesSysexCommand<Command::GetAudioPortParm> {
  GetAudioPortParmCommand(DeviceID deviceID, Word transID, Word audioPortID)
      : BytesSysexCommand(deviceID, transID) {
    data.append(audioPortID);
  }
};  // struct GetAudioPortParmCommand

typedef SysexCommand<Command::SetAudioPortParm, AudioPortParm>
    SetAudioPortParmCommand;

}  // namespace GeneSysLib

#endif  // __AUDIOPORTPARAM_H__
