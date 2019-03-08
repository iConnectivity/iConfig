/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __DEVICEINFORMATION_H__
#define __DEVICEINFORMATION_H__

#include "CommandDefines.h"
#include "CommandDataKey.h"
#include "Communicator.h"
#include "DeviceID.h"
#include "EthernetPortInfo.h"
#include "Info.h"
#include "MIDIPortDetail.h"
#include "MIDIPortFilter.h"
#include "MIDIPortRemap.h"
#include "Screen.h"
#include "SysexCommand.h"
#include "USBHostMIDIDeviceDetail.h"

#include <boost/algorithm/cxx11/any_of.hpp>

using GeneSysLib::CommPtr;
using GeneSysLib::commandDataKey_t;
using GeneSysLib::commandData_t;
using GeneSysLib::DeviceID;
using GeneSysLib::Info;
using GeneSysLib::MIDIPortRemap;
using GeneSysLib::MIDIPortFilter;

struct DeviceInfo {
  DeviceInfo(GeneSysLib::CommPtr comm);
  DeviceInfo(GeneSysLib::CommPtr comm, GeneSysLib::DeviceID deviceID,
             Word transID);
  virtual ~DeviceInfo();

  GeneSysLib::DeviceID getDeviceID();
  Word getPID();
  SerialNumber getSerialNumber();
  Word getTransID();

  void closeDevice();

  typedef std::map<commandDataKey_t, commandData_t> CommandDataMap;
  typedef CommandDataMap::iterator CommandDataIterator;

  inline void send(const Bytes &sysex) { comm->sendSysex(sysex); }

  template <typename DATA_T, typename... Ts> void send(Ts... vs) {
    DATA_T(deviceID, transID, vs...).send(comm);
  }

  bool startQuery(Screen screen, const std::list<GeneSysLib::CmdEnum> &query);
  bool rereadStored();

  // Command Information
  void addCommandData(commandData_t commandData);
  bool containsCommandData(GeneSysLib::CmdEnum command) const;
  bool contains(const commandDataKey_t &key) const;

  // generic
  template <typename T> size_t typeCount() const {
    const auto &lower = storedCommandData.lower_bound(T::minKey());
    const auto &upper = storedCommandData.upper_bound(T::maxKey());
    return std::distance(lower, upper);
  }

  // This method will return true if the storedCommandData map contains the key
  // generated with the command and the variadic list of parameters
  template <typename T, typename... Ts>
  bool contains(Ts... ts) const {
    return MyAlgorithms::contains(
        storedCommandData,
        GeneSysLib::generateKey<Ts...>(T::retCommand(), ts...));
  }

  template <typename T>
  T &get(const commandDataKey_t &key) {
    return this->storedCommandData.at(key).get<T>();
  }

  template <typename T>
  const T &get(const commandDataKey_t &key) const {
    return this->storedCommandData.at(key).get<T>();
  }

  template <typename T>
  T &get() {
    return storedCommandData.at(GeneSysLib::generateKey(T::retCommand()))
        .template get<T>();
  }

  template <typename T>
  const T &get() const {
    return storedCommandData.at(GeneSysLib::generateKey(T::retCommand()))
        .template get<T>();
  }

  template <typename T, typename... Ts> T &get(Ts... ts) {
    return get<T>(T::queryKey(ts...));
  }

  template <typename T, typename... Ts> const T &get(Ts... ts) const {
    return get<T>(T::queryKey(ts...));
  }

  template <typename T, typename Action>
  void for_each(Action action) {
    auto lower = storedCommandData.lower_bound(T::minKey());
    const auto &upper = storedCommandData.upper_bound(T::maxKey());

    while (lower != upper) {
      action(lower->second.template get<T>());
      ++lower;
    }
  }

  template <typename T, typename Action>
  void for_each(Action action) const {
    auto lower = storedCommandData.lower_bound(T::minKey());
    const auto &upper = storedCommandData.upper_bound(T::maxKey());

    while (lower != upper) {
      action(lower->second.template get<T>());
      ++lower;
    }
  }

  template <typename T, typename UnaryPredicate>
  bool any_of(UnaryPredicate pred) {
    CommandDataIterator lower = storedCommandData.lower_bound(T::minKey());
    const auto &upper = storedCommandData.upper_bound(T::maxKey());
    bool result = false;
    for (; lower != upper; ++lower) {
      if (pred(lower->second.get<T>())) {
        result = true;
        break;
      }
    }
    return result;
  }

