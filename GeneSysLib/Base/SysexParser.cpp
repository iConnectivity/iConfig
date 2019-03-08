/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "SysexParser.h"

#include "MyAlgorithms.h"
#include "CommandData.h"
#include "Device.h"
#include "DeviceID.h"
#include "CommandList.h"
#include "InfoList.h"
#include "Info.h"
#include "ACK.h"
#include "ResetList.h"
#include "BytesCommandData.h"
#include "SaveRestoreList.h"
#include "EthernetPortInfo.h"
#include "GizmoCount.h"
#include "GizmoInfo.h"

#include "MIDIInfo.h"
#include "MIDIPortFilter.h"
#include "MIDIPortInfo.h"
#include "MIDIPortRemap.h"
#include "MIDIPortRoute.h"
#include "MIDIPortDetail.h"
#include "RTPMIDIConnectionDetail.h"
#include "USBHostMIDIDeviceDetail.h"

#include "AudioInfo.h"
#include "AudioCfgInfo.h"
#include "AudioPortInfo.h"
#include "AudioPortCfgInfo.h"
#include "AudioPortPatchbay.h"
#include "AudioClockInfo.h"

#include "AudioGlobalParm.h"
#include "AudioPortParm.h"
#include "AudioDeviceParm.h"
#include "AudioControlParm.h"
#include "AudioControlDetail.h"
#include "AudioControlDetailValue.h"
#include "AudioClockParm.h"
#include "AudioPatchbayParm.h"
#include "AudioPortMeterValue.h"

#include "MixerInputControl.h"
#include "MixerOutputControl.h"
#include "MixerInputControlValue.h"
#include "MixerOutputControlValue.h"
#include "MixerInputParm.h"
#include "MixerOutputParm.h"
#include "MixerParm.h"
#include "MixerPortParm.h"
#include "MixerMeterValue.h"

#include <map>
#include <numeric>
#ifndef Q_MOC_RUN
#include <boost/range.hpp>
#include <boost/range/adaptors.hpp>
#endif

using namespace std;
using namespace boost;
using namespace boost::adaptors;
using namespace MyAlgorithms;

