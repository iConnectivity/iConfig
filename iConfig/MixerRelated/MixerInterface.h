/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef MIXERINTERFACE_H
#define MIXERINTERFACE_H

#include "DeviceInfo.h"
#include <vector>

struct MixerInterface {
  // Constructor/destructor
  MixerInterface(DeviceInfoPtr device);
  virtual ~MixerInterface();

  int8_t activeMixerConfigurationNumber() const;
  void activeMixerConfigurationNumber(int8_t value);

  int8_t mixerConfigurationCount() const;
  int8_t maximumInputs(Byte mixerConfigurationNumber) const;
  int8_t maximumOutputs(Byte mixerConfigurationNumber) const;

  int8_t numberInputs(Word audioPortID) const;
  void numberInputs(Word audioPortID, int8_t value);
  int8_t numberOutputs(Word audioPortID) const;
  void numberOutputs(Word audioPortID, int8_t value);

  int16_t audioPortIDForInput(Word audioPortID, Byte mixerInputNumber) const;
  void audioPortIDForInput(Word audioPortID, Byte mixerInputNumber, int16_t value);
  int8_t channelIDForInput(Word audioPortID, Byte mixerInputNumber) const;
  void channelIDForInput(Word audioPortID, Byte mixerInputNumber, int16_t value);

  int8_t numberOfMixerOutputAssignments(Word audioPortID, Byte mixerOutputNumber) const;
  std::vector<int8_t> channelIDsForOutput(Word audioPortID, Byte mixerOutputNumber) const;
  void channelIDsForOutput(Word audioPortID, Byte mixerOutputNumber, std::vector<int8_t> values);

  int8_t maximumMixNameLength(Word audioPortID, Byte mixerOutputNumber) const;
  int8_t mixNameLength(Word audioPortID, Byte mixerOutputNumber) const;
  void mixNameLength(Word audioPortID, Byte mixerOutputNumber, int8_t value);
  std::string mixName(Word audioPortID, Byte mixerOutputNumber) const;
  void mixName(Word audioPortID, Byte mixerOutputNumber, std::string value);

private:
  DeviceInfoPtr device;
};

#endif // MIXERINTERFACE_H
