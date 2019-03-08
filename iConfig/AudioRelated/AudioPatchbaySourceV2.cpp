/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "CommandDefines.h"
#include "AudioPatchbaySourceV2.h"
#include "AudioGlobalParm.h"
#include "AudioPortParm.h"
#include "AudioPatchbayParm.h"
#include "MixerPortParm.h"
#include "CommandList.h"
#include <algorithm>
#include <MyAlgorithms.h>
#include <QDebug>

using namespace GeneSysLib;
using namespace std;
using namespace MyAlgorithms;

CommandQList AudioPatchbaySourceV2::Query() {
  CommandQList query;

  query << Command::RetAudioGlobalParm;
  query << Command::RetAudioPortParm;
  query << Command::RetAudioPatchbayParm;

  query << Command::RetMixerParm;
  query << Command::RetMixerPortParm;
  query << Command::RetMixerInputParm;
  query << Command::RetMixerOutputParm;


  return query;
}

AudioPatchbaySourceV2::AudioPatchbaySourceV2(DeviceInfoPtr _deviceInfo)
    : device(_deviceInfo) {
  isMixerTooBool = false;
  mixerInterface = 0;
}

void AudioPatchbaySourceV2::init(
    const boost::function<void(void)> &refreshCallback) {

  if (device->get<CommandList>().contains(Command::GetMixerParm)) {
    isMixerTooBool = true;
    mixerInterface = new MixerInterface(device);
  }
  if (refreshCallback) {
    refreshCallback();
  }

  const auto &globalParm = device->get<AudioGlobalParm>();
  for (int i = 1; i <= globalParm.numAudioPorts(); i++) {
    isCollapsedInputVec.push_back(false);
    isCollapsedOutputVec.push_back(false);
    if (isMixerTooBool) {
      isCollapsedInputVec.push_back(false);
      isCollapsedOutputVec.push_back(false);
    }
  }
}

int AudioPatchbaySourceV2::numSections() const {
  const auto &globalParm = device->get<AudioGlobalParm>();
  int toReturn = globalParm.numAudioPorts();
  if (isMixerTooBool)
    toReturn *= 2;
  return toReturn;
}

int AudioPatchbaySourceV2::numInputsPerSection(int section) const {
  int toReturn;
  if (!isMixerTooBool) {
    const auto &portParm = device->get<AudioPortParm>(section);
    toReturn = portParm.numInputChannels();
  }
  else {
    int realSection = floor((section+1)/2);
    if (section % 2) { // odd = true port, not mixer
      const auto &portParm = device->get<AudioPortParm>(realSection);
      toReturn = portParm.numInputChannels();
    }
    else { // even == mixer port
      MixerPortParm &mixerPortParm = device->get<MixerPortParm>();
      toReturn = mixerInterface->numberInputs(realSection);
      if (!mixerInterface->numberOutputs(realSection)) {
        toReturn = 0;
      }
    }
  }
  if (toReturn >= 1) {
    if (isCollapsedOutput(section))
      toReturn = 1;
  }
  return toReturn;
}

int AudioPatchbaySourceV2::numOutputsPerSection(int section) const {
  int toReturn;
  if (!isMixerTooBool) {
    const auto &portParm = device->get<AudioPortParm>(section);
    toReturn = portParm.numOutputChannels();
  }
  else {
    int realSection = floor((section+1)/2);
    if (section % 2) { // odd = true port, not mixer
      const auto &portParm = device->get<AudioPortParm>(realSection);
      toReturn = portParm.numOutputChannels();
    }
    else { // even == mixer port
      MixerPortParm &mixerPortParm = device->get<MixerPortParm>();
      toReturn = mixerInterface->numberOutputs(realSection);
    }
  }
  if (toReturn >= 1) {
    if (isCollapsedInput(section))
      toReturn = 1;
  }
  return toReturn;
}

QString AudioPatchbaySourceV2::nameForSection(int section) const {
  QString toReturn;
  if (!isMixerTooBool) {
    const auto &portParm = device->get<AudioPortParm>(section);
    toReturn = QString::fromStdString(portParm.portName());
  }
  else {
    int realSection = floor((section+1)/2);
    if (section % 2) { // odd = true port, not mixer
      const auto &portParm = device->get<AudioPortParm>(realSection);
      toReturn = QString::fromStdString(portParm.portName());
    }
    else { // even == mixer port
      const auto &portParm = device->get<AudioPortParm>(realSection);
      toReturn = QString::fromStdString(portParm.portName()) + " Mixer";
    }
  }
  return toReturn;
}

