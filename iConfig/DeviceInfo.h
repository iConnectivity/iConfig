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
#include "CommandQList.h"
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
#include <QMutex>

#ifndef Q_MOC_RUN
#include <boost/algorithm/cxx11/any_of.hpp>
#endif

using GeneSysLib::CommPtr;
using GeneSysLib::commandDataKey_t;
using GeneSysLib::commandData_t;
using GeneSysLib::DeviceID;
using GeneSysLib::Info;
using GeneSysLib::MIDIPortRemap;
using GeneSysLib::MIDIPortFilter;

extern QMutex sysexMutex;

class DeviceInfo : public QObject {
  Q_OBJECT
 public:
  explicit DeviceInfo(CommPtr comm, QObject *parent = 0);
  explicit DeviceInfo(CommPtr comm, DeviceID deviceID, Word transID = 0x00,
                      QObject *parent = 0);
  virtual ~DeviceInfo();

  std::pair<DeviceID, Word> getInfo() const;
  DeviceID getDeviceID();
  Word getPID();
  SerialNumber getSerialNumber();
  Word getTransID();

#ifdef _WIN32
  bool isNewWindowsDriver();
#endif

  typedef std::map<commandDataKey_t, commandData_t> CommandDataMap;
  typedef CommandDataMap::iterator CommandDataIterator;

  inline void send(const Bytes &sysex) { comm->sendSysex(sysex); }

  template <typename DATA_T, typename... Ts> void send(Ts... vs) {
    DATA_T(deviceID, transID, vs...).send(comm);
  }

  bool startQuery(Screen screen, const CommandQList &query);
  bool rereadAudioInfo();
  bool rereadStored();
  bool rereadAudioControls();
  bool rereadMixerControls();
  bool rereadMeters();

  // Command Information
  void addCommandData(commandData_t commandData);
  bool containsCommandDataType(GeneSysLib::CmdEnum command) const;
  bool containsCommandData(GeneSysLib::CmdEnum command) const;
  bool contains(const commandDataKey_t &key) const;

  // generic
  template <typename T> size_t typeCount() const {
    const auto &lower = storedCommandData.lower_bound(T::minKey());
    const auto &upper = storedCommandData.upper_bound(T::maxKey());
    return std::distance(lower, upper);
  }

  template <typename T> bool containsType() const { return typeCount<T>() > 0; }

  // This method will return true if the storedCommandData map contains the key
  // generated with the command and the variadic list of parameters
  template <typename T, typename... Ts> bool contains(Ts... ts) const {
    /*//printf("contains--\n");
    for(auto it = storedCommandData.cbegin(); it != storedCommandData.cend(); ++it)
    {
      for( std::vector<unsigned char>::const_iterator i = it->first.begin(); i != it->first.end(); ++i)
          //printf("%.2X ", *i);
      //printf("\n");
    }*/
    return MyAlgorithms::contains(
        storedCommandData,
        GeneSysLib::generateKey<Ts...>(T::retCommand(), ts...));
  }

  template <typename T> T &get(const commandDataKey_t &key) {
    /*if (key == GeneSysLib::generateKey(T::retCommand())) {
      //printf("About to assert on %2X!\n", T::retCommand());
    }
    else {
      //printf("Doesn't match %2X?!\n", T::retCommand());
    }*/
    Q_ASSERT(contains(key));
    return this->storedCommandData.at(key).get<T>();
  }

  template <typename T> const T &get(const commandDataKey_t &key) const {
    Q_ASSERT(contains(key));
    return this->storedCommandData.at(key).get<T>();
  }

  template <typename T> T &get() {
    Q_ASSERT(contains<T>());
    return storedCommandData.at(GeneSysLib::generateKey(T::retCommand()))
        .template get<T>();
  }

  template <typename T> const T &get() const {
    Q_ASSERT(contains<T>());
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
  MIDIPortRemap &midiPortRemap(Word portID, GeneSysLib::RemapTypeEnum remapType);
  const MIDIPortRemap &midiPortRemap(Word portID,
                                     GeneSysLib::RemapTypeEnum remapType) const;

  // Audio Port Info
  size_t audioPortInfoCount() const;

  void replaceChecksumByte(unsigned char *arr, size_t size);

  Bytes serialize();
  bool deserialize(Bytes data);

  Bytes serialize2(std::set<GeneSysLib::Command::Enum> commandsToSave, QString description = "");
  Bytes serialize2midi(std::set<GeneSysLib::Command::Enum> commandsToSave, bool reboot);
  bool deserialize2(Bytes data);

  std::vector<GeneSysLib::USBHostMIDIDeviceDetail> usbHostMIDIDeviceDetails;

  long registerHandler(GeneSysLib::CmdEnum commandID, Handler handler);

  void unRegisterHandler(GeneSysLib::CmdEnum commandID);
  void unRegisterHandler(GeneSysLib::CmdEnum commandID, long handlerID);
  void unRegisterAll();

  void registerExclusiveHandler(GeneSysLib::CmdEnum commandID, Handler handler);
  void unRegisterExclusiveHandler();

signals:
  void queryStarted();
  void queryCompleted(Screen screen, CommandQList foundItems);

  void writingStarted(int max);
  void writingProgress(int value);
  void writeCompleted();

  void sendStart(int msec);
  void sendStop();

 public slots:
  void timeout();
  bool sendNextSysex();

 private:
  void registerAllHandlers();
  void unRegisterHandlerAllHandlers();

  void addCommand(const Bytes &sysex);
  void addCommand(const Bytes &&sysex);

  template <typename T> void addCommand(const T &command) {
//    sysexMutex.lock();
    sysexMessages.push(command.sysex());
//    sysexMutex.unlock();
  }

  template <typename T> void addCommand(const T &&command) {
//    sysexMutex.lock();
    sysexMessages.push(command.sysex());
//    sysexMutex.unlock();
  }

  template <typename T, typename... Ts> void addCommand(Ts... vs) {
//    sysexMutex.lock();
    sysexMessages.push(T(deviceID, transID, vs...).sysex());
//    sysexMutex.unlock();
  }

  void writeAll();

  bool commonHandleCode(DeviceID deviceID, Word transID);

  void handleCommandData(GeneSysLib::CmdEnum command, DeviceID deviceID,
                         Word transID, commandData_t commandData);
  void handleUSBHostMIDIDeviceDetailData(GeneSysLib::CmdEnum command,
                                         DeviceID deviceID, Word transID,
                                         commandData_t commandData);
  void handleACKData(GeneSysLib::CmdEnum command, DeviceID deviceID,
                     Word transID, commandData_t commandData);

  void addQuerySysex(GeneSysLib::CmdEnum command);

  Bytes generate(GeneSysLib::CmdEnum command,
                 const commandData_t &commandData) const;

  CommandDataMap storedCommandData;

  DeviceID deviceID;
  Word transID;

  CommandQList queriedItems;
  Screen queryScreen;
  int maxWriteItems;
  std::list<GeneSysLib::CmdEnum> currentQuery;
  std::map<GeneSysLib::CmdEnum, long> registeredHandlerIDs;
  std::queue<boost::tuple<Screen, std::list<GeneSysLib::CmdEnum> > >
      pendingQueries;
  std::queue<Bytes> sysexMessages;
  std::set<GeneSysLib::CmdEnum> attemptedQueries;
  CommPtr comm;
};

typedef boost::shared_ptr<DeviceInfo> DeviceInfoPtr;

#endif  // __DEVICEINFORMATION_H__
