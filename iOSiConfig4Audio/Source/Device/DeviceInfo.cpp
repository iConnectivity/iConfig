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
#import "ICRunOnMain.h"

#include <boost/bind.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/algorithm/find_if.hpp>

#import <CommonCrypto/CommonDigest.h>
#include <boost/tuple/tuple.hpp>
#include <numeric>

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
      comm(_comm) {
        registerAllHandlers();
        sendLock = [[NSLock alloc] init];
        attemptedQueriesLock = [[NSLock alloc] init];
        currentQueriesLock = [[NSLock alloc] init];
        queriedItemsLock = [[NSLock alloc] init];
        pendingQueriesLock = [[NSLock alloc] init];
}

DeviceInfo::DeviceInfo(CommPtr _comm, DeviceID _deviceID, Word _transID)
    : usbHostMIDIDeviceDetails(),
      storedCommandData(),
      deviceID(_deviceID),
      transID(_transID),
      queryScreen(UnknownScreen),
      currentQuery(),
      pendingQueries(),
      comm(_comm) {
        registerAllHandlers();
        sendLock = [[NSLock alloc] init];
        attemptedQueriesLock = [[NSLock alloc] init];
        currentQueriesLock = [[NSLock alloc] init];
        queriedItemsLock = [[NSLock alloc] init];
        pendingQueriesLock = [[NSLock alloc] init];
}

DeviceInfo::~DeviceInfo() { closeDevice(); }

void DeviceInfo::closeDevice() {
  //NSLog(@"lock1");
  [sendLock lock];
  while (!sysexMessages.empty()) {
    sysexMessages.pop();
  }
  //NSLog(@"lock2");
  [sendLock unlock];
  unRegisterHandlerAllHandlers();
}

DeviceID DeviceInfo::getDeviceID() { return deviceID; }

Word DeviceInfo::getPID() { return deviceID.pid(); }

SerialNumber DeviceInfo::getSerialNumber() { return deviceID.serialNumber(); }

Word DeviceInfo::getTransID() { return transID; }

bool DeviceInfo::startQuery(Screen screen, const list<CmdEnum>& query) {
  bool result = false;

  bool sysexEmpty;

  //NSLog(@"3Thread: %@", [NSThread currentThread]);
  //NSLog(@"lock3");
  [sendLock lock];
  sysexEmpty = sysexMessages.empty();
  [sendLock unlock];
  //NSLog(@"lock4");

  [sendLock lock];
  [currentQueriesLock lock];
  if ((currentQuery.empty()) && sysexEmpty) {
    //NSLog(@"lock5");
    [attemptedQueriesLock lock];
    attemptedQueries.clear();
    [attemptedQueriesLock unlock];
    //NSLog(@"lock6");

    //NSLog(@"lock7");
    [queriedItemsLock lock];
    queriedItems.clear();
    [queriedItemsLock unlock];
    //NSLog(@"lock8");

    queryScreen = screen;

    // Queries before
    currentQuery = query;

    [currentQueriesLock unlock];
    [sendLock unlock];
    result = sendNextSysex();
  } else {
    [currentQueriesLock unlock];
    [sendLock unlock];
    //NSLog(@"lock9");
    [pendingQueriesLock lock];

    pendingQueries.push(boost::make_tuple(screen, query));
    [pendingQueriesLock unlock];
    //NSLog(@"lock10");
  }

  return result;
}

void DeviceInfo::clearQueries() {
  [sendLock lock];
  while (!sysexMessages.empty()) {
    sysexMessages.pop();
  }
  [sendLock unlock];

  [currentQueriesLock lock];
  currentQuery.clear();
  [currentQueriesLock unlock];

  [pendingQueriesLock lock];
  while (!pendingQueries.empty()) {
    pendingQueries.pop();
  }
  [pendingQueriesLock unlock];

  [queriedItemsLock lock];
  queriedItems.clear();
  [queriedItemsLock unlock];

  [attemptedQueriesLock lock];
  attemptedQueries.clear();
  [attemptedQueriesLock unlock];

  queryScreen = UnknownScreen;
}

bool DeviceInfo::rereadAudioControls() {
  list<CmdEnum> query;

  query.push_back(Command::RetAudioControlDetailValue);

  return startQuery(Screen::RereadAudioControls, query);
}

