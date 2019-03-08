/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "AudioControlTypes.h"
#include "StreamHelpers.h"

namespace GeneSysLib {

namespace AudioControllerType {

Selector Selector::parse(BytesIter beginIter, BytesIter endIter) {
  using namespace std;
  Selector selector;

  selector.currentSelectorInput = nextRWByte(beginIter, endIter);
  selector.numSelectorInputs = nextROByte(beginIter, endIter);
  auto nameLength = nextMidiByte(beginIter, endIter);
  selector.controllerName = nextROString(nameLength, beginIter, endIter);

  return selector;
}

Feature Feature::parse(BytesIter beginIter, BytesIter endIter) {
  Feature feature;
  feature.numFeatureChannels = nextROByte(beginIter, endIter);
  auto nameLength = nextMidiByte(beginIter, endIter);
  feature.controllerName = nextROString(nameLength, beginIter, endIter);
  return feature;
}

ClockSource ClockSource::parse(BytesIter beginIter, BytesIter endIter) {
  ClockSource clockSource;
  clockSource.currentClockSource = nextRWByte(beginIter, endIter);
  clockSource.numClockSourceInputs = nextROByte(beginIter, endIter);
  auto nameLength = nextMidiByte(beginIter, endIter);
  clockSource.controllerName = nextROString(nameLength, beginIter, endIter);
  return clockSource;
}

Bytes generatorVisitor::operator()(const Selector &selector) const {
  Bytes result;
  appendMidiByte(result, selector.currentSelectorInput());
  appendMidiByte(result, selector.numSelectorInputs());
  appendMidiByte(result, selector.controllerName().size());
  appendString(result, selector.controllerName());
  return result;
}

Bytes generatorVisitor::operator()(const Feature &feature) const {
  Bytes result;
  appendMidiByte(result, feature.numFeatureChannels());
  appendMidiByte(result, static_cast<Byte>(feature.controllerName().size()));
  appendString(result, feature.controllerName());
  return result;
}

Bytes generatorVisitor::operator()(const ClockSource &clockSource) const {
  Bytes result;
  appendMidiByte(result, clockSource.currentClockSource());
  appendMidiByte(result, clockSource.numClockSourceInputs());
  appendMidiByte(result,
                 static_cast<Byte>(clockSource.controllerName().size()));
  appendString(result, clockSource.controllerName());
  return result;
}

}  // namespace AudioControllerType

}  // namespace GeneSysLib
