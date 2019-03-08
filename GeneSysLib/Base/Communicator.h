/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __COMMUNICATOR_H__
#define __COMMUNICATOR_H__

#include "LibTypes.h"
#include "ACK.h"
#include "Generator.h"
#include "SysexParser.h"

#ifndef Q_MOC_RUN
#include <boost/shared_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#endif

#ifndef __IOS__
////////////////////////////////////////////////////////////////////////////////
// IOS not includes
////////////////////////////////////////////////////////////////////////////////
#include "DeviceID.h"
#include "IPMode.h"
#include "RtMidi.h"
#include "TimerThread.h"

#ifndef Q_MOC_RUN
#include <boost/tuple/tuple.hpp>
#endif

using namespace boost::tuples;
#else  // __IOS__
////////////////////////////////////////////////////////////////////////////////
// IOS includes
////////////////////////////////////////////////////////////////////////////////
#import "MIDIEndPoint.h"
@interface TimeoutPoster : NSObject
@end
#define kCommunicationTimeout @"CommunicationTimeout"
#endif  // __IOS__

using boost::ptr_vector;

namespace GeneSysLib {

struct Communicator {
  Communicator();
  virtual ~Communicator(void);

  unsigned int getInCount();
  unsigned int getOutCount();

  std::vector<std::string> getInPorts();
  std::vector<std::string> getOutPorts();

  bool openAllInputs();
  bool openOutput(unsigned int outPort);
  bool openAllOutputs();
  bool closeInputs();
  bool closeOutputs();
  bool closeAll();
#ifdef _WIN32
  bool isNewWindowsDriver();
#endif
  void setCurrentOutput(unsigned int outPort);

  void sendSysex(const Bytes &sysex);

  void reset();

  void parseBytes(BytesIter &start, BytesIter &finish, DeviceID deviceID);

  long registerHandler(CmdEnum commandID, Handler handler);

  void unRegisterHandler(CmdEnum commandID);
  void unRegisterHandler(CmdEnum commandID, long handlerID);
  void unRegisterAll();

  void registerExclusiveHandler(CmdEnum commandID, Handler handler);
  void unRegisterExclusiveHandler();

#ifndef __IOS__
  //////////////////////////////////////////////////////////////////////////////
  // Not IOS variables
  //////////////////////////////////////////////////////////////////////////////
  boost::shared_ptr<TimerThread> timerThread;
#endif  //__IOS__

  boost::shared_ptr<SysexParser> m_parser;

 private:
#ifdef __IOS__
  //////////////////////////////////////////////////////////////////////////////
  // IOS variables
  //////////////////////////////////////////////////////////////////////////////
  MIDIClientRef client;
  MIDIPortRef sourcePort;

  std::vector<MIDIEndpointRef> inEndPoints;
  std::vector<MIDIEndpointRef> outEndPoints;
  std::vector<MIDIDeviceRef> inDevices;

 public:
  static NSLock *sendLock;
  static NSLock *timerLock;
  static NSCondition *finishLock;
  static bool pendingCallback;
  static bool pendingSend;

  static void startTimer();
  static void stopTimer();
  static bool timersEmpty();

  static void waitForAllTimers();
  static void cancelAllTimers();

  static NSMutableArray *timeoutTimers;

  static TimeoutPoster *timePoster;

#else  // !__IOS__
  //////////////////////////////////////////////////////////////////////////////
  // Not IOS variables
  //////////////////////////////////////////////////////////////////////////////
  ptr_vector<RtMidiIn> m_midiIn;
  std::map<int, boost::shared_ptr<RtMidiOut> > m_midiOut;
#ifndef _WIN32
  //////////////////////////////////////////////////////////////////////////////
  // Not Win32 variables
  //////////////////////////////////////////////////////////////////////////////
  boost::shared_ptr<RtMidiIn> m_probeIn;
  boost::shared_ptr<RtMidiOut> m_probeOut;
#endif  // _WIN32
#endif  // !__IOS__

  unsigned int currentOutPort;
};  // struct Communicator

typedef boost::shared_ptr<Communicator> CommPtr;

}  // namespace GeneSysLib

#endif  // __COMMUNICATOR_H__
