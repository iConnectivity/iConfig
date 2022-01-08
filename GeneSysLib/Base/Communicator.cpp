/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "Communicator.h"
#include "MyAlgorithms.h"

#ifndef Q_MOC_RUN
#include <boost/range/adaptors.hpp>
#endif

using namespace std;
using namespace boost;
using namespace boost::range;
using namespace boost::adaptors;

#ifdef __IOS__
////////////////////////////////////////////////////////////////////////////////
// IOS includes
////////////////////////////////////////////////////////////////////////////////
#include "MyAlgorithms.h"
#include "ICRunOnMain.h"

#include <CoreMIDI/CoreMIDI.h>
#include <boost/shared_ptr.hpp>

using namespace MyAlgorithms;

#define kTimeoutTime 3.0
#define kStartOfSysexMessage 0xF0
#define kEndOfSysexMessage 0xF7
#define kSysexStatusByte 0xF0

#define kStatusByteOffset 0

#define kNoteOffDataLength 2
#define kNoteOnDataLength 2
#define kPolyAftertouchDataLength 2
#define kControlChangeDataLength 2
#define kProgramChangeDataLength 1
#define kMonoAftertouchDataLength 1
#define kPitchBendDataLength 2

#define IsNoteOffStatusByte(Byte) ((Byte &kStatusMask) == kNoteOffStatusByte)
#define IsNoteOnStatusByte(Byte) ((Byte &kStatusMask) == kNoteOnStatusByte)
#define IsPolyAftertouchStatusByte(Byte) \
  ((Byte &kStatusMask) == kPolyAftertouchStatusByte)
#define IsControlChangeStatusByte(Byte) \
  ((Byte &kStatusMask) == kControlChangeStatusByte)
#define IsProgramChangeStatusByte(Byte) \
  ((Byte &kStatusMask) == kProgramChangeStatusByte)
#define IsMonoAftertouchStatusByte(Byte) \
  ((Byte &kStatusMask) == kMonoAftertouchStatusByte)
#define IsPitchBendStatusByte(Byte) \
  ((Byte &kStatusMask) == kPitchBendStatusByte)
#define IsSysexStatusByte(Byte) (Byte == kSysexStatusByte)

#define MAX_ATTEMPTS 3 //1  //zx, 2017-06-06

@interface TimeoutPoster () {
  GeneSysLib::Communicator *comm;
}

- (id)initWithCommunicator:(GeneSysLib::Communicator *)comm;

- (void)onTimeout;

@end

#pragma mark - C helper prototypes

void ICMIDINotifyProc(const MIDINotification *message, void *refCon);
void ICReadProc(const MIDIPacketList *pktlist, void *readProcRefCon,
                void *srcConnRefCon);
void sysexCommandSentCallback(MIDISysexSendRequest *request);

void midiNotifyProc(const MIDINotification *message, void *refCon);

Bytes lastSent;
int attemptCount;

@implementation TimeoutPoster

- (id)initWithCommunicator:(GeneSysLib::Communicator *)_comm {
  self = [super init];

  if (self) {
    comm = _comm;
  }

  return self;
}

- (void)onTimeout {
  [GeneSysLib::Communicator::finishLock lock];
  [comm->timeoutTimers removeObjectAtIndex:0];
  if (comm->timersEmpty()) {
    [GeneSysLib::Communicator::finishLock signal];
  }
  [GeneSysLib::Communicator::finishLock unlock];
  if (attemptCount > MAX_ATTEMPTS) {
    attemptCount = 0;
    runOnMain(^{
        [[NSNotificationCenter defaultCenter]
            postNotificationName:kCommunicationTimeout
                          object:nil];
    });
  } else if (lastSent.size() > 0) {
    attemptCount++;
    //NSLog(@"doing this lastSent send");
    comm->sendSysex(lastSent);
  }
}

@end