bool DeviceInfo::rereadMixerControls() {
  list<CmdEnum> query;

  query.push_back(Command::RetMixerInputControlValue);
  query.push_back(Command::RetMixerOutputControlValue);

  return startQuery(Screen::RereadAudioControls, query);
}

bool DeviceInfo::rereadMeters() {
  list<CmdEnum> query;

  query.push_back(Command::RetAudioPortMeterValue);
  query.push_back(Command::RetMixerMeterValue);

  dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
    startQuery(Screen::RereadMeters, query);
  });
  return true;
}

bool DeviceInfo::rereadAudioInfo() {

  list<CmdEnum> query;

  query.push_back(Command::RetAudioGlobalParm);
  query.push_back(Command::RetAudioClockParm);
  query.push_back(Command::RetMixerParm);
  query.push_back(Command::RetMixerPortParm);

  return startQuery(Screen::RereadAudioControls, query);
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

void DeviceInfo::registerAllHandlers() {
  auto addHandler = [this](CmdEnum command, Handler handler) {
    this->registeredHandlerIDs[command] =
        comm->registerHandler(command, handler);
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
  addHandler(Command::RetAudioChannelName, commonHandler);
  addHandler(Command::RetAudioPortMeterValue, commonHandler);
  addHandler(Command::RetAudioControlDetailValue, commonHandler);
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
  for (const auto& handler : registeredHandlerIDs) {
    comm->unRegisterHandler(handler.first, handler.second);
  }
  //NSLog(@"lock11");
  [sendLock lock];

  while (!sysexMessages.empty()) {
    sysexMessages.pop();
  }
  [sendLock unlock];
  //NSLog(@"lock12");

}

void DeviceInfo::timeout() {
  //NSLog(@"Device Info timeout");
  comm->unRegisterExclusiveHandler();
  //NSLog(@"lock13");
  [sendLock lock];

  while (!sysexMessages.empty()) {
    sysexMessages.pop();
  }
  [sendLock unlock];
  //NSLog(@"lock14");


  //NSLog(@"lock15");
  [currentQueriesLock lock];

  currentQuery.clear();
  [currentQueriesLock unlock];
  //NSLog(@"lock16");


  //NSLog(@"lock17");
  [pendingQueriesLock lock];

  while (!pendingQueries.empty()) {
    pendingQueries.pop();
  }
  [pendingQueriesLock unlock];
  //NSLog(@"lock18");


  //NSLog(@"lock19");
  [queriedItemsLock lock];

  queriedItems.clear();
  [queriedItemsLock unlock];
  //NSLog(@"lock20");

  //NSLog(@"lock21");
  [attemptedQueriesLock lock];

  attemptedQueries.clear();
  [attemptedQueriesLock unlock];
  //NSLog(@"lock22");

  queryScreen = UnknownScreen;

  runOnMain(^{
      [[NSNotificationCenter defaultCenter]
          postNotificationName:kCommunicationTimeout
                        object:nil];
  });
}

void DeviceInfo::addCommand(const Bytes& sysex) {
  //NSLog(@"lock23");
  [sendLock lock];

  sysexMessages.push(sysex);
  [sendLock unlock];
  //NSLog(@"lock24");
}

void DeviceInfo::addCommand(const Bytes&& sysex) {
  //NSLog(@"lock25");
  [sendLock lock];
  sysexMessages.push(sysex);
  [sendLock unlock];
  //NSLog(@"lock26");
}

bool DeviceInfo::containsCommandDataType(CmdEnum command) const {

  bool value = storedCommandData.lower_bound(generateKey(command)) !=
  storedCommandData.end();
  return value;
}

bool DeviceInfo::sendNextSysex() {


  // is the sysex message queue empty?

  //printf("0: sysexMessages.size(): %d, currentQuery.size(): %d (%d)\n", sysexMessages.size(), currentQuery.size(), QThread::currentThreadId());

  //NSLog(@"27Thread: %@", [NSThread currentThread]);
  bool send = !(sysexMessages.empty());

  if (send) {
    // send the next sysex message
    //printf("presend\n");
    //NSLog(@"lock27");
    [sendLock lock];
    Bytes front = sysexMessages.front();
    sysexMessages.pop();
    [sendLock unlock];
    //NSLog(@"lock28");

    comm->sendSysex(front);

    //printf("postsend\n");

    // remove the sysex message from the message queue
  }


  if (send) {
    //if (sysexMessages.empty())
    return true;
    //else
    //  return sendNextSysex();
  }

  //printf("1: sysexMessages.size(): %d, currentQuery.size(): %d (%d)\n", sysexMessages.size(), currentQuery.size(), QThread::currentThreadId());

  if (!send) {
    //NSLog(@"lock29");
    [currentQueriesLock lock];

    // get the front iterator for the current query
    auto q = currentQuery.begin();

    // loop through the current query
    while (q != currentQuery.end()) {
      auto D = commandDependancy(*q);

      bool metDependancies = true;
      for (auto d : D) {
        if (!containsCommandDataType(d)) {
          //NSLog(@"lock30");
          [attemptedQueriesLock lock];

          if (!MyAlgorithms::contains(attemptedQueries, d)) {
            metDependancies = false;
          }
          [attemptedQueriesLock unlock];
          //NSLog(@"lock31");

        }
      }

      // if all dependancies are met
      if ((D.empty()) || metDependancies) {

        // add the query sysex to the sysex buffer
        addQuerySysex(*q);

        // add query to the attempted queries
        //NSLog(@"lock32");
        [attemptedQueriesLock lock];

        attemptedQueries.insert(*q);
        [attemptedQueriesLock unlock];
        //NSLog(@"lock33");

        // remove query from list of pending commands and increment pointer

        //printf("2: currentQuery.size(): %d, q: %d (%d)\n", currentQuery.size(), q, QThread::currentThreadId());
        q = currentQuery.erase(q);
        [currentQueriesLock unlock];
        //NSLog(@"lock34");

        //printf("438\n");
        return sendNextSysex();
      } else {
        // dependencies not met
        // foreach dependencies
        for (auto d : D) {
          // if dependancy isn't in current query and dependancies is not
          // alrealy being queried and dependancies has not been attempted
          //NSLog(@"lock35");
          [attemptedQueriesLock lock];

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
          [attemptedQueriesLock unlock];
          //NSLog(@"lock36");

        }

        q = currentQuery.begin();
      }
    }

    [currentQueriesLock unlock];
    //NSLog(@"lock37");


    // if there are not pending sysex messages
    if (sysexMessages.empty()) {
      // we are done with the current query

      if (queryScreen != Screen::UnknownScreen) {
        // Remove doubles form list of queried items
        {
          // create a temporary set of commands
          set<CmdEnum> tempSet;

          //NSLog(@"lock38");
          [queriedItemsLock lock];

          // copy all quried items to that set
          tempSet.insert(queriedItems.begin(), queriedItems.end());

          // clear the list of quried items
          queriedItems.clear();

          // add items for the temp set to the quried items
          queriedItems.insert(queriedItems.begin(), tempSet.begin(),
                              tempSet.end());
          [queriedItemsLock unlock];
          //NSLog(@"lock39");

        }

        // create an objective C object to store the results of the query
        NSDictionary* result;

        // create an Objective C object to store the list of queried items
        {
          NSMutableArray* const nsQuery = [NSMutableArray array];

          // loop through all queried items
          for (auto iter = queriedItems.begin(); iter != queriedItems.end();
               ++iter) {
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
           userInfo:result];
        });

        //NSLog(@"lock40");
        [currentQueriesLock lock];

        currentQuery.clear();
        [currentQueriesLock unlock];
        //NSLog(@"lock41");
        queryScreen = Screen::UnknownScreen;
      }

      // if there are pending queries then deal with them now
      if (!pendingQueries.empty()) {
        //NSLog(@"lock42");
        [pendingQueriesLock lock];

        // get next query
        auto nextQuery = pendingQueries.front();

        // remove the next query from the pending list
        pendingQueries.pop();
        [pendingQueriesLock unlock];
        //NSLog(@"lock43");


        // set the query Screen
        queryScreen = boost::get<0>(nextQuery);

        // set the currentQuery to the next query
        //NSLog(@"lock44");
        [currentQueriesLock lock];
        currentQuery = boost::get<1>(nextQuery);
        [currentQueriesLock unlock];
        //NSLog(@"lock45");


        // clear the list of queried items
        //NSLog(@"lock46");
        [queriedItemsLock lock];
        queriedItems.clear();
        [queriedItemsLock unlock];
        //NSLog(@"lock47");

        // start the next query
        //printf("504\n");
        send = sendNextSysex();
      }
    } else {
      // there are pending sysex messages
      //NSLog(@"lock48");
      [sendLock lock];
      // send the next sysex message
      comm->sendSysex(sysexMessages.front());
      // remove the sent sysex message from the queue
      sysexMessages.pop();
      [sendLock unlock];
      //NSLog(@"lock49");

      // we have sent a sysex message
      send = true;
      
      //emit queryStarted();
    }
  }
  
  return send;
}


//bool DeviceInfo::sendNextSysex() {
//  // is the sysex message queue empty?
//  bool isPendingSysexMessage;
//
//  [sendLock lock];
//  isPendingSysexMessage = !(sysexMessages.empty());
//  [sendLock unlock];
//
//  if (isPendingSysexMessage) {
//    // Create a variable to hold the next sysex message
//    Bytes message;
//
//    // Lock the pending sysex queue lock
//    [sendLock lock];
//
//    // get the next pending sysex message
//    message = sysexMessages.front();
//
//    // remove the pending sysex message from the queue
//    sysexMessages.pop();
//
//    // unlock the pending sysex queue lock
//    [sendLock unlock];
//
//    // send the next sysex message
//    comm->sendSysex(message);
//  } else {
//    // get the front iterator for the current query
//    auto q = currentQuery.begin();
//
//    // loop through the current query
//    while (q != currentQuery.end()) {
//      // get a list of the dependencies for the current query
//      auto D = commandDependancy(*q);
//
//      // if all dependancies are met
//      if ((D.empty()) ||
//          (MyAlgorithms::all_of(
//              D.begin(), D.end(),
//              bind(&DeviceInfo::containsCommandData, this, _1)))) {
//        // add the query sysex to the sysex buffer
//        addQuerySysex(*q);
//
//        // add query to the attempted queries
//        attemptedQueries.insert(*q);
//
//        // remove query from list of pending commands and increment pointer
//        q = currentQuery.erase(q);
//      }
//      // dependencies not met
//      else {
//        // foreach dependencies
//        for (auto d : D) {
//          // if dependancy isn't in current query and
//          // dependancies is not alrealy being queried and
//          // dependancies has not been attempted
//          if ((!MyAlgorithms::contains(currentQuery, d)) &&
//              (!containsCommandData(d)) &&
//              (!MyAlgorithms::contains(attemptedQueries, d))) {
//            // add dependancy to current query
//            currentQuery.push_back(d);
//          }
//        }
//
//        // increment q iterator
//        ++q;
//      }
//    }
//
//    // lock the pending sysex message queue lock
//    [sendLock lock];
//
//    // determine if the pending sysex message queue is empty
//    isPendingSysexMessage = !sysexMessages.empty();
//
//    // unlock the pending sysex message queue lock
//    [sendLock unlock];
//
//    // if there are not pending sysex messages
//    if (!isPendingSysexMessage) {
//
//      // check to make sure that the current query is complete
//      if (!currentQuery.empty()) {
//        // the current query isn't complete. There is an error
//
//        //NSLog(@"Current query isn't complete"
//               "but there are no pending messages.");
//        fprintf(stderr, "QUERY - [ ");
//        for (const auto& cmd : currentQuery) fprintf(stderr, "%04X ", cmd);
//        fprintf(stderr, "]\n");
//
//
//        timeout();
//      }
//
//      // check to see that a valid screen us waiting for a response
//      if (queryScreen != Screen::UnknownScreen) {
//
//        // Remove doubles form list of queried items
//        {
//          // create a temporary set of commands
//          set<CmdEnum> tempSet;
//
//          // copy all quried items to that set
//          tempSet.insert(queriedItems.begin(), queriedItems.end());
//
//          // clear the list of quried items
//          queriedItems.clear();
//
//          // add items for the temp set to the quried items
//          queriedItems.insert(queriedItems.begin(), tempSet.begin(),
//                              tempSet.end());
//        }
//
//        // create an objective C object to store the results of the query
//        NSDictionary* result;
//
//        // create an Objective C object to store the list of queried items
//        {
//          NSMutableArray* const nsQuery = [NSMutableArray array];
//
//          // loop through all queried items
//          for (auto iter = queriedItems.begin(); iter != queriedItems.end();
//               ++iter) {
//            // add an Object C object containing the queried object to the
//            // ObjC list of queried objects
//            [nsQuery addObject:[NSNumber numberWithInt:*iter]];
//          }
//
//          // create a dictionary of results
//          result = @{
//            // add the calling screen to the result
//            @"screen" : @((int)queryScreen),
//
//            // add the results to the results
//            @"query" : nsQuery
//          };
//        }
//
//        // post the query complete notification on the main thread
//        runOnMain(^{
//            [[NSNotificationCenter defaultCenter]
//                postNotificationName:@"queryCompleted"
//                              object:nil
//                            userInfo:result];
//        });
//
//        // set the screen to the unknown screen (this prevents duplicate
//        // calls to query completed)
//        queryScreen = Screen::UnknownScreen;
//      }
//
//      // if there are pending queries then deal with them now
//      if (!pendingQueries.empty()) {
//        // get next query
//        auto nextQuery = pendingQueries.front();
//
//        // remove the next query from the pending list
//        pendingQueries.pop();
//
//        // set the query Screen
//        queryScreen = boost::get<0>(nextQuery);
//
//        // set the currentQuery to the next query
//        currentQuery = boost::get<1>(nextQuery);
//
//        // clear the list of queried items
//        queriedItems.clear();
//
//        // start the next query
//        isPendingSysexMessage = sendNextSysex();
//      }
//    }
//    // there are pending sysex messages from the new query
//    else {
//      // there is something to query, recursivley call this method
//      isPendingSysexMessage = sendNextSysex();
//    }
//  }
//
//  return isPendingSysexMessage;
//}

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
    storedCommandData[_commandData.key()] = _commandData;
    //NSLog(@"lock50");
    [queriedItemsLock lock];
    queriedItems.push_back(_command);
    [queriedItemsLock unlock];
    //NSLog(@"lock51");

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
                             [=](USBHostMIDIDeviceDetail usbDetails) {
      return ((usbDetails.usbHostJack() == foundUSBDetails.usbHostJack()) &&
              (usbDetails.usbHostID() == foundUSBDetails.usbHostID()));
    });

    if ((foundItem == usbHostMIDIDeviceDetails.end() &&
         ((foundUSBDetails.numMIDIIn() > 0) ||
          (foundUSBDetails.numMIDIOut() > 0)))) {
           usbHostMIDIDeviceDetails.push_back(foundUSBDetails);
           //NSLog(@"lock52");
           [queriedItemsLock lock];
           queriedItems.push_back(_command);
           [queriedItemsLock unlock];
           //NSLog(@"lock53");
    }

    sendNextSysex();
  }
}

