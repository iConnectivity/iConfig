/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "DeviceInfo.h"

#include "AudioCfgInfo.h"
#include "AudioClockInfo.h"
#include "AudioInfo.h"
#include "AudioPortCfgInfo.h"
#include "AudioPortInfo.h"
#include "AudioPortPatchbay.h"

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

#include "CommandList.h"
#include "Device.h"
#include "EthernetPortInfo.h"
#include "GizmoCount.h"
#include "GizmoInfo.h"
#include "InfoList.h"
#include "MIDIInfo.h"
#include "MIDIPortInfo.h"
#include "MIDIPortDetail.h"
#include "MIDIPortFilter.h"
#include "MIDIPortRemap.h"
#include "MIDIPortRoute.h"
#include "MyAlgorithms.h"
#include "RTPMIDIConnectionDetail.h"
#include "ResetList.h"
#include "SaveRestoreList.h"
#include "USBHostMIDIDeviceDetail.h"

#ifndef Q_MOC_RUN
#include <boost/bind.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#endif

#include <numeric>
#include <QObject>
#include <QCryptographicHash>
#include <QMetaType>
#include <QSet>

using namespace GeneSysLib;
using namespace MyAlgorithms;
using namespace boost::adaptors;
using namespace boost::algorithm;
using namespace boost::assign;
using namespace boost::range;
using namespace boost::tuples;
using namespace boost;
using namespace std;

QMutex sysexMutex;
QMutex currentQueryMutex;
QMutex pendingQueriesMutex;
QMutex queriedItemsMutex;
QMutex attemptedQueriesMutex;

DeviceInfo::DeviceInfo(CommPtr _comm, QObject* _parent)
    : QObject(_parent),
      usbHostMIDIDeviceDetails(),
      storedCommandData(),
      deviceID(),
      transID(),
      queryScreen(UnknownScreen),
      currentQuery(),
      pendingQueries(),
      comm(_comm) {
  Q_ASSERT(_comm);
  registerAllHandlers();

  static bool registerHandlered = false;

  if (!registerHandlered) {
    qRegisterMetaType<CommandQList>("CommandQList");
    qRegisterMetaType<Screen>("Screen");
  }

  connect(_comm->timerThread.get(), SIGNAL(timedOut()), this, SLOT(timeout()));
}

DeviceInfo::DeviceInfo(CommPtr _comm, DeviceID _deviceID, Word _transID,
                       QObject* _parent)
    : QObject(_parent),
      usbHostMIDIDeviceDetails(),
      storedCommandData(),
      deviceID(_deviceID),
      transID(_transID),
      queryScreen(UnknownScreen),
      currentQuery(),
      pendingQueries(),
      comm(_comm) {
  Q_ASSERT(_comm);
  registerAllHandlers();

  static bool registerHandlered = false;

  if (!registerHandlered) {
    qRegisterMetaType<CommandQList>("CommandQList");
    qRegisterMetaType<Screen>("Screen");
  }

  connect(_comm->timerThread.get(), SIGNAL(timedOut()), this, SLOT(timeout()));
}

DeviceInfo::~DeviceInfo() {
  sysexMutex.lock();
  while (!sysexMessages.empty()) {
    sysexMessages.pop();
  }
  sysexMutex.unlock();
  unRegisterHandlerAllHandlers();
}

pair<DeviceID, Word> DeviceInfo::getInfo() const {
  return make_pair(deviceID, transID);
}

DeviceID DeviceInfo::getDeviceID() { return deviceID; }

Word DeviceInfo::getPID() { return deviceID.pid(); }

SerialNumber DeviceInfo::getSerialNumber() { return deviceID.serialNumber(); }

Word DeviceInfo::getTransID() { return transID; }

bool DeviceInfo::startQuery(Screen screen, const CommandQList& query) {
  bool result = false;

  currentQueryMutex.lock();
  if ((currentQuery.empty()) && (sysexMessages.empty())) {
    attemptedQueriesMutex.lock();
    attemptedQueries.clear();
    attemptedQueriesMutex.unlock();
    queriedItemsMutex.lock();
    queriedItems.clear();
    queriedItemsMutex.unlock();
    queryScreen = screen;

    // Queries before
    if (currentQuery.empty()) {
      currentQuery = query.toStdList();
    }
    else {
      pendingQueriesMutex.lock();
      pendingQueries.push(boost::make_tuple(screen, query.toStdList()));
      pendingQueriesMutex.unlock();
      currentQueryMutex.unlock();
      return result;
    }

    //printf("160: %d\n", sysexMessages.size());
    currentQueryMutex.unlock();
    result = sendNextSysex();
    if (screen != Screen::RereadAudioControls && screen != Screen::RereadMeters) {
      emit queryStarted();
    }
  } else {
    currentQueryMutex.unlock();
    pendingQueriesMutex.lock();
    pendingQueries.push(boost::make_tuple(screen, query.toStdList()));
    pendingQueriesMutex.unlock();
  }

  return result;
}

bool DeviceInfo::rereadAudioControls() {
  CommandQList query;

  query << Command::RetAudioControlDetailValue;

  return startQuery(Screen::RereadAudioControls, query);
}

bool DeviceInfo::rereadMixerControls() {
  CommandQList query;

  query << Command::RetMixerInputControlValue;
  query << Command::RetMixerOutputControlValue;

  return startQuery(Screen::RereadAudioControls, query);
}

bool DeviceInfo::rereadMeters() {

  CommandQList query;

  query << Command::RetAudioPortMeterValue;
  query << Command::RetMixerMeterValue;

  return startQuery(Screen::RereadMeters, query);
}

bool DeviceInfo::rereadAudioInfo() {

  CommandQList query;

  query << Command::RetAudioGlobalParm;
  query << Command::RetAudioClockParm;
  query << Command::RetMixerParm;
  query << Command::RetMixerPortParm;

  return startQuery(Screen::RereadAudioControls, query);
}

