/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "AudioGlobalParm.h"
#include "MixerInterface.h"
#include "MixerPortParm.h"
#include "MixerParm.h"
#include "MixerInputParm.h"
#include "MixerOutputParm.h"

#include "property.h"

using namespace std;
using namespace GeneSysLib;

MixerInterface::MixerInterface(DeviceInfoPtr device) {
  this->device = device;
}

MixerInterface::~MixerInterface() {}

int8_t MixerInterface::activeMixerConfigurationNumber() const
{
  const auto &audioGlobalParm = device->get<AudioGlobalParm>();

  const auto& mixerParm = device->get<MixerParm>(audioGlobalParm.currentActiveConfig());
  return mixerParm.activeMixerConfigurationBlock();
}


void MixerInterface::activeMixerConfigurationNumber(const int8_t value)
{
  const auto &audioGlobalParm = device->get<AudioGlobalParm>();

  auto& mixerParm = device->get<MixerParm>(audioGlobalParm.currentActiveConfig());
  mixerParm.activeMixerConfigurationBlock(value);
  device->send<SetMixerParmCommand>(mixerParm);
}


int8_t MixerInterface::mixerConfigurationCount() const
{
  const auto &audioGlobalParm = device->get<AudioGlobalParm>();

  const auto& mixerParm = device->get<MixerParm>(audioGlobalParm.currentActiveConfig());
  return mixerParm.mixerBlockCount();
}


int8_t MixerInterface::maximumInputs(Byte mixerConfigurationNumber) const
{
  const auto &audioGlobalParm = device->get<AudioGlobalParm>();

  const auto& mixerParm = device->get<MixerParm>(audioGlobalParm.currentActiveConfig());
  return mixerParm.mixerBlocks.at(mixerConfigurationNumber - 1).maximumInputs();
}


int8_t MixerInterface::maximumOutputs(Byte mixerConfigurationNumber) const
{
  const auto &audioGlobalParm = device->get<AudioGlobalParm>();

  const auto& mixerParm = device->get<MixerParm>(audioGlobalParm.currentActiveConfig());
  return mixerParm.mixerBlocks.at(mixerConfigurationNumber - 1).maximumOutputs();
}

int8_t MixerInterface::numberInputs(Word audioPortID) const
{
  const auto& mixerPortParm = device->get<MixerPortParm>();
  return mixerPortParm.audioPortMixerBlocks.at(audioPortID - 1).numInputs();
}

void MixerInterface::numberInputs(Word audioPortID, int8_t value)
{
  auto& mixerPortParm = device->get<MixerPortParm>();
  mixerPortParm.audioPortMixerBlocks.at(audioPortID - 1).numInputs(value);
  device->send<SetMixerPortParmCommand>(mixerPortParm);
}

int8_t MixerInterface::numberOutputs(Word audioPortID) const
{
  const auto& mixerPortParm = device->get<MixerPortParm>();
  return mixerPortParm.audioPortMixerBlocks.at(audioPortID - 1).numOutputs();
}

void MixerInterface::numberOutputs(Word audioPortID, int8_t value)
{
  auto& mixerPortParm = device->get<MixerPortParm>();
  mixerPortParm.audioPortMixerBlocks.at(audioPortID - 1).numOutputs(value);
  device->send<SetMixerPortParmCommand>(mixerPortParm);
}

int16_t MixerInterface::audioPortIDForInput(Word audioPortID, Byte mixerInputNumber) const
{
  const MixerInputParm& mixerInputPortParm = device->get<MixerInputParm>(audioPortID, mixerInputNumber);
  return mixerInputPortParm.audioSourceAudioPortID();
}

void MixerInterface::audioPortIDForInput(Word audioPortID, Byte mixerInputNumber, int16_t value)
{
  MixerInputParm& mixerInputPortParm = device->get<MixerInputParm>(audioPortID, mixerInputNumber);
  mixerInputPortParm.audioSourceAudioPortID(value);

  device->send<SetMixerInputParmCommand>(mixerInputPortParm);
}

