/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "AudioControlDetailValueTypes.h"
#include "Generator.h"
#include <iterator>
#ifndef Q_MOC_RUN
#include <boost/range.hpp>
#include <boost/range/algorithm_ext.hpp>
#endif
#include <limits>
#include <algorithm>
#include <bitset>

using namespace std;

namespace GeneSysLib {

namespace AudioControlDetailValueTypes {

// Selector
Selector Selector::parse(BytesIter, BytesIter) { return Selector(); }

// Feature
Feature Feature::parse(BytesIter beginIter, BytesIter endIter) {
  Feature feature;

  feature.valueFlags = std::bitset<8>(nextMidiByte(beginIter, endIter));

  if (feature.isVolumeIncluded()) {
    feature.volume = nextRWWord3Byte(beginIter, endIter);
    feature.trim = nextRWWord3Byte(beginIter, endIter);
  }
  if (feature.isMuteIncluded()) {
    feature.mute = nextRWByte(beginIter, endIter);
  }
  if (feature.isPhantomPowerIncluded()) {
    feature.phantomPower = nextRWByte(beginIter, endIter);
  }
  if (feature.isHighImpedanceIncluded()) {
    feature.highImpedance = nextRWByte(beginIter, endIter);
  }
  if (feature.isStereoLinkIncluded()) {
    feature.stereoLink = nextRWByte(beginIter, endIter);
  }

  return feature;
}

bool Feature::isStereoLinkIncluded() const {
  return valueFlags[stereoLinkOffset];
}

bool Feature::isHighImpedanceIncluded() const {
  return valueFlags[highImpedanceOffset];
}

bool Feature::isPhantomPowerIncluded() const {
  return valueFlags[phantomPowerOffset];
}

bool Feature::isMuteIncluded() const {
 return valueFlags[muteControlOffset];
}

bool Feature::isVolumeIncluded() const {
  return valueFlags[volumeControlOffset];
}

// ClockSource
ClockSource ClockSource::parse(BytesIter, BytesIter) { return ClockSource(); }

Bytes generateVisitor::operator()(const Selector &) const { return Bytes(); }

Bytes generateVisitor::operator()(const Feature &feature) const {
  Bytes result;

  appendMidiByte(result, feature.valueFlags.to_ulong() & 0x7F);

  if (feature.isVolumeIncluded()) {
    appendMidiWord3Byte(result, feature.volume());
    appendMidiWord3Byte(result, feature.trim());
  }

  if (feature.isMuteIncluded()) {
    appendMidiByte(result, feature.mute());
  }

  if (feature.isPhantomPowerIncluded()) {
    appendMidiByte(result, feature.phantomPower());
  }

  if (feature.isHighImpedanceIncluded()) {
    appendMidiByte(result, feature.highImpedance());
  }

  if (feature.isStereoLinkIncluded()) {
    appendMidiByte(result, feature.stereoLink());
  }

  return result;
}

Bytes generateVisitor::operator()(const ClockSource &) const { return Bytes(); }

}  // namespace AudioControlDetailValueTypes

}  // namespace GeneSysLib