bool DeviceInfo::rereadStored() {

  CommandQList query;

  const auto& noDeviceNoCommandList = [](CmdEnum cmd) {
    return ((cmd != Command::RetDevice) && (cmd != Command::RetCommandList));
  }
  ;
  copy(storedCommandData | map_keys | transformed(bind(&keyToCommand, _1)) |
           uniqued | filtered(noDeviceNoCommandList),
       std::back_inserter(query));

  // we always have to reread the USBHost device details
  // clear this to get the updated list
  usbHostMIDIDeviceDetails.clear();
  query << Command::GetUSBHostMIDIDeviceDetail;

  return startQuery(Screen::RereadAllScreen, query);
}

bool DeviceInfo::contains(const commandDataKey_t& key) const {

  return MyAlgorithms::contains(storedCommandData, key);
}

#ifdef _WIN32
bool DeviceInfo::isNewWindowsDriver()
{
  return comm->isNewWindowsDriver();
}
#endif


void DeviceInfo::addCommandData(commandData_t commandData) {

  storedCommandData[commandData.key()] = commandData;
}

bool DeviceInfo::containsCommandDataType(CmdEnum command) const {

  bool value = storedCommandData.lower_bound(generateKey(command)) !=
               storedCommandData.end();
  return value;
}

bool DeviceInfo::containsCommandData(CmdEnum command) const {

  bool value = MyAlgorithms::contains(storedCommandData, generateKey(command));
  return value;
}

bool DeviceInfo::containsInfo(InfoIDEnum infoID) const {

  return MyAlgorithms::contains(storedCommandData,
                                generateKey(Command::RetInfo, infoID));
}

Info& DeviceInfo::infoData(InfoIDEnum infoID) {

  return storedCommandData.at(generateKey(Command::RetInfo, infoID))
      .get<Info>();
}

const Info& DeviceInfo::infoData(InfoIDEnum infoID) const {
  return storedCommandData.at(generateKey(Command::RetInfo, infoID))
      .get<Info>();
}

// MIDI Port Filters
MIDIPortFilter& DeviceInfo::midiPortFilter(
    Word portID, GeneSysLib::FilterIDEnum filterType) {

  return get<MIDIPortFilter>(MIDIPortFilter::queryKey(portID, filterType));
}

const MIDIPortFilter& DeviceInfo::midiPortFilter(
    Word portID, GeneSysLib::FilterIDEnum filterType) const {

  return get<MIDIPortFilter>(MIDIPortFilter::queryKey(portID, filterType));
}

// MIDI Port Remap
GeneSysLib::MIDIPortRemap& DeviceInfo::midiPortRemap(
    Word portID, GeneSysLib::RemapTypeEnum remapType) {

  return get<MIDIPortRemap>(MIDIPortRemap::queryKey(portID, remapType));
}

const GeneSysLib::MIDIPortRemap& DeviceInfo::midiPortRemap(
    Word portID, GeneSysLib::RemapTypeEnum remapType) const {

  return get<MIDIPortRemap>(MIDIPortRemap::queryKey(portID, remapType));
}

// Audio Port Info
size_t DeviceInfo::audioPortInfoCount() const {

  return typeCount<AudioPortInfo>();
}

void DeviceInfo::registerAllHandlers() {

  auto addHandler = [this](CmdEnum command, Handler handler) {
    this->registeredHandlerIDs[command] =
        comm->registerHandler(command, handler);
  }
  ;

  const auto& commonHandler =
      bind(&DeviceInfo::handleCommandData, this, _1, _2, _3, _4);
  addHandler(Command::RetAudioCfgInfo, commonHandler);
  addHandler(Command::RetAudioClockInfo, commonHandler);
  addHandler(Command::RetAudioInfo, commonHandler);
  addHandler(Command::RetAudioPortCfgInfo, commonHandler);
  addHandler(Command::RetAudioPortInfo, commonHandler);
  addHandler(Command::RetAudioPortPatchbay, commonHandler);
  addHandler(Command::RetCommandList, commonHandler);
  addHandler(Command::RetDevice, commonHandler);
  addHandler(Command::RetEthernetPortInfo, commonHandler);
  addHandler(Command::RetGizmoCount, commonHandler);
  addHandler(Command::RetGizmoInfo, commonHandler);
  addHandler(Command::RetInfo, commonHandler);
  addHandler(Command::RetInfoList, commonHandler);
  addHandler(Command::RetMIDIInfo, commonHandler);
  addHandler(Command::RetMIDIPortDetail, commonHandler);
  addHandler(Command::RetMIDIPortFilter, commonHandler);
  addHandler(Command::RetMIDIPortInfo, commonHandler);
  addHandler(Command::RetMIDIPortRemap, commonHandler);
  addHandler(Command::RetMIDIPortRoute, commonHandler);
  addHandler(Command::RetResetList, commonHandler);
  addHandler(Command::RetSaveRestoreList, commonHandler);
  addHandler(Command::RetRTPMIDIConnectionDetail, commonHandler);
  addHandler(Command::RetAudioGlobalParm, commonHandler);
  addHandler(Command::RetAudioPortParm, commonHandler);
  addHandler(Command::RetAudioDeviceParm, commonHandler);
  addHandler(Command::RetAudioControlParm, commonHandler);
  addHandler(Command::RetAudioControlDetail, commonHandler);
  addHandler(Command::RetAudioControlDetailValue, commonHandler);
  addHandler(Command::RetAudioClockParm, commonHandler);
  addHandler(Command::RetAudioPatchbayParm, commonHandler);
  addHandler(Command::RetAudioChannelName, commonHandler);
  addHandler(Command::RetAudioPortMeterValue, commonHandler);
  addHandler(Command::RetMixerParm, commonHandler);
  addHandler(Command::RetMixerPortParm, commonHandler);
  addHandler(Command::RetMixerInputParm, commonHandler);
  addHandler(Command::RetMixerOutputParm, commonHandler);
  addHandler(Command::RetMixerInputControl, commonHandler);
  addHandler(Command::RetMixerOutputControl, commonHandler);
  addHandler(Command::RetMixerInputControlValue, commonHandler);
  addHandler(Command::RetMixerOutputControlValue, commonHandler);
  addHandler(Command::RetMixerMeterValue, commonHandler);

  auto midiHostDeviceHandler = bind(
      &DeviceInfo::handleUSBHostMIDIDeviceDetailData, this, _1, _2, _3, _4);
  addHandler(Command::RetUSBHostMIDIDeviceDetail, midiHostDeviceHandler);
}