int8_t MixerInterface::channelIDForInput(Word audioPortID, Byte mixerInputNumber) const
{
  const MixerInputParm& mixerInputPortParm = device->get<MixerInputParm>(audioPortID, mixerInputNumber);
  return mixerInputPortParm.audioSourceChannelID();
}

void MixerInterface::channelIDForInput(Word audioPortID, Byte mixerInputNumber, int16_t value)
{
  MixerInputParm& mixerInputPortParm = device->get<MixerInputParm>(audioPortID, mixerInputNumber);
  mixerInputPortParm.audioSourceChannelID(value);

  device->send<SetMixerInputParmCommand>(mixerInputPortParm);
}

int8_t MixerInterface::numberOfMixerOutputAssignments(Word audioPortID, Byte mixerOutputNumber) const
{
  const MixerOutputParm& mixerOutputPortParm = device->get<MixerOutputParm>(audioPortID, mixerOutputNumber);
  return mixerOutputPortParm.numberOfMixerOutputAssignments();
}

std::vector<int8_t> MixerInterface::channelIDsForOutput(Word audioPortID, Byte mixerOutputNumber) const
{
  std::vector<int8_t> result;
  const MixerOutputParm& mixerOutputPortParm = device->get<MixerOutputParm>(audioPortID, mixerOutputNumber);
  for (int i = 0; i < mixerOutputPortParm.mixerOutputAssignments.size(); i++) {
    result.push_back(mixerOutputPortParm.mixerOutputAssignments.at(i)());
  }
  return result;
}

void MixerInterface::channelIDsForOutput(Word audioPortID, Byte mixerOutputNumber, std::vector<int8_t> values)
{
  MixerOutputParm& mixerOutputPortParm = device->get<MixerOutputParm>(audioPortID, mixerOutputNumber);
  mixerOutputPortParm.mixerOutputAssignments.clear();
  for (int i = 0; i < values.size(); i++) {
    mixerOutputPortParm.mixerOutputAssignments.push_back(rwByte(values.at(i)));
  }

  mixerOutputPortParm.numberOfMixerOutputAssignments(values.size());

  device->send<SetMixerOutputParmCommand>(mixerOutputPortParm);
}

int8_t MixerInterface::maximumMixNameLength(Word audioPortID, Byte mixerOutputNumber) const
{
  const MixerOutputParm& mixerOutputPortParm = device->get<MixerOutputParm>(audioPortID, mixerOutputNumber);
  return mixerOutputPortParm.maximumMixNameLength();
}

int8_t MixerInterface::mixNameLength(Word audioPortID, Byte mixerOutputNumber) const
{
  const MixerOutputParm& mixerOutputPortParm = device->get<MixerOutputParm>(audioPortID, mixerOutputNumber);
  return mixerOutputPortParm.mixNameLength();
}

std::string MixerInterface::mixName(Word audioPortID, Byte mixerOutputNumber) const
{
  std::string result;
  const MixerOutputParm& mixerOutputPortParm = device->get<MixerOutputParm>(audioPortID, mixerOutputNumber);
  for (int i = 0; i < mixerOutputPortParm.mixNameLength(); i++) {
    char data = mixerOutputPortParm.mixName.at(i)();
    result.append(&data, 1);
  }
  return result;
}

void MixerInterface::mixName(Word audioPortID, Byte mixerOutputNumber, std::string value)
{
  const char * c = value.c_str();
  MixerOutputParm& mixerOutputPortParm = device->get<MixerOutputParm>(audioPortID, mixerOutputNumber);
  mixerOutputPortParm.mixName.clear();
  for (int i = 0; (i < value.size() && (i < mixerOutputPortParm.maximumMixNameLength())); i++) {
    mixerOutputPortParm.mixName.push_back(rwByte(c[i]));
    mixerOutputPortParm.mixNameLength(i + 1);
  }

  device->send<SetMixerOutputParmCommand>(mixerOutputPortParm);
}
