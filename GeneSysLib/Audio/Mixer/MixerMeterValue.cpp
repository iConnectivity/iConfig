/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "MixerMeterValue.h"
#include <limits>

using namespace boost;
using namespace std;

#ifdef __IOS__
NSLock *mixerMeterLock = nil;
#else
#include <QMutex>
QMutex mixerMeterMutex;
#endif

namespace GeneSysLib {

commandDataKey_t MixerMeterValue::minKey() {
  return generateKey(Command::RetMixerMeterValue);
}

commandDataKey_t MixerMeterValue::maxKey() {
  return MixerMeterValue::queryKey(std::numeric_limits<Word>::max(), std::numeric_limits<Byte>::max());
}

commandDataKey_t MixerMeterValue::queryKey(Word audioPortID, Byte mixerOutputNumber) {
  return generateKey(Command::RetMixerMeterValue, audioPortID, mixerOutputNumber);
}

MixerMeterValue::MixerMeterValue() {}

const commandDataKey_t MixerMeterValue::key() const {
  return generateKey(Command::RetMixerMeterValue, audioPortID(), mixerOutputNumber());
}

CmdEnum MixerMeterValue::retCommand() { return Command::RetMixerMeterValue; }

Bytes MixerMeterValue::generate() const {
  Bytes result;

  result += versionNumber();
  appendMidiWord(result, audioPortID());
  appendMidiByte(result, mixerOutputNumber());
  result += static_cast<Byte>(meterType.to_ulong());

  return result;
}

void MixerMeterValue::parse(BytesIter &beginIter, BytesIter &endIter) {
  Byte version = nextMidiByte(beginIter, endIter);

  if (version == versionNumber()) {
#ifdef __IOS__
    if (mixerMeterLock == nil) {
      mixerMeterLock = [[NSLock alloc] init];
    }
    [mixerMeterLock lock];
    //NSLog(@"MixerMeter parse start");
#else
    mixerMeterMutex.lock();
#endif
    audioPortID = roWord(nextMidiWord(beginIter, endIter));
    mixerOutputNumber = roByte(nextMidiByte(beginIter, endIter));
    if (meterBlocks) {
      for (int i = 0; i < meterBlockCount(); i++) {
        if (meterBlocks[i].meterValues) {
          free(meterBlocks[i].meterValues);
        }
      }
      free(meterBlocks);
    }
    meterBlockCount = roByte(nextMidiByte(beginIter, endIter));
    meterBlocks = (MeterBlock*) malloc(sizeof(MeterBlock) * meterBlockCount());

    for (int i = 0; i < meterBlockCount(); i++) {
      MeterBlock mb;
      mb.meterType = std::bitset<8>(nextMidiByte(beginIter, endIter));
      mb.meterValueCount = roByte(nextMidiByte(beginIter, endIter));
      mb.meterValues = (roWord*) malloc(sizeof(roWord) * mb.meterValueCount());
      for (int j = 0; j < mb.meterValueCount(); j++) {
        mb.meterValues[j] = roWord(nextMidiWord(beginIter,endIter));
      }
      meterBlocks[i] = mb;
      //NSLog(@"Mixer: Added a meterBlock!");
    }
#ifdef __IOS__
    [mixerMeterLock unlock];
    //NSLog(@"MixerMeter parse end");
#else
    mixerMeterMutex.unlock();
#endif
  }
}

Byte MixerMeterValue::versionNumber() const { return 0x01; }

Word MixerMeterValue::outputMeter() const
{
#ifdef __IOS__
  if (mixerMeterLock == nil) {
    mixerMeterLock = [[NSLock alloc] init];
  }
  [mixerMeterLock lock];
  //NSLog(@"O lock");
#else
  mixerMeterMutex.lock();
#endif
  Word toRet = 0;
  if (meterBlockCount() > 1) {
    if (meterBlocks[1].meterValueCount() > 0)
      toRet = meterBlocks[1].meterValues[0]();
  }
#ifdef __IOS__
  [mixerMeterLock unlock];
  //NSLog(@"O unlock");
#else
  mixerMeterMutex.unlock();
#endif
  return toRet;
}

Word MixerMeterValue::inputMeter(Byte mixerInputNumber) const
{
#ifdef __IOS__
  if (mixerMeterLock == nil) {
    mixerMeterLock = [[NSLock alloc] init];
  }
  //NSLog(@"I lock");
  [mixerMeterLock lock];
#else
  mixerMeterMutex.lock();
#endif
  Word toRet = 0;
  if (meterBlockCount() > 0) {
    if (meterBlocks[0].meterValueCount() >= mixerInputNumber)
      toRet = meterBlocks[0].meterValues[mixerInputNumber - 1]();
  }
#ifdef __IOS__
  [mixerMeterLock unlock];
  //NSLog(@"I unlock");
#else
  mixerMeterMutex.unlock();
#endif
  return toRet;
}

}  // namespace GeneSysLib

