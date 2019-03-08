/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef MIXEROUTPUTCONTROL_H
#define MIXEROUTPUTCONTROL_H

#include "BytesCommandData.h"
#include "CommandDataKey.h"
#include "CommandDefines.h"
#include "LibTypes.h"
#include "SysexCommand.h"
#include "AudioDeviceTypes.h"
#include "property.h"
#include "ControllerType.h"
#include "StreamHelpers.h"

namespace GeneSysLib {

struct MixerOutputControl {
  static commandDataKey_t minKey();
  static commandDataKey_t maxKey();
  static commandDataKey_t queryKey(Word audioPortID);
  static CmdEnum retCommand();

  MixerOutputControl();

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &beginIter, BytesIter &endIter);

  bool panExists() const { return existFlags.test(6); }
  bool invertExists() const { return existFlags.test(5); }
  bool stereoLinkExists() const { return existFlags.test(4); }
  bool soloPFLExists() const { return existFlags.test(3); }
  bool soloExists() const { return existFlags.test(2); }
  bool muteExists() const { return existFlags.test(1); }
  bool volumeExists() const { return existFlags.test(0); }

  bool panEditable() const { return editFlags.test(6); }
  bool invertEditable() const { return editFlags.test(5); }
  bool stereoLinkEditable() const { return editFlags.test(4); }
  bool soloPFLEditable() const { return editFlags.test(3); }
  bool soloEditable() const { return editFlags.test(2); }
  bool muteEditable() const { return editFlags.test(1); }
  bool volumeEditable() const { return editFlags.test(0); }

  // properties
  Byte versionNumber() const;
  roWord audioPortID;
  std::bitset<8> existFlags;
  std::bitset<8> editFlags;
  roWord maximumPanControl;
  roByte numberPanCurveLaws;
  std::vector<roByte> panCurveLaws;
  roWord minimumVolumeControl;
  roWord maximumVolumeControl;
  roWord resolutionVolumeControl;

};  // struct MixerOutputControl

struct GetMixerOutputControlCommand
    : public BytesSysexCommand<Command::GetMixerOutputControl> {
  GetMixerOutputControlCommand(DeviceID deviceID, Word transID,
                                    Word audioPortID)
      : BytesSysexCommand(deviceID, transID) {
    data.append(audioPortID);
  }
};  // GetMixerOutputControlCommand

}  // namespace GeneSysLib

#endif // MIXEROUTPUTCONTROL_H