void DeviceInfo::unRegisterHandlerAllHandlers() {

  for (auto handler : registeredHandlerIDs) {
    comm->unRegisterHandler(handler.first, handler.second);
  }
}

void DeviceInfo::timeout() {

  //printf("timed out!\n");
  sysexMutex.lock();
  while (!sysexMessages.empty()) {
    sysexMessages.pop();
  }
  sysexMutex.unlock();
  currentQueryMutex.lock();
  currentQuery.clear();
  currentQueryMutex.unlock();
  pendingQueriesMutex.lock();
  while (!pendingQueries.empty()) {
    pendingQueries.pop();
  }
  pendingQueriesMutex.unlock();
  queriedItemsMutex.lock();
  queriedItems.clear();
  queriedItemsMutex.unlock();
  attemptedQueriesMutex.lock();
  attemptedQueries.clear();
  attemptedQueriesMutex.unlock();
  queryScreen = UnknownScreen;
}

void DeviceInfo::addCommand(const Bytes& _sysex) {

//  sysexMutex.lock();
  sysexMessages.push(_sysex);
//  sysexMutex.unlock();
}

void DeviceInfo::addCommand(const Bytes&& _sysex) {

//  sysexMutex.lock();
  sysexMessages.push(_sysex);
//  sysexMutex.unlock();
}

bool DeviceInfo::sendNextSysex() {


  // is the sysex message queue empty?

  //printf("0: sysexMessages.size(): %d, currentQuery.size(): %d (%d)\n", sysexMessages.size(), currentQuery.size(), QThread::currentThreadId());

  sysexMutex.lock();
  bool send = !(sysexMessages.empty());

  if (send) {
    // send the next sysex message
    //printf("presend\n");
    comm->sendSysex(sysexMessages.front());
    //printf("postsend\n");

    // remove the sysex message from the message queue
    sysexMessages.pop();
  }
  sysexMutex.unlock();

  if (send) {
    //if (sysexMessages.empty())
      return true;
    //else
    //  return sendNextSysex();
  }

  //printf("1: sysexMessages.size(): %d, currentQuery.size(): %d (%d)\n", sysexMessages.size(), currentQuery.size(), QThread::currentThreadId());

  if (!send) {
    currentQueryMutex.lock();
    // get the front iterator for the current query
    auto q = currentQuery.begin();

    // loop through the current query
    while (q != currentQuery.end()) {
      auto D = commandDependancy(*q);

      bool metDependancies = true;
      for (auto d : D) {
        if (!containsCommandDataType(d)) {
          attemptedQueriesMutex.lock();
          if (!MyAlgorithms::contains(attemptedQueries, d)) {
            metDependancies = false;
          }
          attemptedQueriesMutex.unlock();
        }
      }

      // if all dependancies are met
      if ((D.empty()) || metDependancies) {

        // add the query sysex to the sysex buffer
        addQuerySysex(*q);

        // add query to the attempted queries
        attemptedQueriesMutex.lock();
        attemptedQueries.insert(*q);
        attemptedQueriesMutex.unlock();

        // remove query from list of pending commands and increment pointer

        //printf("2: currentQuery.size(): %d, q: %d (%d)\n", currentQuery.size(), q, QThread::currentThreadId());
        q = currentQuery.erase(q);
        currentQueryMutex.unlock();
        //printf("438\n");
        return sendNextSysex();
      } else {
        // dependencies not met
        // foreach dependencies
        for (auto d : D) {
          // if dependancy isn't in current query and dependancies is not
          // alrealy being queried and dependancies has not been attempted
          attemptedQueriesMutex.lock();
          if ((!MyAlgorithms::contains(currentQuery, d)) &&
              (!containsCommandDataType(d)) &&
              (!MyAlgorithms::contains(attemptedQueries, d))) {
            // add dependancy to current query
            currentQuery.push_front(d); //push_back
          }
          else if (MyAlgorithms::contains(currentQuery, d)) {
            currentQuery.remove(d);
            currentQuery.push_front(d);
          }
          attemptedQueriesMutex.unlock();
        }

        q = currentQuery.begin();
      }
    }

    currentQueryMutex.unlock();

    // if there are not pending sysex messages
    if (sysexMessages.empty()) {
      // we are done with the current query

      // post the current query
      queriedItemsMutex.lock();
      queriedItems = queriedItems.toSet().toList();
      queriedItemsMutex.unlock();

      if (queryScreen != UnknownScreen) {
        emit queryCompleted(queryScreen, queriedItems);
        currentQueryMutex.lock();
        currentQuery.clear();
        currentQueryMutex.unlock();
        queryScreen = UnknownScreen;
      }

      // if there are pending queries then deal with them now
      if (!pendingQueries.empty()) {
        pendingQueriesMutex.lock();
        // get next query
        auto nextQuery = pendingQueries.front();

        // remove the next query from the pending list
        pendingQueries.pop();
        pendingQueriesMutex.unlock();

        // set the query Screen
        queryScreen = boost::get<0>(nextQuery);

        // set the currentQuery to the next query
        currentQueryMutex.lock();
        currentQuery = boost::get<1>(nextQuery);
        currentQueryMutex.unlock();

        // clear the list of queried items
        queriedItemsMutex.lock();
        queriedItems.clear();
        queriedItemsMutex.unlock();

        // start the next query
        //printf("504\n");
        send = sendNextSysex();
      }
    } else {
      // there are pending sysex messages
      sysexMutex.lock();
      // send the next sysex message
      comm->sendSysex(sysexMessages.front());
      // remove the sent sysex message from the queue
      sysexMessages.pop();
      sysexMutex.unlock();

      // we have sent a sysex message
      send = true;

      //emit queryStarted();
    }
  }

  return send;
}

