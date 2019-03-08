/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "IAudioPatchbaySource.h"

#ifndef Q_MOC_RUN
#include <boost/tuple/tuple.hpp>
#endif

using namespace std;

int IAudioPatchbaySource::numInputs() const {
  int numInput = 0;
  for (int section = 1; section <= numSections(); ++section) {
    numInput += numInputsPerSection(section);
  }
  return numInput;
}

int IAudioPatchbaySource::numOutputs() const {
  int numOutput = 0;
  for (int section = 1; section <= numSections(); ++section) {
    numOutput += numOutputsPerSection(section);
  }
  return numOutput;
}

int IAudioPatchbaySource::inputIndexToTotal(device_port_t targetIndex) const {
  int index = 0;
  int targetSection, channel;
  boost::tie(targetSection, channel) = targetIndex;

  if (targetSection <= numSections()) {
    for (int section = 1; section < targetSection; ++section) {
      index += numInputsPerSection(section);
    }
    index += channel;
  }
  return index;
}

device_port_t IAudioPatchbaySource::inputTotalToIndex(int totalInput) const {
  int section, channel;
  section = channel = 1;

  while ((totalInput - numInputsPerSection(section)) >= 1) {
    totalInput -= numInputsPerSection(section);
    ++section;
  }
  channel = totalInput;

  return std::make_pair(section, channel);
}

int IAudioPatchbaySource::outputIndexToTotal(device_port_t targetIndex) const {
  int index = 0;
  int targetSection, channel;
  boost::tie(targetSection, channel) = targetIndex;
  if (targetSection <= numSections()) {
    for (int section = 1; section < targetSection; ++section) {
      index += numOutputsPerSection(section);
    }
    index += channel;
  }
  return index;
}

device_port_t IAudioPatchbaySource::outputTotalToIndex(int totalOutput) const {
  int section, channel;
  section = channel = 1;

  while ((totalOutput - numOutputsPerSection(section)) >= 1) {
    totalOutput -= numOutputsPerSection(section);
    ++section;
  }
  channel = totalOutput;

  return std::make_pair(section, channel);
}