bool AudioPatchbaySourceV2::isPatched(device_port_t out,
                                      device_port_t in) const {
  bool result = false;
  int outPort, outCh, inPort, inCh, origOutPort, origInPort;
  boost::tie(outPort, outCh) = out;
  boost::tie(inPort, inCh) = in;

  if ((isCollapsedInput(outPort)) || (isCollapsedOutput(inPort))) {
    return true;
  }
  origOutPort = outPort;
  origInPort = inPort;

  if (isMixerTooBool) {
    outPort = floor((outPort + 1.0)/2.0);
    inPort = floor((inPort + 1.0)/2.0);
  }
  if (((origOutPort % 2) && (origInPort % 2)) || (!isMixerTooBool)){ // doesn't involve mixer
    if (device->contains(AudioPatchbayParm::queryKey(inPort))) {
      try {
        const auto &patchbay = device->get<AudioPatchbayParm>(inPort);
        const auto &configBlock = patchbay.findInputBlock(inCh);

        result = (configBlock.portIDOfOutput() == (Word) outPort) &&
                 (configBlock.outputChannelNumber() == (Byte) outCh);
      }
      catch (blockNotFoundException) {
        result = false;
      }
    }
    /*if (isMixerTooBool) {
      for (int x = 1; x <= mixerInterface->numberOutputs(inPort); x++) {
        vector<int8_t> outChs = mixerInterface->channelIDsForOutput(inPort, x);
        std::vector<int8_t>::iterator position = std::find(outChs.begin(), outChs.end(), inCh);
        if (position != outChs.end())
          result = true;
      }
    }*/
  }
  else {
    //printf("isPatched() origOutPort: %d, origInPort: %d\n", origOutPort, origInPort);

    if (!((origOutPort % 2) || (origInPort % 2))) { // both are mixers, no mixer to mixer!!
      result = true; // make an X

    }
    else if (origInPort % 2) { // outPort (top) is even -> inPort is mixer
      // mixer out (inPort) -> some output (outPort)
      if (inPort == outPort) {
        vector<int8_t> outChs = mixerInterface->channelIDsForOutput(outPort, outCh);
        std::vector<int8_t>::iterator position = std::find(outChs.begin(), outChs.end(), inCh);
        if (position != outChs.end())
          result = true;
      }
      else {
        result = true; // make an X
      }
      // here!
    }
    else {
      // some input (inPort) -> mixer in (outPort)
      if ((mixerInterface->audioPortIDForInput(inPort, inCh) == outPort) &&
          (mixerInterface->channelIDForInput(inPort, inCh) == outCh)) {
        result = true;
      }
    }
  }

  return result;
}

void AudioPatchbaySourceV2::setPatch(device_port_t out, device_port_t in, device_port_t toRemove) {
  int outPort, outCh, inPort, inCh, origOutPort, origInPort;
  boost::tie(outPort, outCh) = out;
  boost::tie(inPort, inCh) = in;

  //printf("patching in: %d, %d to out: %d, %d\n", inPort, inCh, outPort, outCh);

  // dest is "in"
  // src is "out" :S

  if ((outPort == 0 && isCollapsedInput(toRemove.first)) || (isCollapsedOutput(inPort)) || (outPort != 0 && isCollapsedInput(outPort))) {
    return;
  }

  origOutPort = outPort;
  origInPort = inPort;

  if (isMixerTooBool) {
    outPort = floor((outPort + 1.0)/2.0);
    inPort = floor((inPort + 1.0)/2.0);
  }

  if ((((origOutPort % 2) || (origOutPort == 0 && toRemove.first % 2)) && ((origInPort % 2))) || (!isMixerTooBool)) { // doesn't involve mixer
    if (device->contains(AudioPatchbayParm::queryKey(inPort))) {
      try {
        auto &patchbay = device->get<AudioPatchbayParm>(inPort);
        auto &configBlock = patchbay.findInputBlock(inCh);

        configBlock.portIDOfOutput((Word) outPort);
        configBlock.outputChannelNumber((Byte) outCh);

        device->send<SetAudioPatchbayParmCommand>(patchbay);
      }
      catch (blockNotFoundException) {
      }

      int numOutputs = mixerInterface->numberOutputs(inPort);
      for (int j = 1; j <= numOutputs; j++) {
        std::vector<int8_t> outputs = mixerInterface->channelIDsForOutput(inPort,j);
        for (int i = 0; i < outputs.size(); i++) {
          if (outputs.at(i) == inCh) {
            outputs.erase(outputs.begin() + i);
            break;
          }
        }
        mixerInterface->channelIDsForOutput(inPort, j, outputs);
      }
    }
  }
  else {
    //printf("origOutPort: %d, origInPort: %d\n", origOutPort, origInPort);
    bool remove = false;
    if (origOutPort == 0) {
      remove = true;
      origOutPort = toRemove.first;
      outPort = floor((origOutPort + 1.0)/2.0);
      outCh = toRemove.second;

    }
    if (!((origOutPort % 2) || (origInPort % 2))) { // both are mixers, no mixer to mixer!!
      return;
    }
    else if (origInPort % 2) { // outPort (top) is even -> outPort is mixer
      if (inPort == outPort) {
        vector<int8_t> outChs = mixerInterface->channelIDsForOutput(outPort, outCh);
        if (remove) {
          outChs.erase(std::remove(outChs.begin(), outChs.end(), inCh), outChs.end());
        }
        else {
          outChs.push_back(inCh);

          // now erase it from other mixes.
          int numOutputs = mixerInterface->numberOutputs(outPort);
          for (int j = 1; j <= numOutputs; j++) {
            if (j != outCh) {
              std::vector<int8_t> outputs = mixerInterface->channelIDsForOutput(outPort,j);
              for (int i = 0; i < outputs.size(); i++) {
                if (outputs.at(i) == inCh) {
                  outputs.erase(outputs.begin() + i);
                  break;
                }
              }
              mixerInterface->channelIDsForOutput(outPort, j, outputs);
            }
          }
        }
        mixerInterface->channelIDsForOutput(outPort, outCh, outChs);
      }

      auto &patchbay = device->get<AudioPatchbayParm>(inPort);
      auto &configBlock = patchbay.findInputBlock(inCh);

      configBlock.portIDOfOutput((Word) 0);
      configBlock.outputChannelNumber((Byte) 0);

      device->send<SetAudioPatchbayParmCommand>(patchbay);
    }
    else {
      // inPort (side) is even -> inPort is mixer
      if (remove) {
        mixerInterface->audioPortIDForInput(inPort, inCh, 0);
        mixerInterface->channelIDForInput(inPort, inCh, 0);
      }
      else {
        mixerInterface->audioPortIDForInput(inPort, inCh, outPort);
        mixerInterface->channelIDForInput(inPort, inCh, outCh);
      }
    }
  }
}

