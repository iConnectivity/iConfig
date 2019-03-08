/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "AudioChannelConfigWidget.h"
#include "AudioControlFeatureSource.h"
#include "AudioPatchbayParm.h"
#include "AudioDeviceParm.h"
#include "AudioControlParm.h"

#include <QCheckBox>
#include <QDebug>

using namespace std;
using namespace GeneSysLib;

AudioChannelConfigWidget::AudioChannelConfigWidget(AudioFeatureControlWidget* audioControl, DeviceInfoPtr device, IAudioControlFeatureSourcePtr audioFeatureSource,
                                                   Word audioPortID, Byte inChannelID, Byte outChannelID, QWidget* parent, int totalOutputChannelInPort)  //Bugfixing for PlayAudio, zx-03-02
  : device(device),
  audioControl(audioControl),
  audioPortID(audioPortID),
  audioFeatureSource(audioFeatureSource),
  inChannelID(inChannelID),
  outChannelID(outChannelID),
  parent(parent),
  QWidget(parent)
{
  m_TotalOutputChannelInPort = totalOutputChannelInPort;

  stereoLinked = false;

  QVBoxLayout* topLayout = new QVBoxLayout;
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

  mixerInterface = new MixerInterface(device);
  mixerOutputInterface = new MixerOutputInterface(device);
  mixerInputInterface = new MixerInputInterface(device);

  //Bugfixing PlayAudio, replacing 4 with nHeadphoneIDOffset, zx-03-01
  int nHeadphoneIDOffset = m_TotalOutputChannelInPort - 2;  //Bugfixing for PlayAudio, zx-03-02
  trueChannelID = 0;
  if (inChannelID == 0) {
    if (audioFeatureSource->stereoLink(outChannelID))
      stereoLinked = true;

    trueChannelID = outChannelID;

    if (!audioFeatureSource->controllerName().compare("Headphones")) {
      trueChannelID += nHeadphoneIDOffset; //Bugfixing PlayAudio, zx-03-01
    }
  }
  else {
    if (audioFeatureSource->stereoLink(inChannelID))
      stereoLinked = true;
  }

  const auto &audioGlobalParm = device->get<AudioGlobalParm>();

  for (Word audioPortIDt = 1; audioPortIDt <= audioGlobalParm.numAudioPorts(); ++audioPortIDt) {
   // const auto &portParm = device->get<AudioPortParm>(audioPortIDt);
    // TODO: Should be real names.
    //portNames.append(QString::fromStdString(portParm.portName()));
    switch(audioPortIDt) {
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

  int numOutputs = mixerInterface->numberOutputs(audioPortID);
  QString mixName = "";
  for (int j = 1; j <= numOutputs; j = j + 2) {
    if (audioPortID == 1) { // TODO: hack for now
      mixName = "Mix U1: " + QString::fromStdString(mixerInterface->mixName(audioPortID,j));
    }
    else if (audioPortID == 2) {
      mixName = "Mix U2: " + QString::fromStdString(mixerInterface->mixName(audioPortID,j));
    }
    else {
      mixName =  "Mix A: " + QString::fromStdString(mixerInterface->mixName(audioPortID,j));
 //     mixName = QString::number(outChannelID) + ":"+QString::number(trueChannelID) + "-" + QString::fromStdString(mixerInterface->mixName(audioPortID,j));
      //printf("mix name: %s\n", mixName.toStdString().c_str());
    }
    portNames.append(mixName);
  }

  if (inChannelID == 0) {
    int nPortCount = audioGlobalParm.numAudioPorts();  //Bugfixing for PlayAudio, zx-03-02
    qDebug() << "Audio Port Count: " << nPortCount;
    for (Word audioPortIDt = 1; audioPortIDt <= nPortCount; ++audioPortIDt) {

      const auto &portParm = device->get<AudioPortParm>(audioPortIDt);
      qDebug() << "Audio Port ID: " << audioPortIDt << "Output Channel Numbers: " << portParm.numOutputChannels();


      for (Byte outChannelIDt = 1; outChannelIDt <= portParm.numOutputChannels(); ++outChannelIDt) {
        qDebug() << "Audio Port ID: " << audioPortIDt << "Output Channel ID: " << outChannelIDt;

        allAvailableChannels.append(QPair<int,int>(audioPortIDt, outChannelIDt));
      }
    }

    //int numOutputs = mixerInterface->numberOutputs(audioPortID);
    for (int j = 1; j <= numOutputs; j = j + 2) {
      int nKey = (nPortCount+1) + (j-1)/2;
      int nVal1 = j - (nKey-(nPortCount+1));
      allAvailableChannels.append(QPair<int,int>(nKey, nVal1));//1));  //Bugfixing PlayAudio, replacing 4 with nHeadphoneIDOffset, zx-03-01
      allAvailableChannels.append(QPair<int,int>(nKey, nVal1+1));//2)); //Bugfixing PlayAudio, replacing 4 with nHeadphoneIDOffset, zx-03-01
    }

    auto &patchbay = device->get<AudioPatchbayParm>(audioPortID);
    auto &configBlock = patchbay.findInputBlock(trueChannelID);

    if (configBlock.portIDOfOutput() != 0) {
      currentChannels.append(QPair<int,int>(configBlock.portIDOfOutput(), configBlock.outputChannelNumber()));
    }
    if (stereoLinked) {
      auto &patchbay = device->get<AudioPatchbayParm>(audioPortID);
      auto &configBlock = patchbay.findInputBlock(trueChannelID + 1);

      if (configBlock.portIDOfOutput() != 0) {
        currentChannelsPaired.append(QPair<int,int>(configBlock.portIDOfOutput(), configBlock.outputChannelNumber()));
      }
    }

    for (int j = 1; j <= numOutputs; j++) {
      for (int k = 0; k < mixerInterface->channelIDsForOutput(audioPortID, j).size(); k++) {
        if (mixerInterface->channelIDsForOutput(audioPortID, j).at(k) == trueChannelID) {
          if (j % 2) {
            //currentChannels.append(QPair<int,int>((nPortCount+1) + (j-1)/2, 1)); //Bugfixing PlayAudio, replacing 4 with nHeadphoneIDOffset, zx-03-01
            int nKey = (nPortCount+1) + (j-1)/2;
            int nVal1 = j - (nKey-(nPortCount+1));
            currentChannels.append(QPair<int,int>(nKey, nVal1)); //Bugfixing PlayAudio, replacing 4 with nHeadphoneIDOffset, zx-03-01
          }
          else {
            //currentChannels.append(QPair<int,int>((nPortCount+1) + (j-2)/2, 2)); //Bugfixing PlayAudio, replacing 4 with nHeadphoneIDOffset, zx-03-01
            int nKey = (nPortCount+1) + (j-2)/2;
            int nVal1 = j - (nKey-(nPortCount+1));
            currentChannels.append(QPair<int,int>(nKey, nVal1)); //Bugfixing PlayAudio, replacing 4 with nHeadphoneIDOffset, zx-03-01
          }
        }
      }
    }

    if (stereoLinked) {
      for (int j = 1; j <= numOutputs; j++) {
        int nKey = (nPortCount+1) + (j-1)/2;
        int nVal1 = j - (nKey-(nPortCount+1));
        for (int k = 0; k < mixerInterface->channelIDsForOutput(audioPortID, j).size(); k++) {
          if (mixerInterface->channelIDsForOutput(audioPortID, j).at(k) == trueChannelID + 1) {
            if (j % 2) {
              //currentChannelsPaired.append(QPair<int,int>((nPortCount+1) + (j-1)/2, 1)); //Bugfixing PlayAudio, replacing 4 with nHeadphoneIDOffset,  zx-03-01
              int nKey = (nPortCount+1) + (j-1)/2;
              int nVal1 = j - (nKey-(nPortCount+1));
              currentChannelsPaired.append(QPair<int,int>(nKey, nVal1)); //Bugfixing PlayAudio, replacing 4 with nHeadphoneIDOffset,  zx-03-01
            }
            else {
              //currentChannelsPaired.append(QPair<int,int>((nPortCount+1) + (j-2)/2, 2)); //Bugfixing PlayAudio, replacing 4 with nHeadphoneIDOffset, zx-03-01
              int nKey = (nPortCount+1) + (j-2)/2;
              int nVal1 = j - (nKey-(nPortCount+1));
              currentChannelsPaired.append(QPair<int,int>(nKey, nVal1)); //Bugfixing PlayAudio, replacing 4 with nHeadphoneIDOffset, zx-03-01
            }
          }
        }
      }
    }
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

    bool bRight = false;//Fix headphone zx, 2017-06-28
    for (int i = 0; i < allAvailableChannels.size(); i++) {
      QHBoxLayout* hLayout = new QHBoxLayout;
      QCheckBox* checkBox = new QCheckBox;
      if (j == 0) {
        if (currentChannels.contains(allAvailableChannels.at(i))) {
          //printf("yup!\n");
          checkBox->setChecked(true);
        }
      }
      else {
        if (currentChannelsPaired.contains(allAvailableChannels.at(i))) {
          checkBox->setChecked(true);
        }
      }
      checkBox->setProperty("whichCheckbox", QVariant(i));
      if (inChannelID == 0)
        checkBox->setProperty("outChannelID", QVariant(trueChannelID + j));
      else {
        checkBox->setProperty("inChannelID", QVariant(inChannelID + j));
      }
      // TODO: special hack for headphones
      if (allAvailableChannels.at(i).first > 3) {
        //if (allAvailableChannels.at(i).second % 2)
        //if (allAvailableChannels.at(i).second % 2)
        //Fix headphone zx, 2017-06-28
        if(bRight == false)
        {
          checkBox->setText(portNames.at(allAvailableChannels.at(i).first - 1) + " : Left"); //??
          bRight = true;
         // checkBox->setText("Left:" + QString::number(allAvailableChannels.at(i).second)+"::"+QString::number(allAvailableChannels.at(i).first));
        }
        else
        {
          bRight = false;
          checkBox->setText(portNames.at(allAvailableChannels.at(i).first - 1) + " : Right"); //??
          //checkBox->setText("Right:" + QString::number(allAvailableChannels.at(i).second)+"::"+QString::number(allAvailableChannels.at(i).first));
        }
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
  setMinimumWidth(290);
}

AudioChannelConfigWidget::~AudioChannelConfigWidget(){}

void AudioChannelConfigWidget::handleDone() {
  //printf("done!\n");
  //mixer->notifyChannelChange();
  parent->close();
}

void AudioChannelConfigWidget::handleCheckBox(bool checked) {
  QCheckBox* chk = dynamic_cast<QCheckBox*>(sender());
  if (inChannelID == 0) {
    if( chk != NULL )
    {
      int which = chk->property("whichCheckbox").toInt();
      int outChannelIDForCheckBox = chk->property("outChannelID").toInt();
      //printf("which: %d\n", which);
//      qDebug() << "CheckBox which:" << which << " Output ChannelID:" << outChannelIDForCheckBox;

      QPair<int,int> channel = allAvailableChannels.at(which);
      qDebug() << "CheckBox which:" << which << " Output ChannelID:" << outChannelIDForCheckBox << "channel.first:" << channel.first << " channel.second:" << channel.second ;

      if (checked) {
        if (channel.first > 3) { // is a mixer
          std::vector<int8_t> outputs = mixerInterface->channelIDsForOutput(audioPortID, (channel.first - 4) + channel.second);
          outputs.push_back(outChannelIDForCheckBox);
          mixerInterface->channelIDsForOutput(audioPortID, (channel.first - 4) + channel.second, outputs);
          currentChannels.append(channel);

          int numOutputs = mixerInterface->numberOutputs(audioPortID);
          for (int j = 1; j <= numOutputs; j++) {
            if (j != (channel.first - 4) + channel.second) {
              std::vector<int8_t> outputs = mixerInterface->channelIDsForOutput(audioPortID,j);
              for (int i = 0; i < outputs.size(); i++) {
                if (outputs.at(i) == outChannelIDForCheckBox) {
                  outputs.erase(outputs.begin() + i);
                  break;
                }
              }
              mixerInterface->channelIDsForOutput(audioPortID, j, outputs);
            }
          }

          auto &patchbay = device->get<AudioPatchbayParm>(audioPortID);
          auto &configBlock = patchbay.findInputBlock(outChannelIDForCheckBox);

          configBlock.portIDOfOutput((Word) 0);
          configBlock.outputChannelNumber((Byte) 0);

          device->send<SetAudioPatchbayParmCommand>(patchbay);

          for (QCheckBox* ck : checkBoxes) {
            if (ck->property("outChannelID").toInt() == outChannelIDForCheckBox) {
              if (ck->property("whichCheckbox").toInt() != which)
                ck->setChecked(false);
            }
          }
        }
        else { // not a mixer
          auto &patchbay = device->get<AudioPatchbayParm>(audioPortID);
          auto &configBlock = patchbay.findInputBlock(outChannelIDForCheckBox);

          configBlock.portIDOfOutput((Word) channel.first);
          configBlock.outputChannelNumber((Byte) channel.second);

          device->send<SetAudioPatchbayParmCommand>(patchbay);

          int numOutputs = mixerInterface->numberOutputs(audioPortID);
          for (int j = 1; j <= numOutputs; j++) {
            std::vector<int8_t> outputs = mixerInterface->channelIDsForOutput(audioPortID,j);
            for (int i = 0; i < outputs.size(); i++) {
              if (outputs.at(i) == outChannelIDForCheckBox) {
                outputs.erase(outputs.begin() + i);
                break;
              }
            }
            mixerInterface->channelIDsForOutput(audioPortID, j, outputs);
          }
          for (QCheckBox* ck : checkBoxes) {
            if (ck->property("outChannelID").toInt() == outChannelIDForCheckBox) {
              if (ck->property("whichCheckbox").toInt() != which)
                ck->setChecked(false);
            }
          }
        }
      }
      else {
        if (channel.first > 3) { // is a mixer
          std::vector<int8_t> outputs = mixerInterface->channelIDsForOutput(audioPortID, (channel.first - 4) + channel.second);
          for (int i = 0; i < outputs.size(); i++) {
            if (outputs.at(i) == outChannelIDForCheckBox) {
              outputs.erase(outputs.begin() + i);
              break;
            }
          }
          mixerInterface->channelIDsForOutput(audioPortID, (channel.first - 4) + channel.second, outputs);
          if (outChannelIDForCheckBox == outChannelIDForCheckBox)
            currentChannels.removeOne(channel);
          else
            currentChannelsPaired.removeOne(channel);
        }
        else { // not a mixer
          auto &patchbay = device->get<AudioPatchbayParm>(audioPortID);
          auto &configBlock = patchbay.findInputBlock(outChannelIDForCheckBox);

          configBlock.portIDOfOutput((Word) 0);
          configBlock.outputChannelNumber((Byte) 0);

          device->send<SetAudioPatchbayParmCommand>(patchbay);
        }
      }
    }
  }
  //Bugfixing for PlayAudio, zx, 2017-03-01
  if(audioControl != NULL) {
      audioControl->updateChannelNameLabel();
  }
}