bool DeviceInfo::commonHandleCode(DeviceID _deviceID, Word _transID) {

  if ((deviceID.serialNumber() == SerialNumber()) ||
      (deviceID.pid() == Word()) || (transID == Word())) {
    deviceID = _deviceID;
    transID = _transID;
  }

  return (deviceID == _deviceID) && (transID == _transID);
}

void DeviceInfo::handleCommandData(CmdEnum _command, DeviceID _deviceID,
                                   Word _transID, commandData_t _commandData) {

  if (commonHandleCode(_deviceID, _transID)) {
    auto key = _commandData.key();
    storedCommandData[key] = _commandData;

    queriedItemsMutex.lock();
    queriedItems.push_back(_command);
    queriedItemsMutex.unlock();

    //printf("543\n");
    sendNextSysex();
  }
}

void DeviceInfo::handleUSBHostMIDIDeviceDetailData(CmdEnum _command,
                                                   DeviceID _deviceID,
                                                   Word _transID,
                                                   commandData_t _commandData) {

  if (commonHandleCode(_deviceID, _transID)) {
    auto& foundUSBDetails = _commandData.get<USBHostMIDIDeviceDetail>();

    auto foundItem = find_if(usbHostMIDIDeviceDetails,
                             [ = ](USBHostMIDIDeviceDetail usbDetails) {
      return ((usbDetails.usbHostJack() == foundUSBDetails.usbHostJack()) &&
              (usbDetails.usbHostID() == foundUSBDetails.usbHostID()));
    });

    if ((foundItem == usbHostMIDIDeviceDetails.end() &&
         ((foundUSBDetails.numMIDIIn() > 0) ||
          (foundUSBDetails.numMIDIOut() > 0)))) {
      usbHostMIDIDeviceDetails.push_back(foundUSBDetails);
      queriedItemsMutex.lock();
      queriedItems.append(_command);
      queriedItemsMutex.unlock();
    }

    //printf("568\n");
    sendNextSysex();
  }
}

void DeviceInfo::handleACKData(CmdEnum, DeviceID, Word, commandData_t) {

  if (!sysexMessages.empty()) {
    emit writingProgress(maxWriteItems - sysexMessages.size());
    //printf("576\n");
    sendNextSysex();
  } else {
    comm->unRegisterExclusiveHandler();
    emit writeCompleted();
  }
}

