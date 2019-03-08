/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "AudioControlWidget.h"
#include "AudioGlobalParm.h"
#include "AudioPortParm.h"
#include "AudioDeviceParm.h"
#include "AudioControlParm.h"
#include "AudioPortMeterValue.h"
#include "IRefreshWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

using namespace GeneSysLib;

const QString AudioControlWidget::DefaultWindowTitle =
  tr("Audio Controls");

CommandQList AudioControlWidget::Query() {
  CommandQList query;

  query << Command::RetAudioGlobalParm;
  query << Command::RetAudioDeviceParm;
  query << Command::RetAudioControlParm;
  query << Command::RetAudioControlDetail;
  query << Command::RetAudioControlDetailValue;
  query << Command::RetAudioPortMeterValue;

  return query;
}

AudioControlWidget::AudioControlWidget(DeviceInfoPtr device, QWidget *parent)
    : RefreshObject(parent), device(device) {

  auto hBoxLayout = new QHBoxLayout();
  setLayout(hBoxLayout);
  hBoxLayout->setSpacing(4);
  hBoxLayout->setContentsMargins(4, 4, 4, 4);
  buildFeatureGroups();
  setWindowTitle(DefaultWindowTitle);

}

void AudioControlWidget::buildFeatureGroups() {
  const auto &audioGlobalParm = device->get<AudioGlobalParm>();
  for (auto audioPortID = 1; audioPortID <= audioGlobalParm.numAudioPorts();
       ++audioPortID) {
    const auto &audioDeviceParm = device->get<AudioDeviceParm>(audioPortID);
    for (auto controllerNumber = 1;
         controllerNumber <= audioDeviceParm.maxControllers();
         ++controllerNumber) {

      const AudioControlParm &audioControlParm =
          device->get<AudioControlParm>(audioPortID, controllerNumber);
      switch (audioControlParm.controllerType()) {
        case ControllerType::Feature: {
          AudioControlFeatureSourcePtr source =
              AudioControlFeatureSourcePtr(new AudioControlFeatureSource(
                  device, audioPortID, controllerNumber));

          //printf("AudioControlFeatureSource %d, %d\n", audioPortID,controllerNumber);

          auto audioChannelWidget =  new AudioChannelsControlWidget(source, device, audioPortID);
          layout()->addWidget(audioChannelWidget);
          audioControlFeatures.push_back(source);
          refreshList.push_back(audioChannelWidget);
        }
        default:
          break;
      }
    }
  }
}

void AudioControlWidget::refreshWidget() {
  for (auto *refresh : refreshList) {
    refresh->refreshWidget();
  }
}