namespace GeneSysLib {

bool Communicator::pendingSend = false;

NSLock *Communicator::sendLock = nil;
NSCondition *Communicator::finishLock = nil;
NSLock *Communicator::timerLock = nil;

NSMutableArray *Communicator::timeoutTimers = nil;

TimeoutPoster *Communicator::timePoster = nil;

dispatch_queue_t _serialQueue = dispatch_queue_create("com.iconnectivity.sendSysex", DISPATCH_QUEUE_SERIAL);

#else  // !__IOS__
#include "RtMidi.h"

#include <QMutex>

QMutex sendMutex;
QMutex writeMutex;

namespace GeneSysLib {

using boost::begin;
using boost::end;

void readCallback(double, Bytes *data, void *userData) {

  //bugfixing: testing mutex already locked or not to
  //avoid dead lock situation
  // --zx, 2016-06-08
  if(writeMutex.tryLock() == false) {
     writeMutex.unlock();
     writeMutex.lock();
  }

  if (data) {
    Communicator *c = (Communicator *)userData;
    if (c) {
      if (c->timerThread) {
        c->timerThread->stopTimer();
      }
      if (c->m_parser) {
        c->m_parser->parse(*data);
      }
    }
  }
  writeMutex.unlock();
}

#endif  // __IOS__

Communicator::Communicator()
    : m_parser(new SysexParser()),
#ifndef __IOS__
      m_midiIn(),
      m_midiOut(),
#ifndef _WIN32
      m_probeIn(new RtMidiIn()),
      m_probeOut(new RtMidiOut()),
#endif  // _WIN32
#endif  // __IOS__
      currentOutPort() {
#ifdef __IOS__
  client = (MIDIClientRef)NULL;

  MIDIClientCreate(CFSTR("IC Client"), &ICMIDINotifyProc, nullptr, &client);

  if (client != (MIDIClientRef)NULL) {
    MIDIInputPortCreate(client, CFSTR("iConnectivity iConfig Input Port"),
                        &ICReadProc, static_cast<void *>(this), &sourcePort);
  }

  if (timePoster == nil) {
    timePoster = [[TimeoutPoster alloc] initWithCommunicator:this];
  }

  if (sendLock == nil) {
    sendLock = [[NSLock alloc] init];
  }

  if (finishLock == nil) {
    finishLock = [[NSCondition alloc] init];
  }

  if (timerLock == nil) {
    timerLock = [[NSLock alloc] init];
  }

  if (timeoutTimers == nil) {
    timeoutTimers = [[NSMutableArray alloc] init];
  }

#else   // NOT __IOS__
  timerThread = boost::shared_ptr<TimerThread>(new TimerThread());
  timerThread->start();
#endif  // __IOS__

  currentOutPort = -1;
}

Communicator::~Communicator(void) {
  closeAll();

#ifdef __IOS__
  if (sourcePort != (MIDIPortRef)NULL) {
    MIDIPortDispose(sourcePort);
    sourcePort = NULL;
  }

  if (client != (MIDIClientRef)NULL) {
    MIDIClientDispose(client);
  }
#endif  // __IOS__
}

unsigned int Communicator::getInCount() {
#ifdef __IOS__
  return static_cast<int>(MIDIGetNumberOfSources());
#else  // __IOS__
#ifdef _WIN32
  boost::shared_ptr<RtMidiIn> midiIn =
      boost::shared_ptr<RtMidiIn>(new RtMidiIn);

  return midiIn->getPortCount();
#else   // _WIN32
  return m_probeIn->getPortCount();
#endif  // _WIN32
#endif  // __IOS__
}

unsigned int Communicator::getOutCount() {
#ifdef __IOS__
  return static_cast<int>(MIDIGetNumberOfDestinations());
#else  // __IOS__
#ifdef _WIN32
  boost::shared_ptr<RtMidiOut> midiOut =
      boost::shared_ptr<RtMidiOut>(new RtMidiOut);

  return midiOut->getPortCount();
#else   // _WIN32
  return m_probeOut->getPortCount();
#endif  // _WIN32
#endif  // __IOS__
}

vector<string> Communicator::getInPorts() {
  vector<string> result;
#ifdef __IOS__
  ItemCount srcCount = MIDIGetNumberOfSources();

  for (auto i = 0; i < srcCount; ++i) {
    MIDIEndpointRef src = MIDIGetSource(i);

    if (src != (MIDIEndpointRef)NULL) {
      CFStringRef name = nil;
      if (noErr == MIDIObjectGetStringProperty(static_cast<MIDIObjectRef>(src),
                                               kMIDIPropertyDisplayName,
                                               &name)) {
        NSString *nsstr = (__bridge NSString *)name;
        auto str = string([nsstr cStringUsingEncoding:NSASCIIStringEncoding]);
        result.push_back(str);
      } else {
        result.push_back("");
      }
    }
  }

#else  // __IOS__
#ifdef _WIN32
  boost::shared_ptr<RtMidiIn> midiIn =
      boost::shared_ptr<RtMidiIn>(new RtMidiIn());

  for (unsigned int i = 0; i < getInCount(); ++i) {
    result += midiIn->getPortName(i);
  }
#else   // _WIN32
  for (unsigned int i = 0; i < getInCount(); ++i) {
    result += m_probeIn->getPortName(i);
  }
#endif  // _WIN32
#endif  // __IOS__
  return result;
}

vector<string> Communicator::getOutPorts() {
  vector<string> result;
#ifdef __IOS__
  ItemCount destCount = MIDIGetNumberOfDestinations();

  for (auto i = 0; i < destCount; ++i) {
    MIDIEndpointRef dest = MIDIGetDestination(i);

    if (dest != (MIDIEndpointRef)NULL) {
      CFStringRef name = nil;
      if (noErr == MIDIObjectGetStringProperty(static_cast<MIDIObjectRef>(dest),
                                               kMIDIPropertyDisplayName,
                                               &name)) {
        NSString *nsstr = (__bridge NSString *)name;
        auto str = string([nsstr cStringUsingEncoding:NSASCIIStringEncoding]);
        result.push_back(str);
      } else {
        result.push_back("");
      }
    }
  }
#else  // __IOS__

#ifdef _WIN32
  boost::shared_ptr<RtMidiOut> midiOut =
      boost::shared_ptr<RtMidiOut>(new RtMidiOut());

  for (unsigned int i = 0; i < getOutCount(); ++i) {
    result += midiOut->getPortName(i);
  }
#else   // !_WIN32
  for (unsigned int i = 0; i < getOutCount(); ++i) {
    result += m_probeOut->getPortName(i);
  }
#endif  // _WIN32

#endif  // __IOS__

  return result;
}

bool Communicator::openAllInputs() {
  bool result = true;

  closeInputs();

#ifdef __IOS__
  OSStatus status;
  status = MIDIRestart();

  if (status != noErr) {
    NSLog(@"MIDIRestart (%d)", (int)status);
  }
  else {
    NSLog(@"MIDIRestart OK");
  }

  for (unsigned int i = 0; i < MIDIGetNumberOfSources(); ++i) {
    // open input
    MIDIEndpointRef inEndPoint = MIDIGetSource(static_cast<ItemCount>(i));

    MIDIEntityRef entityRef;
    MIDIDeviceRef deviceRef;

    MIDIEndpointGetEntity(static_cast<MIDIEndpointRef>(inEndPoint), &entityRef);
    MIDIEntityGetDevice(entityRef, &deviceRef);

    if (sourcePort != (MIDIPortRef)NULL) {
      if (!contains(inDevices, deviceRef)) {
        status = MIDIPortConnectSource(sourcePort, inEndPoint, NULL);
        if (status == 0) {
          inEndPoints.push_back(inEndPoint);
          inDevices.push_back(deviceRef);
        }
      }
    } else {
      result = false;
      break;
    }
  }

#else  // __IOS__
  try {
    unsigned int inCount = getInCount();
    for (unsigned int i = 0; i < inCount; ++i) {
      auto pIn = auto_ptr<RtMidiIn>(new RtMidiIn());

      pIn->openPort(i);
      pIn->ignoreTypes(false);

      pIn->setCallback(readCallback, this);

      m_midiIn.push_back(pIn);
    }
  }
  catch (...) {
    closeInputs();
    m_midiIn.clear();
    result = false;
  }

#endif  // __IOS__
  return result;
}

bool Communicator::openOutput(unsigned int outPort) {
  bool result = true;

#ifdef __IOS__
  closeOutputs();

  if (outPort < MIDIGetNumberOfDestinations()) {
    // open output
    MIDIEndpointRef outEndPoint =
        MIDIGetDestination(static_cast<ItemCount>(outPort));

    outEndPoints.push_back(outEndPoint);
    currentOutPort = (unsigned int)outEndPoints.size() - 1;
  } else {
    result = false;
  }

#else   // !__IOS__

  closeOutputs();

  //bugfixing: testing mutex already being locked or not to
  //avoid dead lock situation
  // --zx, 2016-06-08
  if(writeMutex.tryLock() == false) {
     writeMutex.unlock();
     writeMutex.lock();
  }

  if (!MyAlgorithms::contains(m_midiOut, (int)outPort)) {
    try {
      const auto &output = boost::shared_ptr<RtMidiOut>(new RtMidiOut());
      m_midiOut[outPort] = output;
      output->openPort(outPort);
      currentOutPort = outPort;
    }
    catch (...) {
      writeMutex.unlock();
      closeOutputs();
      result = false;
    }
  } else {
    result = false;
  }
  writeMutex.unlock();
#endif  // __IOS__
  return result;
}

bool Communicator::openAllOutputs() {
  bool result = true;

  closeOutputs();

#ifdef __IOS__
  for (unsigned int outPort = 0; outPort < getOutCount(); ++outPort) {
    if (outPort < MIDIGetNumberOfDestinations()) {
      // open output
      MIDIEndpointRef outEndPoint =
          MIDIGetDestination(static_cast<ItemCount>(outPort));
      if (outEndPoint) outEndPoints.push_back(outEndPoint);
    } else {
      result = false;
    }
  }

  MIDIEndpointRef rtpOut =
      [[MIDINetworkSession defaultSession] destinationEndpoint];
  if (rtpOut) {
    outEndPoints.push_back(rtpOut);
  }

  // set the current out port to the first available port
  if (outEndPoints.size() > 0) {
    currentOutPort = 0;
  }

#else   // NOT __IOS__

  //Bugfixing: MIDI communication error hanging software
  // by iterately calling mutex lock.
  //
  //Cause: the "closeOutput" in "catch" block calling mutex
  // lock before it is unlocked leading dead-lock issue and
  // hang software.
  //
  // Solution: Adding mutex lock checking and unlock mutex before
  // "closeOutput" called.
  // --zx, 2016-06-08
  bool bLocked = true;

  //bugfixing: testing mutex already being locked or not to
  //avoid dead lock situation
  // --zx, 2016-06-08
  if(writeMutex.tryLock() == false) {
     writeMutex.unlock();
     writeMutex.lock();
  }

  currentOutPort = ((getOutCount() > 0) ? (0) : (-1));

  for (unsigned int outPort = 0; outPort < getOutCount(); ++outPort) {
    if (!MyAlgorithms::contains(m_midiOut, (int)outPort)) {
      try {
        boost::shared_ptr<RtMidiOut> output =
            boost::shared_ptr<RtMidiOut>(new RtMidiOut());
        m_midiOut[outPort] = output;
        try {
            output->openPort(outPort);
        }  catch (...) {
            // Bugfixing: --zx, 2016-12-01
            m_midiOut.erase(outPort);
        }
      }
      catch (...) {
      //Bugfixing: --zx, 2016-06-08
       writeMutex.unlock();
       bLocked = false;

       closeOutputs();
       result = false;
       break;
      }
    } else {
        //Bugfixing: --zx, 2016-06-08
        writeMutex.unlock();
        bLocked = false;

        closeOutputs();
        result = false;
        break;
    }
  }

  //Bugfixing: --zx, 2016-06-08
  if(bLocked == true) {
      bLocked = false;
      writeMutex.unlock();
  }
#endif  // __IOS__

  return result;
}

bool Communicator::closeInputs() {
  bool result = true;

#ifdef __IOS__

  if (sourcePort != (MIDIPortRef)NULL) {
    for (MIDIEndpointRef inEndPoint : inEndPoints) {
      if (inEndPoint != (MIDIEndpointRef)NULL) {
        MIDIPortDisconnectSource(sourcePort, inEndPoint);
      } else {
        result = false;
      }
    }
    inEndPoints.clear();
    inDevices.clear();
  } else {
    result = false;
  }

#else  // __IOS__

  for (auto &in : m_midiIn) {
    in.closePort();
  }
  m_midiIn.clear();

#endif  // __IOS__

  return result;
}

bool Communicator::closeOutputs() {
  bool result = true;

  // Set to unused output
  currentOutPort = -1;

#ifdef __IOS__

  for (auto outEndPoint : outEndPoints) {
    MIDIFlushOutput(outEndPoint);
  }

  outEndPoints.clear();

#else  // __IOS__

  //bugfixing: testing mutex already being locked or not to
  //avoid dead lock situation
  // --zx, 2016-06-08
  if(writeMutex.tryLock() == false) {
     writeMutex.unlock();
     writeMutex.lock();
  }

  for (auto mOut : m_midiOut | map_values) {
    mOut->closePort();
    mOut.reset();
  }
  m_midiOut.clear();
  writeMutex.unlock();

#endif  // __IOS__

  return result;
}

bool Communicator::closeAll() {
  bool result = closeInputs();
  result |= closeOutputs();
  return result;
}

#ifdef _WIN32
bool Communicator::isNewWindowsDriver()
{
#if 0
    if ((MyAlgorithms::contains(m_midiOut, (int)currentOutPort)) &&
        m_midiOut.at(currentOutPort)) {
      const auto &outPort = m_midiOut.at(currentOutPort);
      return outPort->isNewWindowsDriver();
    }
    else {
      return false;
    }
#endif
    return false;
}

#endif

void Communicator::setCurrentOutput(unsigned int outPort) {
#ifdef __IOS__
  if (outPort < outEndPoints.size()) {
#else   // NOT __IOS__
  if (MyAlgorithms::contains(m_midiOut, (int)outPort)) {
#endif  // __IOS__
    currentOutPort = outPort;
  }
}

void Communicator::sendSysex(const Bytes &sysex) {
#ifdef __IOS__
  Bytes sysCopy(sysex);
//  dispatch_async(_serialQueue, ^{
//    Bytes sysex(sysCopy);
  NSLog(@"sendSysex %02X%02X", sysex.at(14), sysex.at(15));
  NSMutableString *str = [[NSMutableString alloc] init];
  [str appendString:@"sysex = [ "];
  for (const auto &b : sysex) {
    [str appendString:[NSString stringWithFormat:@"%02X ", b]];
  }
  [str appendString:@"]"];
  NSLog(@"%@", str);


  lastSent = sysex;

  static Byte tempSysex[1024];

  if (currentOutPort < outEndPoints.size() &&
      outEndPoints[currentOutPort] != (MIDIEndpointRef)NULL) {

    BOOL wait = true;

    while (wait) {
      [sendLock lock];
      wait = pendingSend;
      if (!wait)
        pendingSend = true;
      [sendLock unlock];
      if (wait) {
        [[NSRunLoop currentRunLoop]
         runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.01]];
      }
    }

    MIDISysexSendRequest *request =
    (MIDISysexSendRequest *)malloc(sizeof(MIDISysexSendRequest));
    memset(request, 0, sizeof(MIDISysexSendRequest));

    copy(sysex.begin(), sysex.end(), tempSysex);
    NSLog(@"Sending %ld -> Last = [%02X]", sysex.size(), sysex[sysex.size() - 1]);

    request->destination = outEndPoints[currentOutPort];
    request->data = tempSysex;  //.data();
    request->bytesToSend = (UInt32)sysex.size();
    request->complete = NO;
    request->completionProc = &sysexCommandSentCallback;
    request->completionRefCon = request;

    //Communicator::startTimer();
    OSStatus status = MIDISendSysex(request);
    //??NSLogError( status, @"MIDISendSysex");

    [[NSRunLoop currentRunLoop]
     runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.01]];

    if (FAILED(status)) {
      NSLog(@"FAILED(status)");
      pendingSend = false;
      free(request);
    }

    //Communicator::waitForAllTimers();
  }
 // });

#else  // NOT __IOS__