void DeviceInfo::addQuerySysex(CmdEnum command) {

  sysexMutex.lock();
  static int count = 0;

  const auto& commandList =
      contains<CommandList>() ? get<CommandList>() : CommandList();

  const auto& midiInfo = contains<MIDIInfo>() ? get<MIDIInfo>() : MIDIInfo();

  const auto& audioInfo =
      contains<AudioInfo>() ? get<AudioInfo>() : AudioInfo();

  const auto& gizmoCount =
      contains<GizmoCount>() ? get<GizmoCount>() : GizmoCount();

  const auto& audioGlobalParm =
      contains<AudioGlobalParm>() ? get<AudioGlobalParm>() : AudioGlobalParm();

  // At this point all dependancies are met
  switch (command) {
  case Command::GetDevice:
  case Command::RetDevice: {
    addCommand<GetDeviceCommand>();
    break;
  }

  case Command::GetCommandList:
  case Command::RetCommandList: {
    addCommand<GetDeviceCommand>();
    break;
  }

  case Command::GetInfoList:
  case Command::RetInfoList: {
    if (commandList.contains(Command::GetInfoList)) {
      addCommand<GetInfoListCommand>();
    }
    break;
  }

  case Command::GetInfo:
  case Command::RetInfo: {
    if ((commandList.contains(Command::GetInfo)) && (contains<InfoList>())) {
      auto& infoListData = get<InfoList>();

      infoListData.for_each([&](const InfoList::InfoRecord & infoRecord) {
        addCommand<GetInfoCommand>(infoRecord.infoID());
      });
    }
    break;
  }

  case Command::GetResetList:
  case Command::RetResetList: {
    if (commandList.contains(Command::GetResetList)) {
      addCommand<GetResetListCommand>();
    }
    break;
  }

  case Command::GetSaveRestoreList:
  case Command::RetSaveRestoreList: {
    if (commandList.contains(Command::GetSaveRestoreList)) {
      addCommand<GetSaveRestoreListCommand>();
    }
    break;
  }

  case Command::GetEthernetPortInfo:
  case Command::RetEthernetPortInfo: {
    if (commandList.contains(Command::GetEthernetPortInfo)) {
      for (auto portID = 1; portID <= midiInfo.numEthernetJacks(); ++portID) {
        addCommand<GetEthernetPortInfoCommand>(portID);
      }
    }
    break;
  }

  case Command::GetGizmoCount:
  case Command::RetGizmoCount: {
    if (commandList.contains(Command::GetGizmoCount)) {
      addCommand<GetGizmoCountCommand>();
    }
    break;
  }

  case Command::GetGizmoInfo:
  case Command::RetGizmoInfo: {
    if (commandList.contains(Command::GetGizmoInfo)) {
      for (Word i = 1; i <= gizmoCount.gizmoCount(); ++i) {
        addCommand<GetGizmoInfoCommand>(i);
      }
    }
    break;
  }

    // MIDI Related
  case Command::GetMIDIInfo:
  case Command::RetMIDIInfo: {
    if (commandList.contains(Command::GetMIDIInfo)) {
      addCommand<GetMIDIInfoCommand>();
    }
    break;
  }

  case Command::GetMIDIPortInfo:
  case Command::RetMIDIPortInfo: {
    if (commandList.contains(Command::GetMIDIPortInfo)) {
      for (auto portID = 1; portID <= midiInfo.numMIDIPorts(); ++portID) {
        addCommand<GetMIDIPortInfoCommand>(portID);
      }
    }
    break;
  }

  case Command::GetMIDIPortFilter:
  case Command::RetMIDIPortFilter: {
    if (commandList.contains(Command::GetMIDIPortFilter)) {
      for (auto portID = 1; portID <= midiInfo.numMIDIPorts(); ++portID) {
        // Input filter
        addCommand<GetMIDIPortFilterCommand>(portID, FilterID::InputFilter);

        // Output filter
        addCommand<GetMIDIPortFilterCommand>(portID, FilterID::OutputFilter);
      }
    }
    break;
  }

  case Command::GetMIDIPortRemap:
  case Command::RetMIDIPortRemap: {
    if (commandList.contains(Command::GetMIDIPortRemap)) {
      for (auto portID = 1; portID <= midiInfo.numMIDIPorts(); ++portID) {
        // Input remap
        addCommand<GetMIDIPortRemapCommand>(portID, RemapID::InputRemap);

        // Output remap
        addCommand<GetMIDIPortRemapCommand>(portID, RemapID::OutputRemap);
      }
    }
    break;
  }

  case Command::GetMIDIPortRoute:
  case Command::RetMIDIPortRoute: {
    if (commandList.contains(Command::GetMIDIPortRoute)) {
      for (auto portID = 1; portID <= midiInfo.numMIDIPorts(); ++portID) {
        addCommand<GetMIDIPortRouteCommand>(portID);
      }
    }
    break;
  }

  case Command::GetMIDIPortDetail:
  case Command::RetMIDIPortDetail: {
    if (commandList.contains(Command::GetMIDIPortDetail)) {
      for (auto portID = 1; portID <= midiInfo.numMIDIPorts(); ++portID) {
        addCommand<GetMIDIPortDetailCommand>(portID);
      }
    }
    break;
  }

  case Command::GetUSBHostMIDIDeviceDetail:
  case Command::RetUSBHostMIDIDeviceDetail: {
    for (auto jackID = 1; jackID <= midiInfo.numUSBHostJacks(); ++jackID) {
      for (auto hostID = 1; hostID <= midiInfo.numUSBMIDIPortPerHostJack();
           ++hostID) {
        addCommand<GetUSBHostMIDIDeviceDetailCommand>(jackID, hostID);
      }
    }
    break;
  }

  case Command::GetRTPMIDIConnectionDetail:
  case Command::RetRTPMIDIConnectionDetail: {
    if (commandList.contains(Command::GetRTPMIDIConnectionDetail)) {
      auto startPort = midiInfo.numDINPairs() +
                       midiInfo.numUSBDeviceJacks() *
                       midiInfo.numUSBMIDIPortPerDeviceJack() +
                       midiInfo.numUSBHostJacks() *
                       midiInfo.numUSBMIDIPortPerHostJack() + 1;
      for (Byte portID = startPort; portID <= midiInfo.numMIDIPorts();
           ++portID) {
        for (auto connID = 1;
             connID <= midiInfo.numRTPMIDIConnectionsPerSession(); ++connID) {
          addCommand<GetRTPMIDIConnectionDetailCommand>(portID, connID);
        }
      }
    }
  }

    // Audio Related
  case Command::GetAudioInfo:
  case Command::RetAudioInfo: {
    if (commandList.contains(Command::GetAudioInfo)) {
      addCommand<GetAudioInfoCommand>();
    }
    break;
  }

  case Command::GetAudioCfgInfo:
  case Command::RetAudioCfgInfo: {
    if (commandList.contains(Command::GetAudioCfgInfo)) {
      addCommand<GetAudioCfgInfoCommand>();
    }
    break;
  }

  case Command::GetAudioPortInfo:
  case Command::RetAudioPortInfo: {
    if (commandList.contains(Command::GetAudioPortInfo)) {
      for (auto portID = 1; portID <= audioInfo.numberOfAudioPorts();
           ++portID) {
        addCommand<GetAudioPortInfoCommand>(portID);
      }
    }
    break;
  }

  case Command::GetAudioPortCfgInfo:
  case Command::RetAudioPortCfgInfo: {
    if (commandList.contains(Command::GetAudioPortCfgInfo)) {
      for (auto portID = 1; portID <= audioInfo.numberOfAudioPorts();
           ++portID) {
        addCommand<GetAudioPortCfgInfoCommand>(portID);
      }
    }
    break;
  }

  case Command::GetAudioPortPatchbay:
  case Command::RetAudioPortPatchbay: {
    if (commandList.contains(Command::GetAudioPortPatchbay)) {
      for (auto portID = 1; portID <= audioInfo.numberOfAudioPorts();
           ++portID) {
        addCommand<GetAudioPortPatchbayCommand>(portID);
      }
    }

    break;
  }

  case Command::GetAudioClockInfo:
  case Command::RetAudioClockInfo: {
    if (commandList.contains(Command::GetAudioClockInfo)) {
      addCommand<GetAudioClockInfoCommand>();
    }
    break;
  }

    // Audio V2
  case Command::GetAudioGlobalParm:
  case Command::RetAudioGlobalParm: {
    if (commandList.contains(Command::GetAudioGlobalParm)) {
      addCommand<GetAudioGlobalParmCommand>();
    }
    break;
  }

  case Command::GetAudioPortParm:
  case Command::RetAudioPortParm: {
    if (commandList.contains(Command::GetAudioPortParm)) {
      for (Word audioPortID = 1;
           audioPortID <= audioGlobalParm.numAudioPorts(); ++audioPortID) {
        addCommand<GetAudioPortParmCommand>(audioPortID);
      }
    }
    break;
  }

  case Command::GetAudioDeviceParm:
  case Command::RetAudioDeviceParm: {
    if (commandList.contains(Command::GetAudioDeviceParm)) {
      for (Word audioPortID = 1;
           audioPortID <= audioGlobalParm.numAudioPorts(); ++audioPortID) {
        addCommand<GetAudioDeviceParmCommand>(audioPortID);
      }
    }
    break;
  }

  case Command::GetAudioControlParm:
  case Command::RetAudioControlParm: {
    if (commandList.contains(Command::GetAudioControlParm)) {
      for (Word audioPortID = 1;
           audioPortID <= audioGlobalParm.numAudioPorts(); ++audioPortID) {
        const auto& audioDeviceParm = get<AudioDeviceParm>(audioPortID);

        for (auto controllerNumber = 1;
             controllerNumber <= audioDeviceParm.maxControllers();
             ++controllerNumber) {
          addCommand<GetAudioControlParmCommand>(audioPortID,
                                                 controllerNumber);
        }
      }
    }
    break;
  }

  case Command::GetAudioControlDetail:
  case Command::RetAudioControlDetail: {
    if (commandList.contains(Command::GetAudioControlDetail)) {
      for_each<AudioControlParm>(
            [&](const AudioControlParm & audioControlParm) {
        for (Byte detailID = 1; detailID <= audioControlParm.numDetails();
             ++detailID) {
          addCommand<GetAudioControlDetailCommand>(
                audioControlParm.audioPortID(),
                audioControlParm.controllerNumber(), detailID);
        }
      });
    }
    break;
  }

  case Command::GetAudioControlDetailValue:
  case Command::RetAudioControlDetailValue: {
    if (commandList.contains(Command::GetAudioDeviceParm)) {
      for_each<AudioControlParm>(
            [&](const AudioControlParm & audioControlParm) {
        for (Byte detailID = 1; detailID <= audioControlParm.numDetails();
             ++detailID) {
          addCommand<GetAudioControlDetailValueCommand>(
                audioControlParm.audioPortID(),
                audioControlParm.controllerNumber(), detailID);
        }
      });
    }
    break;
  }

  case Command::GetAudioClockParm:
  case Command::RetAudioClockParm: {
    if (commandList.contains(Command::GetAudioClockParm)) {
      addCommand<GetAudioClockParmCommand>();
    }
    break;
  }

  case Command::GetAudioPatchbayParm:
  case Command::RetAudioPatchbayParm: {
    if (commandList.contains(Command::GetAudioPatchbayParm)) {
      for (Word audioPortID = 1;
           audioPortID <= audioGlobalParm.numAudioPorts(); ++audioPortID) {
        addCommand<GetAudioPatchbayParmCommand>(audioPortID);
      }
    }
    break;
  }

  case Command::GetAudioPortMeterValue:
  case Command::RetAudioPortMeterValue: {
    if (commandList.contains(Command::GetAudioDeviceParm)) {
      for (Word audioPortID = 1; audioPortID <= audioGlobalParm.numAudioPorts(); ++audioPortID) {
        addCommand<GetAudioPortMeterValueCommand>(
              audioPortID);
      }
    }
    break;
  }

    // Mixer

  case Command::GetMixerPortParm:
  case Command::RetMixerPortParm: {
    if (commandList.contains(Command::GetMixerPortParm)) {
      addCommand<GetMixerPortParmCommand>();
    }
    break;
  }

  case Command::GetMixerParm:
  case Command::RetMixerParm: {
    if (commandList.contains(Command::GetMixerParm)) {
      for (Byte audioConfigurationNumber = 1; audioConfigurationNumber <= audioGlobalParm.numConfigBlocks(); ++audioConfigurationNumber) {
        addCommand<GetMixerParmCommand>(
              audioConfigurationNumber);
      }
    }
    break;
  }

  case Command::GetMixerInputParm:
  case Command::RetMixerInputParm: {
   //bugfixing: Fixing the C++ assertion caused by failing type checking
   //--zx, 2016-06-08
      // const auto& audioGlobalParm = get<AudioGlobalParm>();
      //const auto& mixerParm = get<MixerParm>(audioGlobalParm.currentActiveConfig());
   const auto& audioGlobalParm =
          contains<AudioGlobalParm>() ? get<AudioGlobalParm>() : AudioGlobalParm();
   const auto& mixerParm = contains<MixerParm>() ? get<MixerParm>(audioGlobalParm.currentActiveConfig()) : MixerParm();

   const auto& mixerPortParm = contains<MixerPortParm>() ? get<MixerPortParm>() : MixerPortParm();

    if (commandList.contains(Command::GetMixerInputParm)) {
        for (Byte audioPortID = 1; audioPortID <= mixerPortParm.audioPortMixerBlockCount(); ++audioPortID) {
          for (Byte mixerInputNumber = 1; mixerInputNumber <= mixerPortParm.audioPortMixerBlocks.at(audioPortID - 1).numInputs(); ++mixerInputNumber) {
            addCommand<GetMixerInputParmCommand>(
                  audioPortID,
                  mixerInputNumber);
          }
        }
      };
    break;
  }

  case Command::GetMixerOutputParm:
  case Command::RetMixerOutputParm: {
      //bugfixing: Fixing the C++ assertion caused by failing type checking
      //--zx, 2016-06-08
    //const auto& audioGlobalParm = get<AudioGlobalParm>();
    //const auto& mixerParm = get<MixerParm>(audioGlobalParm.currentActiveConfig());
      const auto& audioGlobalParm =
             contains<AudioGlobalParm>() ? get<AudioGlobalParm>() : AudioGlobalParm();
      const auto& mixerParm = contains<MixerParm>() ? get<MixerParm>(audioGlobalParm.currentActiveConfig()) : MixerParm();


    const auto& mixerPortParm = contains<MixerPortParm>() ? get<MixerPortParm>() : MixerPortParm();

    if (commandList.contains(Command::GetMixerOutputParm)) {
        for (Byte audioPortID = 1; audioPortID <= mixerPortParm.audioPortMixerBlockCount(); ++audioPortID) {
          for (Byte mixerOutputNumber = 1; mixerOutputNumber <= mixerPortParm.audioPortMixerBlocks.at(audioPortID - 1).numOutputs(); ++mixerOutputNumber) {
            addCommand<GetMixerOutputParmCommand>(
                  audioPortID,
                  mixerOutputNumber);
          }
        }
      };
    break;  }

  case Command::GetMixerInputControl:
  case Command::RetMixerInputControl: {
    const auto& mixerPortParm = contains<MixerPortParm>() ? get<MixerPortParm>() : MixerPortParm();

    if (commandList.contains(Command::GetMixerInputControl)) {
        for (Byte audioPortID = 1; audioPortID <= mixerPortParm.audioPortMixerBlockCount(); ++audioPortID) {
          addCommand<GetMixerInputControlCommand>(
                audioPortID);
        }
      };
    break;
  }

  case Command::GetMixerOutputControl:
  case Command::RetMixerOutputControl: {
    const auto& mixerPortParm = contains<MixerPortParm>() ? get<MixerPortParm>() : MixerPortParm();

    if (commandList.contains(Command::GetMixerOutputControl)) {
        for (Byte audioPortID = 1; audioPortID <= mixerPortParm.audioPortMixerBlockCount(); ++audioPortID) {
          addCommand<GetMixerOutputControlCommand>(
                audioPortID);
        }
      };
    break;
  }


  case Command::GetMixerInputControlValue:
  case Command::RetMixerInputControlValue: {
    const auto& mixerPortParm = contains<MixerPortParm>() ? get<MixerPortParm>() : MixerPortParm();

    if (commandList.contains(Command::GetMixerInputControlValue)) {
      for (Byte audioPortID = 1; audioPortID <= mixerPortParm.audioPortMixerBlockCount(); ++audioPortID) {
        for (Byte mixerOutputNumber = 1; mixerOutputNumber <= mixerPortParm.audioPortMixerBlocks.at(audioPortID - 1).numOutputs(); ++mixerOutputNumber) {
          for (Byte mixerInputNumber = 1; mixerInputNumber <= mixerPortParm.audioPortMixerBlocks.at(audioPortID - 1).numInputs(); ++mixerInputNumber) {
            addCommand<GetMixerInputControlValueCommand>(
                audioPortID, mixerOutputNumber, mixerInputNumber);
          }
        }
      }
    };

    break;
  }

  case Command::GetMixerOutputControlValue:
  case Command::RetMixerOutputControlValue: {
    const auto& mixerPortParm = contains<MixerPortParm>() ? get<MixerPortParm>() : MixerPortParm();

    if (commandList.contains(Command::GetMixerInputControlValue)) {
      for (Byte audioPortID = 1; audioPortID <= mixerPortParm.audioPortMixerBlockCount(); ++audioPortID) {
        for (Byte mixerOutputNumber = 1; mixerOutputNumber <= mixerPortParm.audioPortMixerBlocks.at(audioPortID - 1).numOutputs(); ++mixerOutputNumber) {
          addCommand<GetMixerOutputControlValueCommand>(
              audioPortID, mixerOutputNumber);
        }
      }
    };


    break;
  }

  case Command::GetMixerMeterValue:
  case Command::RetMixerMeterValue: {
    const auto& mixerPortParm = contains<MixerPortParm>() ? get<MixerPortParm>() : MixerPortParm();

    if (commandList.contains(Command::GetMixerInputControlValue)) {
      for (Byte audioPortID = 1; audioPortID <= mixerPortParm.audioPortMixerBlockCount(); ++audioPortID) {
        for (Byte mixerOutputNumber = 1; mixerOutputNumber <= mixerPortParm.audioPortMixerBlocks.at(audioPortID - 1).numOutputs(); ++mixerOutputNumber) {
          addCommand<GetMixerMeterValueCommand>(
              audioPortID, mixerOutputNumber);
        }
      }
    }
    break;
  }

  default:
    // do nothing
    break;
  }
  sysexMutex.unlock();

}

