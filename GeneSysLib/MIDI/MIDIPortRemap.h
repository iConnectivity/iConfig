/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __MIDIPORTREMAP_H__
#define __MIDIPORTREMAP_H__

#include "LibTypes.h"
#include "RemapID.h"
#include "SysexCommand.h"
#include "BytesCommandData.h"
#include "CommandDataKey.h"
#include "property.h"

namespace GeneSysLib {

struct MIDIPortRemap {
  static commandDataKey_t minKey();
  static commandDataKey_t maxKey();
  static commandDataKey_t queryKey(Word portID, RemapTypeEnum remapType);
  static CmdEnum retCommand();
  static CmdEnum setCommand();

  struct RemapStatus {
    Bytes generate() const;
    void parse(BytesIter &begin, BytesIter &end);

    bool pitchBendEvents;
    bool channelPressureEvents;
    bool programChangeEvents;
    bool controlChangeEvents;
    bool polyKeyPressureEvents;
    bool noteEvents;
    Byte channelNumber;
  };  // struct RemapStatus

  typedef boost::array<RemapStatus, 16> RemapStatues;

  struct ChannelBitmapBits {
    enum Enum {
      channel16 = 27,
      channel15 = 26,
      channel14 = 25,
      channel13 = 24,
      channel12 = 19,
      channel11 = 18,
      channel10 = 17,
      channel9 = 16,
      channel8 = 11,
      channel7 = 10,
      channel6 = 9,
      channel5 = 8,
      channel4 = 3,
      channel3 = 2,
      channel2 = 1,
      channel1 = 0
    };
  };  // struct ChannelBitmapBits

  typedef std::bitset<32> ChannelBitmap;

  struct RemapFlags {
    ChannelBitmap channelBitmap;
    Byte controllerSource;
    Byte controllerDestination;

    Bytes generate() const;
    void parse(BytesIter &begin, BytesIter &end);
  };  // struct RemapFlags

  typedef std::vector<RemapFlags> RemapFlagsVector;

  MIDIPortRemap(void);
  MIDIPortRemap(Word portID, RemapTypeEnum remapID, Byte maxControllerSupported,
                RemapStatues remapStatuses, RemapFlagsVector controllers);

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &begin, BytesIter &end);

  // properties
  Byte versionNumber() const;
  roWord portID;
  roRemapTypeEnum remapID;
  roByte maxControllerSupported;

  size_t numRemapStatuses() const;
  RemapStatus &remapStatus_at(size_t index);
  const RemapStatus &remapStatus_at(size_t index) const;

  size_t numControllers() const;
  RemapFlags &controller_at(size_t index);
  const RemapFlags &controller_at(size_t index) const;

 private:
  RemapStatues m_remapStatuses;
  RemapFlagsVector m_controllers;
};  // struct MIDIPortRemap

struct GetMIDIPortRemapCommand
    : public BytesSysexCommand<Command::GetMIDIPortRemap> {
  GetMIDIPortRemapCommand(DeviceID deviceID, Word transID, Word portID,
                          RemapTypeEnum remapID)
      : BytesSysexCommand(deviceID, transID) {
    // the default SysexCommand constructor zero fills the data
    data.append(portID);
    data.append(static_cast<Byte>(remapID));
  }
};

typedef SysexCommand<Command::SetMIDIPortRemap, MIDIPortRemap>
    SetMIDIPortRemapCommand;

}  // namespace GeneSysLib

#endif  // __MIDIPORTREMAP_H__