namespace GeneSysLib {

long SysexParser::nextID = 0;

SysexParser::SysexParser(void)
    : m_handlers(), m_exclusiveHandlerCommand(), m_exclusiveHandler() {}

SysexParser::~SysexParser(void) { m_handlers.clear(); }

long SysexParser::registerHandler(CmdEnum command, Handler& handler) {
  m_handlers[command][nextID] = handler;
  return nextID++;
}

void SysexParser::unRegisterHandler(CmdEnum command) {
  if (m_handlers.find(command) != m_handlers.end()) {
    m_handlers.erase(command);
  }
}

void SysexParser::unRegisterHandler(CmdEnum command, long id) {
  if (m_handlers.find(command) != m_handlers.end()) {
    m_handlers.at(command).erase(id);
  }
}

void SysexParser::unRegisterAll() { m_handlers.clear(); }

void SysexParser::registerExclusiveHandler(CmdEnum command, Handler& handler) {
  m_exclusiveHandlerCommand.reset(command);
  m_exclusiveHandler.reset(handler);
}

void SysexParser::unRegisterExclusiveHandler() {
  m_exclusiveHandlerCommand.reset();
  m_exclusiveHandler.reset();
}

bool SysexParser::parse(Bytes sysex) const {
  bool error = false;
  auto beginIter = sysex.begin();
  auto endIter = sysex.end();

  static unsigned long IDLOG = 0;

  Word productID;
  SerialNumber sn;
  DeviceID deviceID;
  Word transID;
  CmdEnum cmdID;
  Word dataLength;

  // check the size
  error = (sysex.size() < 19);

  // check header
  if (!error) {
    Bytes header;
    copy(beginIter, beginIter + 5, std::back_inserter(header));
    advance(beginIter, 5);

    Bytes expectedHeader;
    expectedHeader += 0xF0, 0x00, 0x01, 0x73, 0x7E;

    error = (expectedHeader != header);
  }


  // check the checksum
  if (!error) {
    Byte cs = (accumulate(beginIter, endIter - 1, 0x00)) & 0x7F;
    error = (cs != 0x00);
  }

  // check footer
  if (!error) {
    error = (sysex.back() != 0xF7);
  }

  if (!error) {
    productID = nextMidiWord(beginIter, endIter);

#if _WIN32
    copy(beginIter, beginIter + 5,
         stdext::checked_array_iterator<SerialNumber::iterator>(sn.begin(), 5));
#else
    copy(beginIter, beginIter + 5, sn.begin());
#endif
    advance(beginIter, 5);
    deviceID = DeviceID(productID, sn);

    transID = nextMidiWord(beginIter, endIter);
    cmdID = static_cast<CmdEnum>(nextMidiWord(beginIter, endIter));

    if ((cmdID & WRITE_BIT) == WRITE_BIT) {
      error = true;
    }

    if (!error) {
      dataLength = nextMidiWord(beginIter, endIter);
      error = (std::distance(beginIter, endIter - 2) != dataLength);
    }

    if (!error) {
      auto cmdData = createCommandDataObject(cmdID);

      auto endWithoutFooter = endIter - 2;
      cmdData.parse(beginIter, endWithoutFooter);

      if ((m_exclusiveHandlerCommand) &&
          (*m_exclusiveHandlerCommand == cmdID) && (m_exclusiveHandler)) {

        (*m_exclusiveHandler)(cmdID, deviceID, transID, cmdData);
      } else {
        for (const auto& handler : m_handlers.at(cmdID) | map_values) {
          handler(cmdID, deviceID, transID, cmdData);
        }
      }
      IDLOG++;
    }
  }

  return error;
}

commandData_t SysexParser::createCommandDataObject(CmdEnum command) const {
  commandData_t result = BytesCommandData<Command::Unknown>();

  static bool tableCreated = false;
  static map<CmdEnum, commandData_t> table;

  if (!tableCreated) {
    tableCreated = true;
    table[Command::RetDevice] = Device();
    table[Command::RetCommandList] = CommandList();
    table[Command::RetInfoList] = InfoList();
    table[Command::RetInfo] = Info();
    table[Command::RetResetList] = ResetList();
    table[Command::RetSaveRestoreList] = SaveRestoreList();
    table[Command::RetEthernetPortInfo] = EthernetPortInfo();
    table[Command::ACK] = ACK();
    table[Command::RetGizmoCount] = GizmoCount();
    table[Command::RetGizmoInfo] = GizmoInfo();

    table[Command::RetMIDIInfo] = MIDIInfo();
    table[Command::RetMIDIPortInfo] = MIDIPortInfo();
    table[Command::RetMIDIPortFilter] = MIDIPortFilter();
    table[Command::RetMIDIPortRemap] = MIDIPortRemap();
    table[Command::RetMIDIPortRoute] = MIDIPortRoute();
    table[Command::RetMIDIPortDetail] = MIDIPortDetail();
    table[Command::RetRTPMIDIConnectionDetail] = RTPMIDIConnectionDetail();
    table[Command::RetUSBHostMIDIDeviceDetail] = USBHostMIDIDeviceDetail();

    table[Command::RetAudioInfo] = AudioInfo();
    table[Command::RetAudioCfgInfo] = AudioCfgInfo();
    table[Command::RetAudioPortInfo] = AudioPortInfo();
    table[Command::RetAudioPortCfgInfo] = AudioPortCfgInfo();
    table[Command::RetAudioPortPatchbay] = AudioPortPatchbay();
    table[Command::RetAudioClockInfo] = AudioClockInfo();

    table[Command::RetAudioGlobalParm] = AudioGlobalParm();
    table[Command::RetAudioPortParm] = AudioPortParm();
    table[Command::RetAudioDeviceParm] = AudioDeviceParm();
    table[Command::RetAudioControlParm] = AudioControlParm();
    table[Command::RetAudioControlDetail] = AudioControlDetail();
    table[Command::RetAudioControlDetailValue] = AudioControlDetailValue();
    table[Command::RetAudioClockParm] = AudioClockParm();
    table[Command::RetAudioPatchbayParm] = AudioPatchbayParm();
    table[Command::RetAudioPortMeterValue] = AudioPortMeterValue();
    table[Command::RetMixerParm] = MixerParm();
    table[Command::RetMixerPortParm] = MixerPortParm();
    table[Command::RetMixerInputParm] = MixerInputParm();
    table[Command::RetMixerOutputParm] = MixerOutputParm();
    table[Command::RetMixerInputControl] = MixerInputControl();
    table[Command::RetMixerOutputControl] = MixerOutputControl();
    table[Command::RetMixerInputControlValue] = MixerInputControlValue();
    table[Command::RetMixerOutputControlValue] = MixerOutputControlValue();
    table[Command::RetMixerMeterValue] = MixerMeterValue();
  }

  if (tableCreated) {
    if (contains(table, command)) {
      result = table.at(command);
    }
  }

  return result;
}

}  // namespace GeneSysLib
