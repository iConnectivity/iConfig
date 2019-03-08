/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "CommandDefines.h"
#include "AudioPatchbaySourceV1.h"
#include "AudioInfo.h"
#include "AudioPortInfo.h"
#include "AudioPortCfgInfo.h"
#include "AudioPortPatchbay.h"
#include <algorithm>
#include <MyAlgorithms.h>
#include <QDebug>

using namespace GeneSysLib;
using namespace std;
using namespace MyAlgorithms;

CommandQList AudioPatchbaySourceV1::Query() {
  QList<CmdEnum> query;
  query << Command::RetAudioInfo << Command::RetAudioCfgInfo
        << Command::RetAudioPortInfo << Command::RetAudioPortCfgInfo
        << Command::RetAudioPortPatchbay;
  return query;
}

AudioPatchbaySourceV1::AudioPatchbaySourceV1(DeviceInfoPtr _deviceInfo)
    : device(_deviceInfo) {}

void AudioPatchbaySourceV1::init(
    const boost::function<void(void)> &refreshCallback) {
  if (refreshCallback) {
    refreshCallback();
  }

  const auto &audioInfo = device->get<AudioInfo>();
  // because V1 was depreicated when only USB Device Jacks were supported I will
  // ignore all the other port types
  for (int i = 0; i < audioInfo.numberOfUSBDeviceJacks(); i++) {
    isCollapsedInputVec.push_back(false);
    isCollapsedOutputVec.push_back(false);
  }
}

int AudioPatchbaySourceV1::numSections() const {
  const auto &audioInfo = device->get<AudioInfo>();
  // because V1 was depreicated when only USB Device Jacks were supported I will
  // ignore all the other port types

  return audioInfo.numberOfUSBDeviceJacks();
}

int AudioPatchbaySourceV1::numInputsPerSection(int section) const {
  const auto &portCfg = device->get<AudioPortCfgInfo>(section);
  if (!isCollapsedOutput(section))
    return portCfg.numInputChannels();
  else
    return 1;
}

int AudioPatchbaySourceV1::numOutputsPerSection(int section) const {
  const auto &portCfg = device->get<AudioPortCfgInfo>(section);
  if (!isCollapsedInput(section))
    return portCfg.numOutputChannels();
  else
    return 1;
}

QString AudioPatchbaySourceV1::nameForSection(int section) const {
  const auto &portInfo = device->get<AudioPortInfo>(section);
  return QString::fromStdString(portInfo.portName());
}

bool AudioPatchbaySourceV1::isPatched(device_port_t out,
                                      device_port_t in) const {
  bool result = false;
  int outPort, outCh, inPort, inCh;
  boost::tie(outPort, outCh) = out;
  boost::tie(inPort, inCh) = in;

  if ((isCollapsedInput(outPort)) || (isCollapsedOutput(inPort))) {
    return true;
  }

  if (device->contains(AudioPortPatchbay::queryKey(inPort))) {
    try {
      const auto &patchbay = device->get<AudioPortPatchbay>(inPort);
      const auto &configBlock = patchbay.findInputBlock(inCh);

      result = (configBlock.portIDOfOutput() == (Word) outPort) &&
               (configBlock.outputChannelNumber() == (Byte) outCh);
    }
    catch (blockNotFoundException) {
      result = false;
    }
  }

  return result;
}

void AudioPatchbaySourceV1::setPatch(device_port_t out, device_port_t in, device_port_t toRemove) {
  int outPort, outCh, inPort, inCh;
  boost::tie(outPort, outCh) = out;
  boost::tie(inPort, inCh) = in;

  if ((outPort == 0 && isCollapsedInput(toRemove.first)) || (isCollapsedOutput(inPort)) || (outPort != 0 && isCollapsedInput(outPort))) {
    return;
  }

  if (device->contains(AudioPortPatchbay::queryKey(inPort))) {
    try {
      auto &patchbay = device->get<AudioPortPatchbay>(inPort);
      auto &configBlock = patchbay.findInputBlock(inCh);

      configBlock.portIDOfOutput((Word) outPort);
      configBlock.outputChannelNumber((Byte) outCh);

      device->send<SetAudioPortPatchbayCommand>(patchbay);
    }
    catch (blockNotFoundException) {
    }
  }
}

bool AudioPatchbaySourceV1::isMixerToo() const {
  return false;
}

void AudioPatchbaySourceV1::toggleCollapseInput(int section)
{
  isCollapsedInputVec.at(section - 1) = !isCollapsedInputVec.at(section - 1);
}

bool AudioPatchbaySourceV1::isCollapsedInput(int section) const
{
  if (section == 0)
    return false;
  return isCollapsedInputVec.at(section - 1);
}

void AudioPatchbaySourceV1::toggleCollapseOutput(int section)
{
  isCollapsedOutputVec.at(section - 1) = !isCollapsedOutputVec.at(section - 1);
}

bool AudioPatchbaySourceV1::isCollapsedOutput(int section) const
{
  if (section == 0)
    return false;
  return isCollapsedOutputVec.at(section - 1);
}

void AudioPatchbaySourceV1::for_each_mixer(
  const boost::function<void(const device_pair_t &)> &func) const {
}

void AudioPatchbaySourceV1::for_each(
    const boost::function<void(const device_pair_t &)> &func) const {
  //printf("av1 for_each\n");
  device->for_each<AudioPortPatchbay>(
      [ = ](const AudioPortPatchbay & patchBay) {
    const auto &flatList = patchBay.flatList();
    for (const auto &f : flatList) {
      int p1 = f.outPortID;
      int c1 = f.outChannelNumber;
      int p2 = f.inPortID;
      int c2 = f.inChannelNumber;

      if (isCollapsedInput(p1))
        c1 = 1;
      if (isCollapsedOutput(p2))
        c2 = 1;

      func(make_pair(make_pair((int) p1, (int) c1),
                     make_pair((int) p2, (int) c2)));
    }
  });
}