  template <typename T, typename Pred> size_t count(Pred pred) const {
    size_t count = 0;
    for_each<T>([&](const T & t) {
      if (pred(t)) {
        ++count;
      }
    });
    return count;
  }

  // General Info
  bool containsInfo(GeneSysLib::InfoIDEnum infoID) const;
  Info &infoData(GeneSysLib::InfoIDEnum infoID);
  const Info &infoData(GeneSysLib::InfoIDEnum infoID) const;

  // MIDI Port Filters
  MIDIPortFilter &midiPortFilter(Word portID,
                                 GeneSysLib::FilterIDEnum filterType);
  const MIDIPortFilter &midiPortFilter(
      Word portID, GeneSysLib::FilterIDEnum filterType) const;

  // MIDI Port Remap
  MIDIPortRemap &midiPortRemap(Word portID,
                               GeneSysLib::RemapTypeEnum remapType);
  const MIDIPortRemap &midiPortRemap(Word portID,
                                     GeneSysLib::RemapTypeEnum remapType) const;

  // Audio Port Info
  size_t audioPortInfoCount() const;

  Bytes serialize();
  bool deserialize(Bytes data);

  std::vector<GeneSysLib::USBHostMIDIDeviceDetail> usbHostMIDIDeviceDetails;

  long registerHandler(GeneSysLib::CmdEnum commandID, Handler handler);

  void unRegisterHandler(GeneSysLib::CmdEnum commandID);
  void unRegisterHandler(GeneSysLib::CmdEnum commandID, long handlerID);
  void unRegisterAll();

  void registerExclusiveHandler(GeneSysLib::CmdEnum commandID, Handler handler);
  void unRegisterExclusiveHandler();

  void timeout();

 private:
  void registerAllHandlers();
  void unRegisterHandlerAllHandlers();

  void addCommand(const Bytes &sysex);
  void addCommand(const Bytes &&sysex);

  template <typename T>
  void addCommand(const T &command) {
    [sendLock lock];
    sysexMessages.push(command.sysex());
    [sendLock unlock];
  }

  template <typename T>
  void addCommand(const T &&command) {
    [sendLock lock];
    sysexMessages.push(command.sysex());
    [sendLock unlock];
  }

  template <typename T, typename... Ts>
  void addCommand(Ts... vs) {
    [sendLock lock];
    sysexMessages.push(T(deviceID, transID, vs...).sysex());
    [sendLock unlock];
  }

  bool sendNextSysex();

  void writeAll();

  bool commonHandleCode(GeneSysLib::DeviceID deviceID, Word transID);

  void handleCommandData(GeneSysLib::CmdEnum command,
                         GeneSysLib::DeviceID deviceID, Word transID,
                         GeneSysLib::commandData_t commandData);

  void handleUSBHostMIDIDeviceDetailData(GeneSysLib::CmdEnum command,
                                         GeneSysLib::DeviceID deviceID,
                                         Word transID,
                                         GeneSysLib::commandData_t commandData);

  void handleACKData(GeneSysLib::CmdEnum command, GeneSysLib::DeviceID deviceID,
                     Word transID, GeneSysLib::commandData_t commandData);

  void addQuerySysex(GeneSysLib::CmdEnum command);

  Bytes generate(GeneSysLib::CmdEnum command,
                 const GeneSysLib::commandData_t &commandData) const;

  // need this lock to prevent a crash
  NSLock *sendLock;

  // This is where everything is stored
  CommandDataMap storedCommandData;

  // Device ID for this device info instance
  GeneSysLib::DeviceID deviceID;

  // Trans ID for this device
  Word transID;

  // Screen performing query
  Screen queryScreen;

  int maxWriteItems;

  // Current Query
  std::list<GeneSysLib::CmdEnum> currentQuery;

  // IDs for all registered handlers
  std::map<GeneSysLib::CmdEnum, long> registeredHandlerIDs;

  // List of all pending queries
  std::queue<boost::tuple<Screen, std::list<GeneSysLib::CmdEnum> > >
      pendingQueries;

  // Pending sysex messages
  std::queue<Bytes> sysexMessages;

  // queries attemped
  std::set<GeneSysLib::CmdEnum> attemptedQueries;

  // Items previously queried
  std::list<GeneSysLib::CmdEnum> queriedItems;

  // Communicator pointer
  GeneSysLib::CommPtr comm;
};

typedef boost::shared_ptr<DeviceInfo> DeviceInfoPtr;

#endif  // __DEVICEINFORMATION_H__
