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
#include "AudioClockParm.h"
#include "AudioPatchbayParm.h"

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
#import "ICRunOnMain.h"

#include <boost/bind.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/algorithm/find_if.hpp>

#import <CommonCrypto/CommonDigest.h>
#include <boost/tuple/tuple.hpp>

using namespace GeneSysLib;
using namespace MyAlgorithms;
using namespace boost::adaptors;
using namespace boost::algorithm;
using namespace boost::assign;
using namespace boost::range;
using namespace boost::tuples;
using namespace boost;
using namespace std;

DeviceInfo::DeviceInfo(CommPtr _comm)
    : usbHostMIDIDeviceDetails(),
      storedCommandData(),
      deviceID(),
      transID(),
      queryScreen(UnknownScreen),
      currentQuery(),
      pendingQueries(),
      comm(_comm),
      mUnansweredMessageCount(0),
      mTimeoutTimer(nil)
{
  registerAllHandlers();
}

DeviceInfo::DeviceInfo(CommPtr _comm, DeviceID _deviceID, Word _transID)
    : usbHostMIDIDeviceDetails(),
      storedCommandData(),
      deviceID(_deviceID),
      transID(_transID),
      queryScreen(UnknownScreen),
      currentQuery(),
      pendingQueries(),
      comm(_comm),
      mUnansweredMessageCount(0),
      mTimeoutTimer(nil)
{
  registerAllHandlers();
}

DeviceInfo::~DeviceInfo()
{
  closeDevice();
}

void DeviceInfo::closeDevice()
{
  if (mTimeoutTimer != nil)
  {
    [mTimeoutTimer invalidate];
    mTimeoutTimer = nil;
  }
  while (!mSysexMessages.empty())
  {
    mSysexMessages.pop();
  }
  unRegisterHandlerAllHandlers();
}

DeviceID DeviceInfo::getDeviceID() { return deviceID; }

Word DeviceInfo::getPID() { return deviceID.pid(); }

SerialNumber DeviceInfo::getSerialNumber() { return deviceID.serialNumber(); }

Word DeviceInfo::getTransID() { return transID; }

bool DeviceInfo::startQuery(Screen screen, const list<CmdEnum>& query)
{
  bool result = false;

  bool sysexEmpty = mSysexMessages.empty();

  if ((currentQuery.empty()) && sysexEmpty)
  {
    mUnansweredMessageCount = 0;
    attemptedQueries.clear();
    queriedItems.clear();
    queryScreen = screen;

    // Queries before
    currentQuery = query;

    result = sendNextSysex();
  }
  else
  {
    pendingQueries.push(boost::make_tuple(screen, query));
  }

  return result;
}

bool DeviceInfo::rereadStored() {
  list<CmdEnum> query;

  const auto& noDeviceNoCommandList = [](CmdEnum cmd) {
    return ((cmd != Command::RetDevice) && (cmd != Command::RetCommandList));
  };
  copy(storedCommandData | map_keys | transformed(bind(&keyToCommand, _1)) |
           uniqued | filtered(noDeviceNoCommandList),
       std::back_inserter(query));

  // we always have to reread the USBHost device details
  // clear this to get the updated list
  usbHostMIDIDeviceDetails.clear();
  query.push_back(Command::GetUSBHostMIDIDeviceDetail);

  return startQuery(Screen::RereadAllScreen, query);
}

bool DeviceInfo::contains(const commandDataKey_t& key) const {
  return MyAlgorithms::contains(storedCommandData, key);
}

void DeviceInfo::addCommandData(commandData_t commandData) {
  storedCommandData[commandData.key()] = commandData;
}

