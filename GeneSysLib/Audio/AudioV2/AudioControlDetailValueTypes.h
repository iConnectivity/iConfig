/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __AUDIOCONTROLDETAILVALUETYPES_H__
#define __AUDIOCONTROLDETAILVALUETYPES_H__

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

namespace AudioControlDetailValueTypes {

struct Selector {
  static Selector parse(BytesIter beginIter, BytesIter endIter);
};  // struct Selector

struct Feature {

  enum Flags {
    stereoLinkOffset = 0x04,
    highImpedanceOffset = 0x03,
    phantomPowerOffset = 0x02,
    muteControlOffset = 0x01,
    volumeControlOffset = 0x00
  };

  static Feature parse(BytesIter beginIter, BytesIter endIter);
  rwWord volume;
  rwWord trim;
  rwByte mute;
  rwByte phantomPower;
  rwByte highImpedance;
  rwByte stereoLink;

  bool isStereoLinkIncluded() const;
  bool isHighImpedanceIncluded() const;
  bool isPhantomPowerIncluded() const;
  bool isMuteIncluded() const;
  bool isVolumeIncluded() const;

  std::bitset<8> valueFlags;

};  // struct Feature

struct ClockSource {
  static ClockSource parse(BytesIter beginIter, BytesIter endIter);
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

}  // namespace AudioControlDetailValueTypes

}  // namespace GeneSysLib

#endif  // __AUDIOCONTROLDETAILVALUETYPES_H__
