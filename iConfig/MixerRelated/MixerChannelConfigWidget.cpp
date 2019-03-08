/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "MixerChannelConfigWidget.h"
#include "AudioPatchbayParm.h"

#include <QCheckBox>

using namespace std;
using namespace GeneSysLib;

MixerChannelConfigWidget::MixerChannelConfigWidget(MixerChannelWidget* mixer, DeviceInfoPtr device,
                                                   Word audioPortID, Byte mixerOutputNumber, Byte mixerInputNumber, QWidget* parent):
  device(device),
  mixer(mixer),
  audioPortID(audioPortID),
  mixerOutputNumber(mixerOutputNumber),
  mixerInputNumber(mixerInputNumber),
  parent(parent),
  QWidget(parent)
{
  stereoLinked = false;

  QVBoxLayout* topLayout = new QVBoxLayout;

  mixerInterface = new MixerInterface(device);

  if (mixerInputNumber == 0) {
    auto mixerOutputInterface = new MixerOutputInterface(device);
    if (mixerOutputInterface->stereoLink(audioPortID, mixerOutputNumber))
      stereoLinked = true;
  }
  else {
    auto mixerInputInterface = new MixerInputInterface(device);
    if (mixerInputInterface->stereoLink(audioPortID, mixerOutputNumber, mixerInputNumber))
      stereoLinked = true;
  }

  const auto &audioGlobalParm = device->get<AudioGlobalParm>();

  for (Word audioPortID = 1; audioPortID <= audioGlobalParm.numAudioPorts(); ++audioPortID) {
    const auto &portParm = device->get<AudioPortParm>(audioPortID);
    // TODO: Should be real names.
    //portNames.append(QString::fromStdString(portParm.portName()));
    switch(audioPortID) {
    case 1:
      portNames.append(QString("USB1"));
      break;
    case 2:
      portNames.append(QString("USB2"));
      break;
    case 3:
      portNames.append(QString("ANLG"));
      break;
    default:
      portNames.append(QString("UNKN"));
      break;
    }
  }

  if (mixerInputNumber == 0) {
    const auto &portParm = device->get<AudioPortParm>(audioPortID);
    for (Byte outChannelID = 1; outChannelID <= portParm.numInputChannels(); ++ outChannelID) {
      allAvailableChannels.append(QPair<int,int>(audioPortID, outChannelID));
    }

    int numberOutputs = mixerInterface->channelIDsForOutput(audioPortID, mixerOutputNumber).size();
    for (int i = 0; i < numberOutputs; i++) {
      currentChannels.append(QPair<int,int>(audioPortID,mixerInterface->channelIDsForOutput(audioPortID, mixerOutputNumber).at(i)));
    }

    numberOutputs = mixerInterface->channelIDsForOutput(audioPortID, mixerOutputNumber + 1).size();
    for (int i = 0; i < numberOutputs; i++) {
      currentChannelsPaired.append(QPair<int,int>(audioPortID,mixerInterface->channelIDsForOutput(audioPortID, mixerOutputNumber + 1).at(i)));
    }
  }
  else {
    for (Word audioPortID = 1; audioPortID <= audioGlobalParm.numAudioPorts(); ++audioPortID) {
      const auto &portParm = device->get<AudioPortParm>(audioPortID);
      for (Byte outChannelID = 1; outChannelID <= portParm.numOutputChannels(); ++ outChannelID) {
        allAvailableChannels.append(QPair<int,int>(audioPortID, outChannelID));
      }
    }

    currentChannels.append(QPair<int,int>(mixerInterface->audioPortIDForInput(audioPortID, mixerInputNumber),mixerInterface->channelIDForInput(audioPortID, mixerInputNumber)));
    if (stereoLinked)
      currentChannelsPaired.append(QPair<int,int>(mixerInterface->audioPortIDForInput(audioPortID, mixerInputNumber + 1),mixerInterface->channelIDForInput(audioPortID, mixerInputNumber + 1)));
  }

  setMinimumHeight((allAvailableChannels.size() + 1) * 20 + 15);

  QHBoxLayout *bigHLayout = new QHBoxLayout;
  bigHLayout->setContentsMargins(0, 0, 0, 0);
  bigHLayout->setSpacing(0);

  for (int j = 0; j <= (int)stereoLinked; j++) {
    QVBoxLayout* vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    if (stereoLinked) {
      QLabel* titleLabel = new QLabel();
      titleLabel->setStyleSheet("font-size: 12pt;");
      if (j == 0) {
        titleLabel->setText("Left");
      }
      else {
        titleLabel->setText("Right");
      }
      vLayout->addWidget(titleLabel);
    }

    //PlayAudio UI bugfixing, zx-03-01
    int nChannelTotal = allAvailableChannels.size();
    for (int i = 0; i < nChannelTotal; i++) {
      QHBoxLayout* hLayout = new QHBoxLayout;
      QCheckBox* checkBox = new QCheckBox;
      if (j == 0) {
        if (currentChannels.contains(allAvailableChannels.at(i))) {
          checkBox->setChecked(true);
        }
      }
      else {
        if (currentChannelsPaired.contains(allAvailableChannels.at(i))) {
          checkBox->setChecked(true);
        }
      }
      checkBox->setProperty("whichCheckbox", QVariant(i));
      if (mixerInputNumber == 0)
        checkBox->setProperty("mixerOutputNumber", QVariant(mixerOutputNumber + j));
      else {
        checkBox->setProperty("mixerInputNumber", QVariant(mixerInputNumber + j));
      }
      // TODO: special hack for headphones
      //PlayAudio UI bugfixing, zx-03-01
      if (allAvailableChannels.at(i).first == 3 && allAvailableChannels.at(i).second > nChannelTotal-2) {
        if (allAvailableChannels.at(i).second == nChannelTotal-1)
          checkBox->setText(portNames.at(allAvailableChannels.at(i).first - 1) + " : " + "HL");
        else
          checkBox->setText(portNames.at(allAvailableChannels.at(i).first - 1) + " : " + "HR");
      }
      else {
        checkBox->setText(portNames.at(allAvailableChannels.at(i).first - 1) + " : " + QString::number(allAvailableChannels.at(i).second));
      }

      connect(checkBox, SIGNAL(clicked(bool)),this,SLOT(handleCheckBox(bool)));
      checkBoxes.append(checkBox);

      hLayout->setContentsMargins(0, 0, 0, 0);
      hLayout->setSpacing(0);

      hLayout->addWidget(checkBox);
      QSizePolicy exp = QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
      checkBox->setSizePolicy(exp);
      //hLayout->addWidget(lbl);
      vLayout->addLayout(hLayout);
      vLayout->setAlignment(hLayout, Qt::AlignTop);
    }
    vLayout->setAlignment(Qt::AlignCenter | Qt::AlignTop);
    bigHLayout->addLayout(vLayout);
  }

  topLayout->addLayout(bigHLayout);

  doneButton = new QPushButton();
  doneButton->setMinimumWidth(50);
  doneButton->setMaximumWidth(50);
  doneButton->setText("Done");
  doneButton->setStyleSheet(QString("QPushButton { color:white; padding:3px; border: 1px solid #8f8f91; border-radius: 6px; background-color: #00BFFF;}"));

  connect(doneButton,SIGNAL(clicked()), this, SLOT(handleDone()));

  QWidget* spacingWidget = new QWidget();
  spacingWidget->setMinimumHeight(5);
  spacingWidget->setMaximumHeight(5);
  topLayout->addWidget(spacingWidget);
  topLayout->addWidget(doneButton);
  topLayout->setAlignment(doneButton, Qt::AlignHCenter);

  topLayout->setContentsMargins(0, 0, 0, 0);
  topLayout->setSpacing(0);
  topLayout->setAlignment(Qt::AlignCenter);

  setLayout(topLayout);
}

