/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __AUDIOCONTROLTYPES_H__
#define __AUDIOCONTROLTYPES_H__

#include "property.h"
#include "LibTypes.h"
#include "ControllerType.h"
#include <vector>
#ifndef Q_MOC_RUN
#include <boost/variant.hpp>
#endif

namespace GeneSysLib {

namespace AudioControllerType {

struct Selector {
  static Selector parse(BytesIter beginIter, BytesIter endIter);
  rwByte currentSelectorInput;
  roByte numSelectorInputs;
  roString controllerName;
};

struct Feature {
  static Feature parse(BytesIter beginIter, BytesIter endIter);
  roByte numFeatureChannels;
  roString controllerName;
};

struct ClockSource {
  static ClockSource parse(BytesIter beginIter, BytesIter endIter);
  rwByte currentClockSource;
  roByte numClockSourceInputs;
  roString controllerName;
};

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
}

struct generatorVisitor : public boost::static_visitor<Bytes> {
  Bytes operator()(const Selector &) const;
  Bytes operator()(const Feature &) const;
  Bytes operator()(const ClockSource &) const;
};

inline Bytes generate(Variants variant) {
  return boost::apply_visitor(generatorVisitor(), variant);
}

}  // namespace AudioControllerType

}  // namespace GeneSysLib

#endif  // __AUDIOCONTROLTYPES_H__