  //bugfixing:Checking timeout timer existance and stop
  // running timeout timer to avoid timeout event
  // interupting during slow communcation process.
  // -- zx, 2016-06-08
  //if(timerThread->isTimerActive() == true)
  timerThread->stopTimer();

  //bugfixing: testing mutex already being locked or not to
  //avoid dead lock situation
  // --zx, 2016-06-08
  if(sendMutex.tryLock() == false) {
     sendMutex.unlock();
     sendMutex.lock();
  }

  //bugfxing: flag if send command is not succeded, don't start
  //timeout timer.
  //--zx,2016-06-08
  bool bSucceded = true;

  if ((MyAlgorithms::contains(m_midiOut, (int)currentOutPort)) &&
      m_midiOut.at(currentOutPort)) {
    Bytes sendBytes = sysex;

    try {
      const auto &outPort = m_midiOut.at(currentOutPort);
      outPort->sendMessage(&sendBytes);
    }
    catch (...) {
      //bugfxing: If send command is not succeded, unlock mutex and flag
      //the succeded as false.
      //--zx,2016-06-08
      bSucceded = false;
      sendMutex.unlock();

      closeAll();
    }
  }

  timerThread->startTimer();

  //bugfxing: If send command is succeded, unlock mutex.
  //--zx,2016-06-08
  if(bSucceded == false) {
    sendMutex.unlock();
  }
#endif  // __IOS__
}

void Communicator::reset() {
//#ifndef __IOS__
//  for (auto inIter = m_midiIn.begin(); inIter != m_midiIn.end(); ++inIter) {
//    inIter->reset();
//  }
//#endif
    ;
}

void Communicator::parseBytes(BytesIter &beginIter, BytesIter &endIter, DeviceID deviceID) {
  if (m_parser) {
    auto blockStart = find(beginIter, endIter, (Byte)0xF0);
    auto blockEnd = find(blockStart, endIter, (Byte)0xF7);

    while ((blockStart != endIter) && (blockEnd != endIter)) {
      Bytes block;
      block.insert(block.end(), blockStart, blockEnd + 1);

      if (block[7] == 0 &&
          block[8] == 0 &&
          block[9] == 0 &&
          block[10] == 0 &&
          block[11] == 0) { // serial number is 0. replace w/ ours!
        SerialNumber sn = deviceID.serialNumber();
        block[7] = sn[0];
        block[8] = sn[1];
        block[9] = sn[2];
        block[10] = sn[3];
        block[11] = sn[4];

        int acc = 0;
        for (size_t i = 5; i < block.size() - 2; i++) {
          acc += block[i];
        }
        block[block.size() - 2] = (~(acc) + 1) & 0x7F; // redo crc
      }

      m_parser->parse(block);

      if (blockEnd == endIter) {
        break;
      }

      blockStart = find(blockEnd, endIter, (Byte)0xF0);
      blockEnd = find(blockStart, endIter, (Byte)0xF7);
    }
  }
}

long Communicator::registerHandler(CmdEnum commandID, Handler handler) {
  if (m_parser) {
    return m_parser->registerHandler(commandID, handler);
  } else {
    return -1;
  }
}

void Communicator::unRegisterHandler(CmdEnum commandID) {
  if (m_parser) {
    m_parser->unRegisterHandler(commandID);
  }
}

void Communicator::unRegisterHandler(CmdEnum commandID, long handlerID) {
  if (m_parser) {
    m_parser->unRegisterHandler(commandID, handlerID);
  }
}

void Communicator::unRegisterAll() {
  if (m_parser) {
    m_parser->unRegisterAll();
  }
}

void Communicator::registerExclusiveHandler(CmdEnum commandID,
                                            Handler handler) {
  if (m_parser) {
    m_parser->registerExclusiveHandler(commandID, handler);
  }
}

void Communicator::unRegisterExclusiveHandler() {
  if (m_parser) {
    m_parser->unRegisterExclusiveHandler();
  }
}

#ifdef __IOS__
void Communicator::startTimer() {
  NSLog(@"starting Timer");
  runOnMain(^{
    [GeneSysLib::Communicator::finishLock lock];
      NSLog(@"starting %lu timer on main", [timeoutTimers count] + 1L);
      //zx, 2017-06-16
/*      NSTimer *timeoutTimer = [NSTimer timerWithTimeInterval:kTimeoutTime
                                             target:timePoster
                                           selector:@selector(onTimeout)
                                           userInfo:nil
                                            repeats:NO];
      [[NSRunLoop currentRunLoop] addTimer:timeoutTimer
                                   forMode:NSDefaultRunLoopMode]; */
      NSTimer *timeoutTimer = [NSTimer scheduledTimerWithTimeInterval:kTimeoutTime
                                                      target:timePoster
                                                    selector:@selector(onTimeout)
                                                    userInfo:nil
                                                     repeats:NO];
      
    [timeoutTimers addObject:timeoutTimer];
    [GeneSysLib::Communicator::finishLock unlock];
  });
}
#endif  // __IOS__

#if __IOS__
void Communicator::stopTimer() {
  runOnMain(^{
    NSLog(@"timer %lu stopping", (unsigned long)[timeoutTimers count]);
    [Communicator::finishLock lock];
    if ([timeoutTimers count]) {
      NSLog(@"timer %lu stopped on main", (unsigned long)[timeoutTimers count]);
      NSTimer *timeoutTimer = [timeoutTimers objectAtIndex:0];
      [timeoutTimer invalidate];
      timeoutTimer = nil;

      [timeoutTimers removeObjectAtIndex:0];
    }
    if (![timeoutTimers count]) {
      NSLog(@"signalling");
      [Communicator::finishLock broadcast];
    }
    else {
      NSLog(@"not signalling");
    }
    [Communicator::finishLock unlock];
  });
}
#endif  // __IOS__

#if __IOS__
bool Communicator::timersEmpty() {
  return ([timeoutTimers count] == 0);
}
#endif  // __IOS__

#if __IOS__
void Communicator::waitForAllTimers() {
  [finishLock lock];
  while (!timersEmpty())
    [finishLock wait];
  [finishLock unlock];
}
#endif  // __IOS__

#if __IOS__
  void Communicator::cancelAllTimers() {
    [finishLock lock];
    while ([timeoutTimers count]) {
      [[timeoutTimers objectAtIndex:0] invalidate];
      [timeoutTimers removeObjectAtIndex:0];
    }

    [finishLock signal];
    [finishLock unlock];
  }
#endif  // __IOS__

}  // namespace GeneSysLib