void DeviceInfo::handleACKData(CmdEnum, DeviceID, Word, commandData_t) {
  bool sysexEmpty;
  size_t messageLength;

  //NSLog(@"lock54");
  [sendLock lock];
  sysexEmpty = sysexMessages.empty();
  messageLength = sysexMessages.size();
  [sendLock unlock];
  //NSLog(@"lock55");

  if (!sysexEmpty) {
    runOnMain(^{
        [[NSNotificationCenter defaultCenter]
            postNotificationName:@"writingProgress"
                          object:nil
                        userInfo:@{
                                   @"progress" :
                                   @((int)(maxWriteItems - messageLength))
                                 }];
    });

    sendNextSysex();
  } else {
    runOnMain(^{
        [[NSNotificationCenter defaultCenter]
            postNotificationName:@"writeCompleted"
                          object:nil];
    });
    comm->unRegisterExclusiveHandler();
  }
}

void DeviceInfo::addQuerySysex(CmdEnum command) {
  NSLog(@"addQuerySysex: %d", command);

  const auto& commandList =
      contains<CommandList>() ? get<CommandList>() : CommandList();

  const auto& midiInfo = contains<MIDIInfo>() ? get<MIDIInfo>() : MIDIInfo();

  const auto& audioInfo =
      contains<AudioInfo>() ? get<AudioInfo>() : AudioInfo();

  const auto& gizmoCount =
      contains<GizmoCount>() ? get<GizmoCount>() : GizmoCount();

  const auto& audioGlobalParm =
      contains<AudioGlobalParm>() ? get<AudioGlobalParm>() : AudioGlobalParm();

  //NSLog(@"lock56");
  //[sendLock lock];
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
                                         //NSLog(@"adding AudioControlParm: %d, %d, %d", audioControlParm.audioPortID(), audioControlParm.controllerNumber(), detailID);
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
  //[sendLock unlock];
  //NSLog(@"lock57");
  NSLog(@"!addQuerySysex: %d", command);

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

  unsigned char digest[16];
  CC_MD5((char*)result.data(), (result.size()), digest);
  for (size_t i = 0; i < 16; ++i) {
    result.push_back(digest[i]);
  }

  return result;
}