Bytes DeviceInfo::serialize() {
  Bytes result;

  // Add Magic Number
  result += 0x69, 0x43, 0x4D;

  // Add Model Number
  result += deviceID.pid();

  // Add Version Number
  result += 0x01;

  for (const auto& cmdPair : storedCommandData) {
    push_back(result, generate(keyToCommand(cmdPair.first), cmdPair.second));
  }

  auto hash = QCryptographicHash::hash(
      QByteArray::fromRawData((char*)result.data(), result.size()),
      QCryptographicHash::Md5);
  push_back(result, hash);

  return result;
}

Bytes DeviceInfo::serialize2(std::set<Command::Enum> commandsToSave, QString description) {
  Bytes result;

  // Add Magic Number
  result += 0x69, 0x43, 0x4D;

  // Add Model Number
  result += deviceID.pid();

  // Add Version Number
  result += 0x02;

  description = description.mid(0,239);
  result += (unsigned char) description.size();
  for (int x = 0; x < description.size(); x++) {
    result += description.at(x).toAscii();
  }

  for (const auto& cmdPair : storedCommandData) {
    if (commandsToSave.find(keyToCommand(cmdPair.first)) != commandsToSave.end()) { // if we're supposed to save it
      Bytes toWrite = generate(keyToCommand(cmdPair.first), cmdPair.second);
      toWrite[7] = toWrite[8] = toWrite[9] = toWrite[10] = toWrite[11] = 0;
      toWrite[toWrite.size() - 2] = (~(accumulate(toWrite.begin() + 5, toWrite.end() - 2, 0x00)) + 1) & 0x7F;

      push_back(result, toWrite);
      std::cout << "writing: ";
      for( Bytes::const_iterator i = toWrite.begin(); i != toWrite.end(); ++i)
          std::cout << std::hex << (int)*i << ' ';
      std::cout << '\n';
    }
  }

  auto hash = QCryptographicHash::hash(
      QByteArray::fromRawData((char*)result.data(), result.size()),
      QCryptographicHash::Md5);
  push_back(result, hash);

  return result;
}