#ifdef __IOS__
#pragma mark - C helper implementations

void ICMIDINotifyProc(const MIDINotification *message, void *refCon) {}

void ICReadProc(const MIDIPacketList *pktlist, void *readProcRefCon,
                void *srcConnRefCon) {
  NSLog(@"Recv");
  static BOOL inSysexMessage = NO;
  static Bytes currentBuffer;

  const MIDIPacket *pkt = &pktlist->packet[0];
  auto *const comm = static_cast<GeneSysLib::Communicator *>(readProcRefCon);
  auto parser = comm->m_parser;

  if (parser != nullptr) {
    for (UInt32 i = 0; i < pktlist->numPackets; i++) {
      if (!inSysexMessage) {
        Byte statusByte = pkt->data[kStatusByteOffset];
        if (IsSysexStatusByte(statusByte)) {
          // copy the rest of this packet (up until the end of
          // message)
          inSysexMessage = YES;

          // clear the currentBuffer
          currentBuffer.clear();

          for (UInt16 j = 0; j < pkt->length; j++) {
            // copy Byte
            currentBuffer.push_back(pkt->data[j]);
            if (pkt->data[j] == kEndOfSysexMessage) {
              inSysexMessage = NO;

              // Stop the timer
              comm->stopTimer();

              NSMutableString *str = [[NSMutableString alloc] init];
              [str appendString:@"currentBuffer1 (Recv) = [ "];
              for (const auto &b : currentBuffer) {
                [str appendString:[NSString stringWithFormat:@"%02X ", b]];
              }
              [str appendString:@"]"];
              NSLog(@"%@", str);

              // Parse here
              if (parser->parse(currentBuffer)) {
                NSLog(@"Parse Failed 1!!!");

                // Start time timeout
                comm->startTimer();
              } else {
                attemptCount = 0;
              }
              currentBuffer.clear();
            }
          }
        }
      } else {  // In Sysex Message
        for (UInt16 j = 0; j < pkt->length; j++) {
          currentBuffer.push_back(pkt->data[j]);
          if (pkt->data[j] == kEndOfSysexMessage) {
            inSysexMessage = NO;

            // Stop the timer
            comm->stopTimer();

            NSMutableString *str = [[NSMutableString alloc] init];
            [str appendString:@"currentBuffer2 (Recv) = [ "];
            for (const auto &b : currentBuffer) {
              [str appendString:[NSString stringWithFormat:@"%02X ", b]];
            }
            [str appendString:@"]"];
            NSLog(@"%@", str);

            // Parse here
            if (parser->parse(currentBuffer)) {
              NSLog(@"Parse Failed 2!!!");

              // Start time timeout
              comm->startTimer();
            } else {
              attemptCount = 0;
            }
            currentBuffer.clear();
          }
        }
      }
      pkt = MIDIPacketNext(pkt);
    }
  }
  else {
    NSLog(@"parser is null?");
  }
}

void sysexCommandSentCallback(MIDISysexSendRequest *request) {
  //NSLog(@"Sent complete? %@", (request->complete) ? (@"Yes") : (@"No"));
  //NSLog(@"Sent Complete");
  GeneSysLib::Communicator::pendingSend = false;
  free(request);
}

void midiNotifyProc(const MIDINotification *message, void *refCon) {}

#endif  // __IOS__