Bytes DeviceInfo::serialize2(std::set<Command::Enum> commandsToSave, NSString *description) {
  Bytes result;

  // Add Magic Number
  result += 0x69, 0x43, 0x4D;

  // Add Model Number
  result += deviceID.pid();

  // Add Version Number
  result += 0x02;

  description = [description substringWithRange:NSMakeRange(0,239)];
  result += (unsigned char) [description length];
  for (int x = 0; x < [description length]; x++) {
    result += (unsigned char)[description characterAtIndex:x];
  }

  for (const auto& cmdPair : storedCommandData) {
    if (commandsToSave.find(keyToCommand(cmdPair.first)) != commandsToSave.end()) { // if we're supposed to save it
      Bytes toWrite = generate(keyToCommand(cmdPair.first), cmdPair.second);
      push_back(result, toWrite);
    }
  }

  unsigned char digest[16];
  CC_MD5((char*)result.data(), (result.size()), digest);
  for (size_t i = 0; i < 16; ++i) {
    result.push_back(digest[i]);
  }

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
    static unsigned char arr2[] = {0xF0, 0x00, 0x01, 0x73, 0x7E, 0x00, (unsigned char) (deviceID.pid() & 0xFF), 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x01, 0x40, 0x10, 0x00, 0x01, 0x01, 0x00, 0xF7}; // Reset
    replaceChecksumByte(arr2, sizeof(arr2) / sizeof(arr2[0]));
    Bytes toWrite2(arr2, arr2 + sizeof(arr2) / sizeof(arr2[0]) );
    toWrite2.insert(toWrite2.begin() + 1, toWrite2.size() - 1);
    result += 10;
    push_back(result, toWrite2);
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
      return valid;
    }
  }

  // Verify the footer
  if (valid) {
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
  return comm->registerHandler(commandID, handler);
}