void AudioPatchbaySourceV2::toggleCollapseInput(int section)
{
  isCollapsedInputVec.at(section - 1) = !isCollapsedInputVec.at(section - 1);
}

bool AudioPatchbaySourceV2::isCollapsedInput(int section) const
{
  //printf("isCollapsedInput: %d\n", section);
  if (section == 0)
    return false;
  return isCollapsedInputVec.at(section - 1);
}

void AudioPatchbaySourceV2::toggleCollapseOutput(int section)
{
  isCollapsedOutputVec.at(section - 1) = !isCollapsedOutputVec.at(section - 1);
}

bool AudioPatchbaySourceV2::isCollapsedOutput(int section) const
{
  //printf("isCollapsedOutput: %d\n", section);
  if (section == 0)
    return false;
  return isCollapsedOutputVec.at(section - 1);
}

bool AudioPatchbaySourceV2::isMixerToo() const {
  return isMixerTooBool;
}

void AudioPatchbaySourceV2::for_each_mixer (
    const boost::function<void(const device_pair_t &)> &func) const {
  if (isMixerTooBool) {
    device->for_each<AudioPatchbayParm>(
          [ = ](const AudioPatchbayParm & patchBay) {
      for (int i = 2; i <= numSections(); i = i+2) {
        for (int j = 1; j <= mixerInterface->numberInputs(i / 2); j++) {
          if (mixerInterface->channelIDForInput(i/2, j) &&
              mixerInterface->audioPortIDForInput(i/2,j)) {
            int p1 = mixerInterface->audioPortIDForInput(i/2,j) * 2 - 1;
            int c1 = mixerInterface->channelIDForInput(i/2, j);
            int p2 = i;
            int c2 = j;

            if (isCollapsedInput(p1))
              c1 = 1;
            if (isCollapsedOutput(p2))
              c2 = 1;

            func(make_pair(make_pair((int) p1,
                                     (int) c1),
                           make_pair((int) p2,
                                     (int) c2)));
          }
        }
        for (int k = 1; k <= mixerInterface->numberOutputs(i / 2); k++) {
          std::vector<int8_t> outChs = mixerInterface->channelIDsForOutput(i / 2,k);

          if (outChs.size() != 0) {
            for (int m = 0; m < outChs.size(); m++) {
              int p1 = i;
              int c1 = k;
              int p2 = i - 1;
              int c2 = outChs.at(m);

              if (isCollapsedInput(p1))
                c1 = 1;
              if (isCollapsedOutput(p2))
                c2 = 1;

              func(make_pair(make_pair((int) p1,
                                       (int) c1),
                             make_pair((int) p2,
                                       (int) c2)));
            }
          }
        }

      }
    });
  }
}

void AudioPatchbaySourceV2::for_each(
    const boost::function<void(const device_pair_t &)> &func) const {
  //printf("start of for_each\n");
  device->for_each<AudioPatchbayParm>(
      [ = ](const AudioPatchbayParm & patchBay) {
    const auto &flatList = patchBay.flatList();
    if (!isMixerTooBool) {
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
    }
    else {
      for (const auto &f : flatList) {
        int p1 = f.outPortID * 2 - 1;
        int c1 = f.outChannelNumber;
        int p2 = f.inPortID * 2 - 1;
        int c2 = f.inChannelNumber;

        if (p1 < 0)
          p1 = 0;
        if (p2 < 0)
          p2 = 0;

        if (isCollapsedInput(p1))
          c1 = 1;
        if (isCollapsedOutput(p2))
          c2 = 1;

        func(make_pair(make_pair((int) p1, (int) c1),
                       make_pair((int) p2, (int) c2)));
      }
    }
  });
  //printf("end of for_each\n");
}
