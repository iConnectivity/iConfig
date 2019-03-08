/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "MIDIPortFilter.h"
#include "Generator.h"

#ifndef Q_MOC_RUN
#include <boost/range/algorithm_ext.hpp>
#include <boost/algorithm/cxx11/any_of.hpp>
#endif
#include <limits>

using boost::range::push_back;
using boost::algorithm::any_of;

namespace GeneSysLib {

commandDataKey_t MIDIPortFilter::minKey() {
  return generateKey(Command::RetMIDIPortFilter);
}

commandDataKey_t MIDIPortFilter::maxKey() {
  return MIDIPortFilter::queryKey(std::numeric_limits<Word>::max(),
                                  FilterID::Unknown);
}

commandDataKey_t MIDIPortFilter::queryKey(Word portID,
                                          FilterIDEnum filterType) {
  return generateKey(Command::RetMIDIPortFilter, portID, filterType);
}

CmdEnum MIDIPortFilter::retCommand() { return Command::RetMIDIPortFilter; }

CmdEnum MIDIPortFilter::setCommand() { return Command::SetMIDIPortFilter; }

Bytes MIDIPortFilter::ControllerFilter::generate() const {
  Bytes result;

  // MIDI channel bitmap
  unsigned long val = channelBitmap.to_ulong();  // TPK - CANNOT BE AUTO
  result += static_cast<Byte>((val >> 24) & 0x0F);
  result += static_cast<Byte>((val >> 16) & 0x0F);
  result += static_cast<Byte>((val >> 8) & 0x0F);
  result += static_cast<Byte>((val) & 0x0F);

  // controller ID
  result += controllerID & 0x7F;

  return result;
}

void MIDIPortFilter::ControllerFilter::parse(BytesIter &beginIter,
                                             BytesIter &endIter) {
  // MIDI channel bitmap
  unsigned long val = nextMidiByte(beginIter, endIter);  // TPK - CANNOT BE AUTO
  val = (val << 8) | (nextMidiByte(beginIter, endIter));
  val = (val << 8) | (nextMidiByte(beginIter, endIter));
  val = (val << 8) | (nextMidiByte(beginIter, endIter));
  channelBitmap = ChannelBitmap(val);

  // controller ID
  controllerID = nextMidiByte(beginIter, endIter);
}

MIDIPortFilter::MIDIPortFilter(void)
    : portID(),
      filterID(),
      filterStatus(),
      m_channelFilterStatus(),
      m_controllerFilters() {}

const commandDataKey_t MIDIPortFilter::key() const {
  return generateKey(Command::RetMIDIPortFilter, portID(), filterID());
}

Bytes MIDIPortFilter::generate() const {
  Bytes result;

  // command version  (Byte   #1)
  result += versionNumber() & 0x7F;

  // port ID          (Bytes #2-3)
  appendMidiWord(result, portID());

  // filter ID        (Byte   #4)
  result += static_cast<Byte>(filterID());

  // max ctrl fltrs   (Byte   #5)
  result += static_cast<Byte>(m_controllerFilters.size());

  // filter status    (Bytes #6-7)
  auto val = filterStatus().to_ulong();
  result += static_cast<Byte>((val >> 8) & 0x7F);
  result += static_cast<Byte>(val & 0x7F);

  // channel filter   (Bytes #8-23)
  for (const auto &ch : m_channelFilterStatus) {
    result += static_cast<Byte>(ch.to_ulong() & 0x7F);
  }

  // controller filters (Bytes #26 - N)
  for (const auto &ctrl : m_controllerFilters) {
    push_back(result, ctrl.generate());
  }

  return result;
}

void MIDIPortFilter::parse(BytesIter &beginIter, BytesIter &endIter) {
  auto version = nextMidiByte(beginIter, endIter);

  if (version == versionNumber()) {
    portID = nextROWord(beginIter, endIter);
    filterID = roFilterIDEnum(
        static_cast<FilterIDEnum>(nextMidiByte(beginIter, endIter)));
    auto numControllerFilters = nextMidiByte(beginIter, endIter);

    long val = nextMidiByte(beginIter, endIter);
    val = (val << 8) | nextMidiByte(beginIter, endIter);
    filterStatus = rwFilterStatus(FilterStatus(val));

    for (auto &chFilterStatus : m_channelFilterStatus) {
      chFilterStatus = nextMidiByte(beginIter, endIter);
    }

    m_controllerFilters.clear();
    for (auto i = 0; i < numControllerFilters; ++i) {
      ControllerFilter ctrlFilter;
      ctrlFilter.parse(beginIter, endIter);
      m_controllerFilters.push_back(ctrlFilter);
    }
  }
}

MIDIPortFilter::ChannelFilterStatues MIDIPortFilter::channelFilterStatus()
    const {
  return m_channelFilterStatus;
}

MIDIPortFilter::ChannelFilterStatus &MIDIPortFilter::channelFilterStatus_at(
    size_t index) {
  return m_channelFilterStatus.at(index);
}

const MIDIPortFilter::ChannelFilterStatus &
MIDIPortFilter::channelFilterStatus_at(size_t index) const {
  return m_channelFilterStatus.at(index);
}

MIDIPortFilter::ControllerFilters MIDIPortFilter::controllerFilters() const {
  return m_controllerFilters;
}

MIDIPortFilter::ControllerFilter &MIDIPortFilter::controllerFilter_at(
    size_t index) {
  return m_controllerFilters.at(index);
}

const MIDIPortFilter::ControllerFilter &MIDIPortFilter::controllerFilter_at(
    size_t index) const {
  return m_controllerFilters.at(index);
}

bool MIDIPortFilter::allChannelsSet(
    ChannelFilterStatusBitEnum channelFilterStatusBit) const {
  return boost::algorithm::any_of(
      m_channelFilterStatus.begin(), m_channelFilterStatus.end(),
      boost::bind(&MIDIPortFilter::ChannelFilterStatus::test, _1,
                  channelFilterStatusBit));
}

void MIDIPortFilter::setAllChannels(
    ChannelFilterStatusBitEnum channelFilterStatusBit, bool selected) {
  for (auto &cs : m_channelFilterStatus) {
    cs[channelFilterStatusBit] = selected;
  }
}

Byte MIDIPortFilter::versionNumber() const { return 0x01; }

}  // namespace GeneSysLib