void DeviceInfo::unRegisterHandler(CmdEnum commandID) {
  comm->unRegisterHandler(commandID);
}

void DeviceInfo::unRegisterHandler(CmdEnum commandID, long handlerID) {
  comm->unRegisterHandler(commandID, handlerID);
}

void DeviceInfo::unRegisterAll() { comm->unRegisterAll(); }

void DeviceInfo::registerExclusiveHandler(CmdEnum commandID, Handler handler) {
  comm->registerExclusiveHandler(commandID, handler);
}

void DeviceInfo::unRegisterExclusiveHandler() {
  comm->unRegisterExclusiveHandler();
}

void DeviceInfo::writeAll() {
  for (const auto& cmdData : storedCommandData) {
    addCommand(generate((CmdEnum)(WRITE_BIT | keyToCommandID(cmdData.first)),
                        cmdData.second));
  }

  //NSLog(@"lock58");
  [sendLock lock];
  maxWriteItems = sysexMessages.size();
  [sendLock unlock];
  //NSLog(@"lock59");


  runOnMain(^{
      [[NSNotificationCenter defaultCenter]
          postNotificationName:@"writingStarted"
                        object:nil
                      userInfo:@{@"maxWriteItems" : @(maxWriteItems)}];
  });

  sendNextSysex();
}

Bytes DeviceInfo::generate(CmdEnum command,
                           const commandData_t& cmdData) const {
  return ::generate(deviceID, transID, command, cmdData);
}