void DeviceInfo::replaceChecksumByte(unsigned char *arr, size_t size) {
  int acc = 0;
  for (size_t i = 5; i < size - 2; i++) {
    acc += arr[i];
  }
  arr[size-2] = (~(acc) + 1) & 0x7F;
}

Bytes DeviceInfo::serialize2midi(std::set<Command::Enum> commandsToSave, bool reboot) {
  Bytes result;

  for (const auto& cmdPair : storedCommandData) {
    if (commandsToSave.find(keyToCommand(cmdPair.first)) != commandsToSave.end()) { // if we're supposed to save it
      Bytes toWrite = generate(keyToCommand(cmdPair.first), cmdPair.second);
      toWrite[7] = toWrite[8] = toWrite[9] = toWrite[10] = toWrite[11] = 0;
      toWrite[14] = 0x40;
      toWrite[toWrite.size() - 2] = (~(accumulate(toWrite.begin() + 5, toWrite.end() - 2, 0x00)) + 1) & 0x7F; //redo checksum
      toWrite.insert(toWrite.begin() + 1, toWrite.size() - 1);
      result += 1;
      push_back(result, toWrite);
      std::cout << "writing midi: ";
      for( Bytes::const_iterator i = toWrite.begin(); i != toWrite.end(); ++i)
          std::cout << std::hex << (int)*i << ' ';
      std::cout << '\n';
    }
  }
  if (reboot) {
    static unsigned char arr[] = {0xF0, 0x00, 0x01, 0x73, 0x7E, 0x00, (unsigned char) (deviceID.pid() & 0xFF), 0x00, 0x00, 0x00, 0x00, 0x00,
                                        0x00, 0x01, 0x40, 0x11, 0x00, 0x01, 0x01, 0x00, 0xF7}; // SaveToFlash
    replaceChecksumByte(arr, sizeof(arr) / sizeof(arr[0]));
    Bytes toWrite(arr, arr + sizeof(arr) / sizeof(arr[0]) );
    toWrite.insert(toWrite.begin() + 1, toWrite.size() - 1);
    result += 1;
    push_back(result, toWrite);
    std::cout << "writing midi: ";
    for( Bytes::const_iterator i = toWrite.begin(); i != toWrite.end(); ++i)
        std::cout << std::hex << (int)*i << ' ';
    std::cout << '\n';
    static unsigned char arr2[] = {0xF0, 0x00, 0x01, 0x73, 0x7E, 0x00, (unsigned char) (deviceID.pid() & 0xFF), 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x01, 0x40, 0x10, 0x00, 0x01, 0x01, 0x00, 0xF7}; // Reset
    replaceChecksumByte(arr2, sizeof(arr2) / sizeof(arr2[0]));
    Bytes toWrite2(arr2, arr2 + sizeof(arr2) / sizeof(arr2[0]) );
    toWrite2.insert(toWrite2.begin() + 1, toWrite2.size() - 1);
    result += 10;
    push_back(result, toWrite2);
    std::cout << "writing midi: ";
    for( Bytes::const_iterator i = toWrite2.begin(); i != toWrite2.end(); ++i)
        std::cout << std::hex << (int)*i << ' ';
    std::cout << '\n';
  }

  return result;
}

