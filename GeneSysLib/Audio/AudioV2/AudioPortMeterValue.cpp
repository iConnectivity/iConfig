/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "AudioPortMeterValue.h"
#include <limits>

#ifdef __IOS__
NSLock *audioMeterLock = nil;
#else
#include <QMutex>
QMutex audioMeterMutex;
#endif

using namespace boost;
using namespace std;


namespace GeneSysLib {

commandDataKey_t AudioPortMeterValue::minKey() {
  return generateKey(Command::RetAudioPortMeterValue);
}

commandDataKey_t AudioPortMeterValue::maxKey() {
  return AudioPortMeterValue::queryKey(std::numeric_limits<Word>::max());
}

commandDataKey_t AudioPortMeterValue::queryKey(Word audioPortID) {
  return generateKey(Command::RetAudioPortMeterValue, audioPortID);
}

AudioPortMeterValue::AudioPortMeterValue() {}

const commandDataKey_t AudioPortMeterValue::key() const {
  return generateKey(Command::RetAudioPortMeterValue, audioPortID());
}

CmdEnum AudioPortMeterValue::retCommand() { return Command::RetAudioPortMeterValue; }

Bytes AudioPortMeterValue::generate() const {
  Bytes result;

  result += versionNumber();
  appendMidiWord(result, audioPortID());
  result += static_cast<Byte>(meterType.to_ulong());

  return result;
}

void AudioPortMeterValue::parse(BytesIter &beginIter, BytesIter &endIter) {
  Byte version = nextMidiByte(beginIter, endIter);

#ifdef __IOS__
  if (audioMeterLock == nil) {
    audioMeterLock = [[NSLock alloc] init];
  }
#endif
  
  if (version == versionNumber()) {
#ifdef __IOS__
    [audioMeterLock lock];
    //NSLog(@"AudioPortMeter parse start");
#else
    audioMeterMutex.lock();
#endif
    audioPortID = roWord(nextMidiWord(beginIter, endIter));
    if (meterBlocks) {
      for (int i = 0; i < meterBlockCount(); i++) {
        if (meterBlocks[i].meterValues) {
          free(meterBlocks[i].meterValues);
        }
      }
      free(meterBlocks);
    }
    meterBlockCount = roByte(nextMidiByte(beginIter, endIter));
    meterBlocks = (MeterBlock*) calloc(meterBlockCount(), sizeof(MeterBlock));

    for (int i = 0; i < meterBlockCount(); i++) {
      MeterBlock mb;
      mb.meterType = std::bitset<8>(nextMidiByte(beginIter, endIter));
      mb.meterValueCount = roByte(nextMidiByte(beginIter, endIter));
      mb.meterValues = (roWord*) calloc(mb.meterValueCount(), sizeof(roWord));

      for (int j = 0; j < mb.meterValueCount(); j++) {
        mb.meterValues[j] = roWord(nextMidiWord(beginIter,endIter));
      }
      meterBlocks[i] = mb;
      //NSLog(@"Audio: Added a meterBlock!");
    }
#ifdef __IOS__
    [audioMeterLock unlock];
    //NSLog(@"AudioPortMeter parse end");
#else
    audioMeterMutex.unlock();
#endif
  }
}

Word AudioPortMeterValue::meterValue(int blockIndex, int meterIndex) const {
  Word toRet = 0;
#ifdef __IOS__

  if (audioMeterLock == nil) {
    audioMeterLock = [[NSLock alloc] init];
  }
  [audioMeterLock lock];
#else
  audioMeterMutex.lock();
#endif
    if ((int)meterBlockCount() > (int)blockIndex) {
      if ((int)meterBlocks[blockIndex].meterValueCount() > (int)meterIndex) {
        toRet = meterBlocks[blockIndex].meterValues[meterIndex]();
      }
    }
#ifdef __IOS__
  [audioMeterLock unlock];
#else
  audioMeterMutex.unlock();
#endif
  return toRet;
}

Byte AudioPortMeterValue::versionNumber() const { return 0x01; }

}  // namespace GeneSysLib

