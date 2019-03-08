/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "AudioControlDetailTypes.h"
#include "StreamHelpers.h"

namespace GeneSysLib {

namespace AudioControlDetailTypes {

Selector Selector::parse(BytesIter beginIter, BytesIter endIter) {
  Selector selector;
  auto nameLength = nextMidiByte(beginIter, endIter);
  selector.selectorInputName = nextROString(nameLength, beginIter, endIter);
  return selector;
}

////////////////////////////////////////////////////////////////////////////////

Feature Feature::parse(BytesIter beginIter, BytesIter endIter) {
  Feature feature;

  feature.audioChannelType = nextROByte(beginIter, endIter);
  feature.audioChannelNumber = nextROByte(beginIter, endIter);
  feature.existFlags = std::bitset<8>(nextMidiByte(beginIter, endIter));
  feature.editFlags = std::bitset<8>(nextMidiByte(beginIter, endIter));
  feature.minimumVolumeControl = nextROWord3Byte(beginIter, endIter);
  feature.maximumVolumeControl = nextROWord3Byte(beginIter, endIter);
  feature.resolutionVolumeControl = nextROWord3Byte(beginIter, endIter);
  feature.padVolumeControl = nextROWord3Byte(beginIter, endIter);
  feature.minimumTrimControl = nextROWord3Byte(beginIter, endIter);
  feature.maximumTrimControl = nextROWord3Byte(beginIter, endIter);

  auto channelNameLength = nextMidiByte(beginIter, endIter);
  feature.channelName = nextROString(channelNameLength, beginIter, endIter);

  return feature;
}

int8_t Feature::channelType() const {
  return audioChannelType();
}

int8_t Feature::channelNumber() const {
  return audioChannelNumber();
}

bool Feature::stereoLinkControlExists() const {
  return existFlags[stereoLinkControlOffset];
}

bool Feature::stereoLinkControlEditable() const {
  return editFlags[stereoLinkControlOffset];
}

bool Feature::highImpedanceControlExists() const {
  return existFlags[highImpedanceControlOffset];
}

bool Feature::highImpedanceControlEditable() const {
  return editFlags[highImpedanceControlOffset];
}

bool Feature::phantomPowerExists() const {
  return existFlags[phantomPowerControlOffset];
}

bool Feature::phantomPowerEditable() const {
  return editFlags[phantomPowerControlOffset];
}

bool Feature::muteControlExists() const {
  return existFlags[muteControlOffset];
}

bool Feature::muteControlEditable() const {
  return editFlags[muteControlOffset];
}

bool Feature::volumeControlExists() const {
  return existFlags[volumeControlOffset];
}

bool Feature::volumeControlEditable() const {
  return editFlags[volumeControlOffset];
}

////////////////////////////////////////////////////////////////////////////////

ClockSource ClockSource::parse(BytesIter beginIter, BytesIter endIter) {
  ClockSource clockSource;
  auto nameLength = nextMidiByte(beginIter, endIter);
  clockSource.clockSourceName = nextROString(nameLength, beginIter, endIter);
  return clockSource;
}

Bytes generateVisitor::operator()(const Selector &selector) const {
  Bytes result;
  appendMidiByte(result,
                 static_cast<Byte>(selector.selectorInputName().size()));
  appendString(result, selector.selectorInputName());
  return result;
}

Bytes generateVisitor::operator()(const Feature &feature) const {
  Bytes result;
  appendMidiByte(result,
                 static_cast<Byte>(feature.existFlags.to_ulong() & 0x7F));
  appendMidiByte(result,
                 static_cast<Byte>(feature.editFlags.to_ulong() & 0x7F));
  appendMidiWord3Byte(result, feature.minimumVolumeControl());
  appendMidiWord3Byte(result, feature.maximumVolumeControl());
  appendMidiWord3Byte(result, feature.resolutionVolumeControl());
  appendMidiWord3Byte(result, feature.padVolumeControl());
  appendMidiWord3Byte(result, feature.minimumTrimControl());
  appendMidiWord3Byte(result, feature.maximumTrimControl());
  appendMidiByte(result,
                 static_cast<Byte>(feature.channelName().size() & 0x7F));
  appendString(result, feature.channelName());
  return result;
}

Bytes generateVisitor::operator()(const ClockSource &clockSource) const {
  Bytes result;
  appendMidiByte(result,
                 static_cast<Byte>(clockSource.clockSourceName().size()));
  appendString(result, clockSource.clockSourceName());
  return result;
}

}  // namespace AudioControlDetailTypes

}  // namespace GeneSysLib