bool DeviceInfo::deserialize(Bytes data) {

  bool valid = true;
  auto start = data.begin();
  auto finish = data.end();

  valid = (data.size() > 21);

  if (valid) {
    Bytes magicStart;
    // Verify the header
    magicStart += 0x69, 0x43, 0x4D;

    // Verify PID
    magicStart += deviceID.pid();

    // Version Number
    magicStart += 0x01;

    Bytes header;
    copy(start, start + 5, std::back_inserter(header));

    valid = (header == magicStart);
  }

  if (!valid) { // try version 2 of a save file.
    Bytes magicStart;
    // Verify the header
    magicStart += 0x69, 0x43, 0x4D;

    // Verify PID
    magicStart += deviceID.pid();

    // Version Number
    magicStart += 0x02;

    Bytes header;
    copy(start, start + 5, std::back_inserter(header));

    valid = (header == magicStart);

    if (!valid) {
//      return deserialize2(data);
//    }
//    else {
      return valid;
    }
  }

  // Verify the footer
  if (valid) {
    BytesIter md5Iter = finish - 16;

    auto hashAlgorithm = boost::shared_ptr<QCryptographicHash>(
        new QCryptographicHash(QCryptographicHash::Md5));
    hashAlgorithm->addData((char*)(data.data()), data.size() - 16);

    // check the hash
    auto hash = hashAlgorithm->result();
    valid = std::equal(boost::begin(hash), boost::end(hash), md5Iter, [](Byte a, Byte b) {
      return a == b;
    });
  }

  if (valid) {
    DeviceID storedDeviceID = deviceID;
    Word storedTransID = transID;

    //storedCommandData.clear();
    //usbHostMIDIDeviceDetails.clear();

    comm->parseBytes(start, finish, deviceID);

    deviceID = storedDeviceID;
    transID = storedTransID;

    auto ackHandler = bind(&DeviceInfo::handleACKData, this, _1, _2, _3, _4);
    comm->registerExclusiveHandler(Command::ACK, ackHandler);
    writeAll();
  }

  return valid;
}

long DeviceInfo::registerHandler(CmdEnum commandID, Handler handler) {

  Q_ASSERT(comm);
  return comm->registerHandler(commandID, handler);
}

void DeviceInfo::unRegisterHandler(CmdEnum commandID) {

  Q_ASSERT(comm);
  comm->unRegisterHandler(commandID);
}

void DeviceInfo::unRegisterHandler(CmdEnum commandID, long handlerID) {

  Q_ASSERT(comm);
  comm->unRegisterHandler(commandID, handlerID);
}

void DeviceInfo::unRegisterAll() {

  Q_ASSERT(comm);
  comm->unRegisterAll();
}

void DeviceInfo::registerExclusiveHandler(CmdEnum commandID, Handler handler) {

  Q_ASSERT(comm);
  comm->registerExclusiveHandler(commandID, handler);
}

void DeviceInfo::unRegisterExclusiveHandler() {

  Q_ASSERT(comm);
  comm->unRegisterExclusiveHandler();
}

void DeviceInfo::writeAll() {

  for (const auto& cmdData : storedCommandData) {
    sysexMutex.lock();
    addCommand(generate((CmdEnum)(WRITE_BIT | keyToCommandID(cmdData.first)),
                        cmdData.second));
    sysexMutex.unlock();

  }

  maxWriteItems = sysexMessages.size();
  emit writingStarted(maxWriteItems);
  //printf("1207\n");
  sendNextSysex();
}

Bytes DeviceInfo::generate(CmdEnum command,
                           const commandData_t& cmdData) const {
  return ::generate(deviceID, transID, command, cmdData);
}
