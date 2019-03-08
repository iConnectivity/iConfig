/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef AUDIOCONTROLDETAILTYPES_H
#define AUDIOCONTROLDETAILTYPES_H

#include "property.h"
#include "LibTypes.h"
#include "ControllerType.h"
#include <bitset>
#include <vector>
#include <map>
#ifndef Q_MOC_RUN
#include <boost/variant.hpp>
#endif

namespace GeneSysLib {

namespace AudioControlDetailTypes {

struct Selector {
  static Selector parse(BytesIter beginIter, BytesIter endIter);
  roString selectorInputName;
};  // struct Selector

struct Feature {
  enum FlagOffsets {
    stereoLinkControlOffset = 0x04,
    highImpedanceControlOffset = 0x03,
    phantomPowerControlOffset = 0x02,
    muteControlOffset = 0x01,
    volumeControlOffset = 0x00
  };

  static Feature parse(BytesIter beginIter, BytesIter endIter);

  int8_t channelType() const;
  int8_t channelNumber() const;

  bool stereoLinkControlExists() const;
  bool stereoLinkControlEditable() const;

  bool highImpedanceControlExists() const;
  bool highImpedanceControlEditable() const;

  bool phantomPowerExists() const;
  bool phantomPowerEditable() const;

  bool muteControlExists() const;
  bool muteControlEditable() const;

  bool volumeControlExists() const;
  bool volumeControlEditable() const;

  roWord minimumVolumeControl;
  roWord maximumVolumeControl;
  roWord resolutionVolumeControl;
  roWord padVolumeControl;
  roWord minimumTrimControl;
  roWord maximumTrimControl;

  roString channelName;

  roByte audioChannelType;
  roByte audioChannelNumber;

private:
  friend struct generateVisitor;

  std::bitset<8> existFlags;
  std::bitset<8> editFlags;
};  // struct Feature

struct ClockSource {
  static ClockSource parse(BytesIter beginIter, BytesIter endIter);
  roString clockSourceName;
};  // struct ClockSource

typedef boost::variant<Selector, Feature, ClockSource> Variants;

template <typename T>
T parse(BytesIter beginIter, BytesIter endIter) {
  return T::parse(beginIter, endIter);
}

inline Variants parse(ControllerTypeEnum controllerType, BytesIter beginIter,
                      BytesIter endIter) {
  Variants result;

  switch (controllerType) {
    case ControllerType::Selector:
      result = parse<Selector>(beginIter, endIter);
      break;
    case ControllerType::Feature:
      result = parse<Feature>(beginIter, endIter);
      break;
    case ControllerType::ClockSource:
      result = parse<ClockSource>(beginIter, endIter);
      break;
    default:
      break;
  }

  return result;
}  // parse

struct generateVisitor : public boost::static_visitor<Bytes> {
  Bytes operator()(const Selector &selector) const;
  Bytes operator()(const Feature &feature) const;
  Bytes operator()(const ClockSource &clockSource) const;
};  // struct generateVisitor

inline Bytes generate(Variants variant) {
  return boost::apply_visitor(generateVisitor(), variant);
}

}  // namespace AudioControlDetailTypes

}  // namespace GeneSysLib

#endif  // AUDIOCONTROLDETAILTYPES_H