MixerChannelConfigWidget::~MixerChannelConfigWidget(){}

void MixerChannelConfigWidget::handleDone() {
  //printf("done!\n");
  mixer->notifyChannelChange();
  parent->close();
}

void MixerChannelConfigWidget::handleCheckBox(bool checked) {
  QCheckBox* chk = dynamic_cast<QCheckBox*>(sender());
  if (mixerInputNumber == 0) {
    if( chk != NULL )
    {
      int which = chk->property("whichCheckbox").toInt();
      int mixerOutputNumberForCheckBox = chk->property("mixerOutputNumber").toInt();
      //printf("which: %d\n", which);

      QPair<int,int> channel = allAvailableChannels.at(which);

      if (checked) {
        std::vector<int8_t> outputs = mixerInterface->channelIDsForOutput(audioPortID, mixerOutputNumberForCheckBox);
        outputs.push_back(channel.second);
        mixerInterface->channelIDsForOutput(audioPortID, mixerOutputNumberForCheckBox, outputs);
        currentChannels.append(channel);

        int numOutputs = mixerInterface->numberOutputs(audioPortID);
        for (int j = 1; j <= numOutputs; j++) {
          if (j != mixerOutputNumberForCheckBox) {
            std::vector<int8_t> outputs = mixerInterface->channelIDsForOutput(audioPortID,j);
            for (int i = 0; i < outputs.size(); i++) {
              if (outputs.at(i) == channel.second) {
                outputs.erase(outputs.begin() + i);
                for (QCheckBox* ck : checkBoxes) {
                  if (ck->property("whichCheckbox").toInt() == which &&
                      ck->property("mixerOutputNumber").toInt() != mixerOutputNumberForCheckBox) {
                    ck->setChecked(false);
                  }
                }
                break;
              }
            }
            mixerInterface->channelIDsForOutput(audioPortID, j, outputs);
          }
        }

        auto &patchbay = device->get<AudioPatchbayParm>(audioPortID);
        auto &configBlock = patchbay.findInputBlock(channel.second);

        configBlock.portIDOfOutput((Word) 0);
        configBlock.outputChannelNumber((Byte) 0);

        device->send<SetAudioPatchbayParmCommand>(patchbay);

        mixer->notifyChannelChange();
      }
      else {
        std::vector<int8_t> outputs = mixerInterface->channelIDsForOutput(audioPortID,mixerOutputNumberForCheckBox);
        for (int i = 0; i < outputs.size(); i++) {
          if (outputs.at(i) == channel.second) {
            outputs.erase(outputs.begin() + i);
            break;
          }
        }
        mixerInterface->channelIDsForOutput(audioPortID, mixerOutputNumberForCheckBox, outputs);
        if (mixerOutputNumberForCheckBox == mixerOutputNumber)
          currentChannels.removeOne(channel);
        else
          currentChannelsPaired.removeOne(channel);

        mixer->notifyChannelChange();
      }
    }
  }
  else {
    if( chk != NULL )
    {
      int which = chk->property("whichCheckbox").toInt();
      int mixerInputNumberForCheckBox = chk->property("mixerInputNumber").toInt();

      QPair<int,int> channel = allAvailableChannels.at(which);

      if (checked) {
        mixerInterface->audioPortIDForInput(audioPortID,mixerInputNumberForCheckBox, channel.first);
        mixerInterface->channelIDForInput(audioPortID,mixerInputNumberForCheckBox, channel.second);

        if (channel.first < 3) { // TODO: another hack
          MixerInputInterface* mixerInputInterface = new MixerInputInterface(device);
          for (int i = 1; i <= mixerInterface->numberOutputs(audioPortID); i ++) { // set usb inputs to 0
            mixerInputInterface->volumeCurrent(audioPortID, i, mixerInputNumberForCheckBox, 0);
          }
        }
        else {
          MixerInputInterface* mixerInputInterface = new MixerInputInterface(device);
          for (int i = 1; i <= mixerInterface->numberOutputs(audioPortID); i ++) { // set analog inputs to -inf
            mixerInputInterface->volumeCurrent(audioPortID, i, mixerInputNumberForCheckBox, (uint16_t)0x8000);
          }
        }

        for (QCheckBox* ck : checkBoxes) {
          if (ck->property("whichCheckbox").toInt() != which &&
              ck->property("mixerInputNumber").toInt() == mixerInputNumberForCheckBox) {
            ck->setChecked(false);
          }
        }

        if (mixerInputNumberForCheckBox == mixerInputNumber)
          currentChannels.append(channel);
        else
          currentChannelsPaired.append(channel);

        mixer->notifyChannelChange();
      }
      else {
        mixerInterface->audioPortIDForInput(audioPortID,mixerInputNumberForCheckBox, 0);
        mixerInterface->channelIDForInput(audioPortID,mixerInputNumberForCheckBox, 0);

        if (mixerInputNumberForCheckBox == mixerInputNumber)
          currentChannels.removeOne(channel);
        else
          currentChannelsPaired.removeOne(channel);
        mixer->notifyChannelChange();
      }
    }
  }
}