bool DeviceInfo::containsCommandData(CmdEnum command) const {
  return MyAlgorithms::contains(storedCommandData, generateKey(command));
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

void DeviceInfo::notifyScreen()
{
  NSLog(@"Notify screen");
  // check to see that a valid screen us waiting for a response
  if (queryScreen != Screen::UnknownScreen)
  {
    // Remove doubles form list of queried items
    {
      // create a temporary set of commands
      set<CmdEnum> tempSet;

      // copy all quried items to that set
      tempSet.insert(queriedItems.begin(), queriedItems.end());

      // clear the list of quried items
      queriedItems.clear();

      // add items for the temp set to the quried items
      queriedItems.insert(queriedItems.begin(), tempSet.begin(),
                          tempSet.end());
    }

    // create an objective C object to store the results of the query
    NSDictionary* result;
    // create an Objective C object to store the list of queried items
    {
      NSMutableArray* const nsQuery = [NSMutableArray array];

      // loop through all queried items
      for (auto iter = queriedItems.begin(); iter != queriedItems.end(); ++iter)
      {
        // add an Object C object containing the queried object to the
        // ObjC list of queried objects
        [nsQuery addObject:[NSNumber numberWithInt:*iter]];
      }

      // create a dictionary of results
      result = @{
        // add the calling screen to the result
        @"screen" : @((int)queryScreen),

        // add the results to the results
        @"query" : nsQuery
      };
    }

    // post the query complete notification on the main thread
    runOnMain(^{
        [[NSNotificationCenter defaultCenter]
            postNotificationName:@"queryCompleted"
                          object:nil
                        userInfo:result];});

    // set the screen to the unknown screen (this prevents duplicate
    // calls to query completed)
    queryScreen = Screen::UnknownScreen;
  }
}

void DeviceInfo::checkUnanswered()
{
  NSLog(@"Check Unanswered %i", mUnansweredMessageCount);
  if (mUnansweredMessageCount > 0)
  {
    timeout();
  }
  else
  {
    NSLog(@"All Fine");
  }
}

void DeviceInfo::registerAllHandlers()
{
  auto addHandler = [this](CmdEnum command, Handler handler)
  {
    this->registeredHandlerIDs[command] = comm->registerHandler(command, handler);
  };

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
  addHandler(Command::RetAudioClockParm, commonHandler);
  addHandler(Command::RetAudioPatchbayParm, commonHandler);

  auto midiHostDeviceHandler = bind(
      &DeviceInfo::handleUSBHostMIDIDeviceDetailData, this, _1, _2, _3, _4);
  addHandler(Command::RetUSBHostMIDIDeviceDetail, midiHostDeviceHandler);
}

void DeviceInfo::unRegisterHandlerAllHandlers()
{
  for (const auto& handler : registeredHandlerIDs)
  {
    comm->unRegisterHandler(handler.first, handler.second);
  }
  while (!mSysexMessages.empty())
  {
    mSysexMessages.pop();
  }
}

void DeviceInfo::timeout()
{
  NSLog(@"Device Info timeout");
  comm->unRegisterExclusiveHandler();
  while (!mSysexMessages.empty())
  {
    mSysexMessages.pop();
  }

  currentQuery.clear();
  while (!pendingQueries.empty())
  {
    pendingQueries.pop();
  }
  queriedItems.clear();
  attemptedQueries.clear();
  queryScreen = UnknownScreen;

  runOnMain(^{
      [[NSNotificationCenter defaultCenter]
          postNotificationName:kCommunicationTimeout
                        object:nil];
  });
}

void DeviceInfo::addCommand(const Bytes& sysex)
{
  mSysexMessages.push(sysex);
}

void DeviceInfo::addCommand(const Bytes&& sysex)
{
  mSysexMessages.push(sysex);
}

bool DeviceInfo::sendNextSysex()
{
  // is the sysex message queue empty?
  bool isPendingSysexMessage = !(mSysexMessages.empty());
  
  if (isPendingSysexMessage)
  {
    // get the next pending sysex message
    Bytes message = mSysexMessages.front();
    // remove the pending sysex message from the queue
    mSysexMessages.pop();

    ++mUnansweredMessageCount;
    //NSLog(@"Unanswered %i", mUnansweredMessageCount);

    // send the next sysex message
    comm->sendSysex(message);
  }
  else
  {
    if (mTimeoutTimer != nil)
    {
      [mTimeoutTimer invalidate];
      mTimeoutTimer = nil;
    }
    NSLog(@"Create Timeout Timer");
    mTimeoutTimer = [NSTimer scheduledTimerWithTimeInterval:10.0
                                                    repeats:NO
                                                      block:^(NSTimer*) { checkUnanswered(); mTimeoutTimer = nil;} ];

    // get the front iterator for the current query
    auto q = currentQuery.begin();

    // loop through the current query
    while (q != currentQuery.end())
    {
      // get a list of the dependencies for the current query
      auto D = commandDependancy(*q);

      // if all dependancies are met
      if ((D.empty()) ||
          (MyAlgorithms::all_of(
              D.begin(), D.end(),
              bind(&DeviceInfo::containsCommandData, this, _1))))
      {
        // add the query sysex to the sysex buffer
        addQuerySysex(*q);

        // add query to the attempted queries
        attemptedQueries.insert(*q);

        // remove query from list of pending commands and increment pointer
        q = currentQuery.erase(q);
      }
      else // dependencies not met
      {
        // foreach dependencies
        for (auto d : D)
        {
          // if dependancy isn't in current query and
          // dependancies is not alrealy being queried and
          // dependancies has not been attempted
          if ((!MyAlgorithms::contains(currentQuery, d)) &&
              (!containsCommandData(d)) &&
              (!MyAlgorithms::contains(attemptedQueries, d)))
          {
            // add dependancy to current query
            currentQuery.push_back(d);
          }
        }
        // increment q iterator
        ++q;
      }
    }

    // determine if the pending sysex message queue is empty
    isPendingSysexMessage = !mSysexMessages.empty();

    // if there are not pending sysex messages
    if (!isPendingSysexMessage)
    {
      // if there are pending queries then deal with them now
      if (!pendingQueries.empty())
      {
        // get next query
        auto nextQuery = pendingQueries.front();

        // remove the next query from the pending list
        pendingQueries.pop();

        // set the query Screen
        queryScreen = boost::get<0>(nextQuery);

        // set the currentQuery to the next query
        currentQuery = boost::get<1>(nextQuery);

        // clear the list of queried items
        queriedItems.clear();

        // start the next query
        isPendingSysexMessage = sendNextSysex();
      }
    }
    // there are pending sysex messages from the new query
    else
    {
      // there is something to query, recursivley call this method
      isPendingSysexMessage = sendNextSysex();
    }
  }

  return isPendingSysexMessage;
}

bool DeviceInfo::commonHandleCode(DeviceID _deviceID, Word _transID)
{
  if ((deviceID.serialNumber() == SerialNumber()) ||
      (deviceID.pid() == Word()) || (transID == Word()))
  {
    deviceID = _deviceID;
    transID = _transID;
  }

  return (deviceID == _deviceID) && (transID == _transID);
}

void DeviceInfo::handleCommandData(CmdEnum _command, DeviceID _deviceID,
                                   Word _transID, commandData_t _commandData)
{
  if (commonHandleCode(_deviceID, _transID))
  {
    auto key = _commandData.key();
    storedCommandData[key] = _commandData;
    queriedItems.push_back(_command);

    sendNextSysex();
    --mUnansweredMessageCount;
    //NSLog(@"Unanswered %i", mUnansweredMessageCount);

    bool isPendingSysexMessage = !mSysexMessages.empty();
    if (mUnansweredMessageCount == 0 && !isPendingSysexMessage)
    {
      notifyScreen();
    }
  }
}

void DeviceInfo::handleUSBHostMIDIDeviceDetailData(CmdEnum _command,
                                                   DeviceID _deviceID,
                                                   Word _transID,
                                                   commandData_t _commandData) {
  if (commonHandleCode(_deviceID, _transID))
  {
    auto& foundUSBDetails = _commandData.get<USBHostMIDIDeviceDetail>();

    auto foundItem = find_if(usbHostMIDIDeviceDetails,
                             [=](USBHostMIDIDeviceDetail usbDetails){
        return ((usbDetails.usbHostJack() == foundUSBDetails.usbHostJack()) &&
                (usbDetails.usbHostID() == foundUSBDetails.usbHostID()));
      });

    if ((foundItem == usbHostMIDIDeviceDetails.end() &&
         ((foundUSBDetails.numMIDIIn() > 0) ||
          (foundUSBDetails.numMIDIOut() > 0)))) {
      usbHostMIDIDeviceDetails.push_back(foundUSBDetails);
      queriedItems.push_back(_command);
    }

    sendNextSysex();

    --mUnansweredMessageCount;
    //NSLog(@"Unanswered %i", mUnansweredMessageCount);

    bool isPendingSysexMessage = !mSysexMessages.empty();
    if (mUnansweredMessageCount == 0 && !isPendingSysexMessage)
    {
      notifyScreen();
    }
  }
}

void DeviceInfo::handleACKData(CmdEnum, DeviceID, Word, commandData_t)
{
  bool sysexEmpty;
  size_t messageLength;

  sysexEmpty = mSysexMessages.empty();
  messageLength = mSysexMessages.size();

  if (!sysexEmpty)
  {
    runOnMain(^{
        [[NSNotificationCenter defaultCenter]
            postNotificationName:@"writingProgress"
                          object:nil
                        userInfo:@{
                                   @"progress" :
                                   @((int)(mMaxWriteItems - messageLength))
                                 }];
    });

    sendNextSysex();
  }
  else
  {
    runOnMain(^{
        [[NSNotificationCenter defaultCenter]
            postNotificationName:@"writeCompleted"
                          object:nil];
        });
    comm->unRegisterExclusiveHandler();
  }
}

void DeviceInfo::addQuerySysex(CmdEnum command) {
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

        infoListData.for_each([&](const InfoList::InfoRecord& infoRecord) {
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
        for (Word portID = 1; portID <= midiInfo.numEthernetJacks(); ++portID) {
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
        for (Word portID = 1; portID <= midiInfo.numMIDIPorts(); ++portID) {
          addCommand<GetMIDIPortInfoCommand>(portID);
        }
      }
      break;
    }

    case Command::GetMIDIPortFilter:
    case Command::RetMIDIPortFilter: {
      if (commandList.contains(Command::GetMIDIPortFilter)) {
        for (Word portID = 1; portID <= midiInfo.numMIDIPorts(); ++portID) {
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
        for (Word portID = 1; portID <= midiInfo.numMIDIPorts(); ++portID) {
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
        for (Word portID = 1; portID <= midiInfo.numMIDIPorts(); ++portID) {
          addCommand<GetMIDIPortRouteCommand>(portID);
        }
      }
      break;
    }

    case Command::GetMIDIPortDetail:
    case Command::RetMIDIPortDetail: {
      if (commandList.contains(Command::GetMIDIPortDetail)) {
        for (Word portID = 1; portID <= midiInfo.numMIDIPorts(); ++portID) {
          addCommand<GetMIDIPortDetailCommand>(portID);
        }
      }
      break;
    }

    case Command::GetUSBHostMIDIDeviceDetail:
    case Command::RetUSBHostMIDIDeviceDetail: {
      for (Word jackID = 1; jackID <= midiInfo.numUSBHostJacks(); ++jackID) {
        for (Word hostID = 1; hostID <= midiInfo.numUSBMIDIPortPerHostJack();
             ++hostID) {
          addCommand<GetUSBHostMIDIDeviceDetailCommand>(jackID, hostID);
        }
      }
      break;
    }

    case Command::GetRTPMIDIConnectionDetail:
    case Command::RetRTPMIDIConnectionDetail: {
      if (commandList.contains(Command::GetRTPMIDIConnectionDetail)) {
        auto startPort =
            midiInfo.numDINPairs() +
            midiInfo.numUSBDeviceJacks() *
                midiInfo.numUSBMIDIPortPerDeviceJack() +
            midiInfo.numUSBHostJacks() * midiInfo.numUSBMIDIPortPerHostJack() +
            1;
        for (Byte portID = startPort; portID <= midiInfo.numMIDIPorts();
             ++portID) {
          for (Word connID = 1;
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
        for (Word portID = 1; portID <= audioInfo.numberOfAudioPorts();
             ++portID) {
          addCommand<GetAudioPortInfoCommand>(portID);
        }
      }
      break;
    }

    case Command::GetAudioPortCfgInfo:
    case Command::RetAudioPortCfgInfo: {
      if (commandList.contains(Command::GetAudioPortCfgInfo)) {
        for (Word portID = 1; portID <= audioInfo.numberOfAudioPorts();
             ++portID) {
          addCommand<GetAudioPortCfgInfoCommand>(portID);
        }
      }
      break;
    }

    case Command::GetAudioPortPatchbay:
    case Command::RetAudioPortPatchbay: {
      if (commandList.contains(Command::GetAudioPortPatchbay)) {
        for (Word portID = 1; portID <= audioInfo.numberOfAudioPorts();
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
        for_each<AudioDeviceParm>([&](const AudioDeviceParm& audioDeviceParm) {
          for (Byte i = 1; i < audioDeviceParm.maxControllers(); ++i) {
            addCommand<GetAudioControlParmCommand>(
                audioDeviceParm.audioPortID(), i);
          }
        });
      }
      break;
    }

    case Command::GetAudioControlDetail:
    case Command::RetAudioControlDetail: {
      if (commandList.contains(Command::GetAudioControlDetail)) {
        for_each<AudioControlParm>([&](
            const AudioControlParm& audioControlParm) {
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

    default:
      // do nothing
      break;
  }
}

Bytes DeviceInfo::serialize()
{
  Bytes result;

  // Add File Prefix
  result += 'i', 'C', 'M';

  // Add Model Number
  result += deviceID.pid();

  // Add Version Number
  result += 0x01;

  for (const auto& cmdPair : storedCommandData) {
    push_back(result, generate(keyToCommand(cmdPair.first), cmdPair.second));
  }

  unsigned char digest[16];
  CC_MD5((char*)result.data(), (result.size()), digest);
  for (size_t i = 0; i < 16; ++i) {
    result.push_back(digest[i]);
  }

  return result;
}

bool DeviceInfo::deserialize(Bytes data)
{
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

  // Verify the footer
  if (valid)
  {
    BytesIter md5Iter = finish - 16;

    unsigned char digest[16];
    CC_MD5((char*)data.data(), (data.size() - 16), digest);

    for (size_t i = 0; i < 16; ++i) {
      if (digest[i] != *md5Iter) {
        valid = false;
        break;
      }
      ++md5Iter;
    }
  }

  if (valid)
  {
    DeviceID storedDeviceID = deviceID;
    Word storedTransID = transID;

    storedCommandData.clear();
    usbHostMIDIDeviceDetails.clear();

    comm->parseBytes(start, finish, deviceID);

    deviceID = storedDeviceID;
    transID = storedTransID;

    auto ackHandler = bind(&DeviceInfo::handleACKData, this, _1, _2, _3, _4);
    comm->registerExclusiveHandler(Command::ACK, ackHandler);
    writeAll();
  }

  return valid;
}

long DeviceInfo::registerHandler(CmdEnum commandID, Handler handler)
{
  return comm->registerHandler(commandID, handler);
}

void DeviceInfo::unRegisterHandler(CmdEnum commandID)
{
  comm->unRegisterHandler(commandID);
}

void DeviceInfo::unRegisterHandler(CmdEnum commandID, long handlerID)
{
  comm->unRegisterHandler(commandID, handlerID);
}

void DeviceInfo::unRegisterAll()
{
  comm->unRegisterAll();
}

void DeviceInfo::registerExclusiveHandler(CmdEnum commandID, Handler handler)
{
  comm->registerExclusiveHandler(commandID, handler);
}

void DeviceInfo::unRegisterExclusiveHandler()
{
  comm->unRegisterExclusiveHandler();
}

void DeviceInfo::writeAll()
{
  for (const auto& cmdData : storedCommandData)
  {
    addCommand(generate((CmdEnum)(WRITE_BIT | keyToCommandID(cmdData.first)), cmdData.second));
  }

  mMaxWriteItems = mSysexMessages.size();

  runOnMain(^{
      [[NSNotificationCenter defaultCenter]
          postNotificationName:@"writingStarted"
                        object:nil
                      userInfo:@{@"maxWriteItems" : @(mMaxWriteItems)}];
      });

  sendNextSysex();
}

Bytes DeviceInfo::generate(CmdEnum command,
                           const commandData_t& cmdData) const
{
  return ::generate(deviceID, transID, command, cmdData);
}
