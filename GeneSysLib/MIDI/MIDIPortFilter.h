/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __MIDIPORTFILTERDATA_H__
#define __MIDIPORTFILTERDATA_H__

#include "LibTypes.h"
#include "FilterID.h"
#include "ChannelFilterStatusBit.h"
#include "ChannelBitmapBit.h"
#include "FilterStatusBit.h"
#include "SysexCommand.h"
#include "BytesCommandData.h"
#include "CommandDataKey.h"
#include "property.h"
#include <bitset>

#define kMIDIPortFilterNumberOfChannels 16


namespace GeneSysLib {

struct MIDIPortFilter {
  static commandDataKey_t minKey();
  static commandDataKey_t maxKey();
  static commandDataKey_t queryKey(Word portID, FilterIDEnum filterType);
  static CmdEnum retCommand();
  static CmdEnum setCommand();

  typedef std::bitset<16> FilterStatus;
  typedef readwrite_property<FilterStatus> rwFilterStatus;

  typedef std::bitset<9> ChannelFilterStatus;
  typedef readwrite_property<ChannelFilterStatus> rwChannelFilterStatus;

  typedef std::bitset<32> ChannelBitmap;
  typedef readwrite_property<ChannelBitmap> rwChannelBitmap;

  typedef boost::array<ChannelFilterStatus, kMIDIPortFilterNumberOfChannels>
      ChannelFilterStatues;

  struct ControllerFilter {
    Bytes generate() const;
    void parse(BytesIter &begin, BytesIter &end);

    ChannelBitmap channelBitmap;
    Byte controllerID;
  };  // struct ControllerFilter

  typedef std::vector<ControllerFilter> ControllerFilters;

  MIDIPortFilter(void);

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &begin, BytesIter &end);

  // properties
  roWord portID;
  roFilterIDEnum filterID;

  rwFilterStatus filterStatus;

  ChannelFilterStatues channelFilterStatus() const;

  ChannelFilterStatus &channelFilterStatus_at(size_t index);
  const ChannelFilterStatus &channelFilterStatus_at(size_t index) const;

  ControllerFilters controllerFilters() const;

  ControllerFilter &controllerFilter_at(size_t index);
  const ControllerFilter &controllerFilter_at(size_t index) const;

  bool allChannelsSet(ChannelFilterStatusBitEnum filterStatusBit) const;
  void setAllChannels(ChannelFilterStatusBitEnum filterStatusBit,
                      bool selected);

 private:
  Byte versionNumber() const;
  ChannelFilterStatues m_channelFilterStatus;
  ControllerFilters m_controllerFilters;
};  // struct MIDIPortFilter

struct GetMIDIPortFilterCommand
    : public BytesSysexCommand<Command::GetMIDIPortFilter> {
  GetMIDIPortFilterCommand(DeviceID deviceID, Word transID, Word portID,
                           FilterIDEnum filterID)
      : BytesSysexCommand(deviceID, transID) {
    // fill the bytes command data with the relavent data
    data.append(portID);
    data.append(static_cast<Byte>(filterID));
  }
};

typedef SysexCommand<Command::SetMIDIPortFilter, MIDIPortFilter>
    SetMIDIPortFilterCommand;

}  // namespace GeneSysLib

#endif  // __MIDIPORTFILTERDATA_H__
