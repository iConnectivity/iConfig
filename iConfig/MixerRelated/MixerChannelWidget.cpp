/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "MixerChannelWidget.h"
#include "MixerPortWidget.h"
#include "AudioGlobalParm.h"
#include "AudioPortParm.h"
#include "MixerChannelConfigWidget.h"

#include <QLineEdit>

const double MixerChannelWidget::dbConversionFactor = 1.0 / 256.0;

using namespace GeneSysLib;

MixerChannelWidget::MixerChannelWidget(DeviceInfoPtr device, Word audioPortID,
                                       Byte mixerInputNumber, Byte mixerOutputNumber,
                                       MixerType mixerType, MixerPortWidget* parent)
  : device(device), audioPortID(audioPortID), mixerInputNumber(mixerInputNumber),
    mixerOutputNumber(mixerOutputNumber), mixerType(mixerType), parent(parent)
{
  mixerInterface = new MixerInterface(device);
  mixerInputInterface = new MixerInputInterface(device);
  mixerOutputInterface = new MixerOutputInterface(device);

  auto vBoxLayout = new QVBoxLayout();
  vBoxLayout->setSpacing(0);
  vBoxLayout->setContentsMargins(0, 0, 0, 0);
  vBoxLayout->setAlignment(Qt::AlignTop);
  setLayout(vBoxLayout);
  setContentsMargins(0,0,0,0);
  setStyleSheet("border: 1px white;");

  otherChannel = 0;
  disabled = false;

  panBoxLayout = new QHBoxLayout();
  panBoxLayout->setContentsMargins(0,0,0,0);
  panBoxLayout->setSpacing(0);
  panBoxLayout->setAlignment(Qt::AlignCenter);
  panBox = new QWidget();
  panBox->setContentsMargins(0,0,0,0);

  invertBoxLayout = new QHBoxLayout();

  QSizePolicy exp = QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);

  invertBoxLayout->setContentsMargins(0, 10, 0, 0);
  invertBoxLayout->setSpacing(0);
  invertBox = new QWidget();
  invertBox->setSizePolicy(exp);
/*  invertBox->setContentsMargins(0,0,0,0);
  invertBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
  invertBox->setStyleSheet("padding:-10px;");*/

  this->setMaximumWidth(60);
  this->setMinimumWidth(60);
  //invertBox->setMinimumWidth(80);

  this->setStyleSheet(QString("QComboBox {font-size:9pt;}; QPushButton { font-size:9pt};"));

  soloLightOn = false;

  if (mixerType == in) {
    stereoLinked = mixerInputInterface->stereoLink(audioPortID, mixerOutputNumber, mixerInputNumber);
    outputStereoLinked = mixerOutputInterface->stereoLink(audioPortID, mixerOutputNumber);

    if (mixerInputNumber % 2) {
      otherChannel = mixerInputNumber + 1;
    }
    else {
      otherChannel = mixerInputNumber - 1;
    }

    if (mixerInterface->audioPortIDForInput(audioPortID,mixerInputNumber) == 0) {
      //disabled = true;
    }
    if (stereoLinked && mixerInterface->audioPortIDForInput(audioPortID, otherChannel) == 0) {
      //disabled = true;
    }
  }
  else {
    outputStereoLinked = stereoLinked = mixerOutputInterface->stereoLink(audioPortID, mixerOutputNumber);

    if (mixerOutputNumber % 2) {
      otherChannel = mixerOutputNumber + 1;
    }
    else {
      otherChannel = mixerOutputNumber - 1;
    }

    if (mixerInterface->channelIDsForOutput(audioPortID,mixerOutputNumber).size() == 0) {
      //disabled = true;
    }

    if (outputStereoLinked && mixerInterface->channelIDsForOutput(audioPortID, otherChannel).size() == 0) {
      //disabled = true;
    }
  }

  if (stereoLinked) {
    this->setMaximumWidth(80);
    this->setMinimumWidth(80);
    //invertBox->setMinimumWidth(160);
  }
  if (mixerType == out) {
    this->setMaximumWidth(120);
  }

  clippingTimer1 = new QTimer();
  clippingTimer2 = new QTimer();
  connect(clippingTimer1,SIGNAL(timeout()), this, SLOT(turnOffClipping1()));
  connect(clippingTimer2,SIGNAL(timeout()), this, SLOT(turnOffClipping2()));

  buildAll();
  updateAll();
}

MixerChannelWidget::~MixerChannelWidget () {

}

void MixerChannelWidget::volumeSliderChanged(int state)
{
  //printf("volumeSliderChanged called\n");
  int volMin = 0;
  int volMax = 0;
  int volRes = 0;
  int volCurrent = 0;
  if (mixerType == in) {
    volMin = mixerInputInterface->volumeMin(audioPortID);
    volMax = mixerInputInterface->volumeMax(audioPortID);
    volRes = mixerInputInterface->volumeResolution(audioPortID);
    volCurrent = mixerInputInterface->volumeCurrent(audioPortID, mixerOutputNumber, mixerInputNumber);
  }
  else {
    volMin = mixerOutputInterface->volumeMin(audioPortID);
    volMax = mixerOutputInterface->volumeMax(audioPortID);
    volRes = mixerOutputInterface->volumeResolution(audioPortID);
    volCurrent = mixerOutputInterface->volumeCurrent(audioPortID, mixerOutputNumber);
  }
  int16_t toSend = pixelsToIntForICA(state);
  if (volCurrent != toSend) {
    if (mixerType == in) {
      mixerInputInterface->volumeCurrent(audioPortID, mixerOutputNumber, mixerInputNumber, toSend);
    }
    else {
      mixerOutputInterface->volumeCurrent(audioPortID, mixerOutputNumber, toSend);
    }
  }
  if (((uint16_t) toSend) == 0x8000) {
    currentVolumeLabel->setValue(-100000);
  }
  else {
    currentVolumeLabel->setValue(pixelsToDb(state));
  }
}

void MixerChannelWidget::panDialChanged(int state)
{
  int panMin = 0;
  int panMax = 0;
  int panCurrent = 0;
  if (mixerType == in) {
    panMin = mixerInputInterface->panMin(audioPortID);
    panMax = mixerInputInterface->panMax(audioPortID);
    panCurrent = mixerInputInterface->panCurrent(audioPortID, mixerOutputNumber, mixerInputNumber);
  }
  else {
    panMin = mixerOutputInterface->panMin(audioPortID);
    panMax = mixerOutputInterface->panMax(audioPortID);
    panCurrent = mixerOutputInterface->panCurrent(audioPortID, mixerOutputNumber);
  }
  if (panCurrent != state) {
    if (mixerType == in) {
      mixerInputInterface->panCurrent(audioPortID, mixerOutputNumber, mixerInputNumber, state);
    }
    else {
      mixerOutputInterface->panCurrent(audioPortID, mixerOutputNumber, state);

    }
  }

  QString toDisplay;
  if (state == 0) {
    toDisplay = QString("C");
  }
  else if (state < 0) {
    toDisplay = QString("L ") + QString::number(abs(state)).rightJustified(3, ' ').replace(" ", "&nbsp;");
  }
  else {
    toDisplay = QString("R ") + QString::number(state).rightJustified(3, ' ').replace(" ", "&nbsp;");
  }
  currentPanLabel->setText("<b>" + toDisplay + "</b>");
}

void MixerChannelWidget::soloDialChanged(int state)
{
  int soloMin = 0;
  int soloMax = 0;
  int soloRes = 0;
  int soloCurrent = 0;
  if (mixerType == out) {
    soloMin = mixerOutputInterface->volumeMin(audioPortID);
    soloMax = mixerOutputInterface->volumeMax(audioPortID);
    soloRes = mixerOutputInterface->volumeResolution(audioPortID);
    soloCurrent = mixerOutputInterface->solo(audioPortID, mixerOutputNumber);
  }
  int16_t toSend = ((int)(state / dbConversionFactor * (soloRes * dbConversionFactor))) & 0xFFFF;
  if (state == soloDial->minimum()) {
    toSend = 0x8000;
  }
  if (soloCurrent != toSend) {
    if (mixerType == out) {
      mixerOutputInterface->solo(audioPortID, mixerOutputNumber, toSend);
    }
  }
  if (((uint16_t) toSend) == 0x8000) {
    currentSoloLabel->setText("<b>" + QString("- &infin;").rightJustified(5, ' ').replace(" ", "&nbsp;") + " dB</b>");
  }
  else {
    currentSoloLabel->setText("<b>" + QString::number(((int)(state / dbConversionFactor * (soloRes * dbConversionFactor))) *
                                                      dbConversionFactor,'f',1).rightJustified(5, ' ').replace(" ", "&nbsp;") + " dB</b>");
  }
}

void MixerChannelWidget::muteStateChanged(bool state)
{
  if (mixerType == in) {
    if (mixerInputInterface->mute(audioPortID, mixerOutputNumber, mixerInputNumber) != state)
      mixerInputInterface->mute(audioPortID, mixerOutputNumber, mixerInputNumber, state);
  }
  else {
    if (mixerOutputInterface->mute(audioPortID, mixerOutputNumber) != state)
      mixerOutputInterface->mute(audioPortID, mixerOutputNumber, state);
  }
}

void MixerChannelWidget::stereoLinkStateChanged(bool state)
{
  if (mixerType == in) {
    if (mixerInputInterface->stereoLink(audioPortID, mixerOutputNumber,mixerInputNumber) != state)
      mixerInputInterface->stereoLink(audioPortID, mixerOutputNumber,mixerInputNumber, state);
  }
  else {
    if (mixerOutputInterface->stereoLink(audioPortID, mixerOutputNumber) != state)
      mixerOutputInterface->stereoLink(audioPortID, mixerOutputNumber, state);
  }
  //printf("notifying parent\n");
  parent->notifyStereoLinkHappened();
}

void MixerChannelWidget::invertStateChanged(bool state)
{
  if (mixerType == in) {
    if (mixerInputInterface->invert(audioPortID, mixerOutputNumber,mixerInputNumber) != state)
      mixerInputInterface->invert(audioPortID, mixerOutputNumber,mixerInputNumber, state);
  }
  else {
    if (mixerOutputInterface->invert(audioPortID, mixerOutputNumber) != state)
      mixerOutputInterface->invert(audioPortID, mixerOutputNumber, state);
  }

}

void MixerChannelWidget::rightInvertStateChanged(bool state)
{
  if (mixerType == in) {
    if (!stereoLinked && outputStereoLinked) {
      if (mixerInputInterface->invert(audioPortID, mixerOutputNumber+1,mixerInputNumber) != state) {
        mixerInputInterface->invert(audioPortID, mixerOutputNumber+1,mixerInputNumber, state);
      }
    }
    else if (stereoLinked && !outputStereoLinked) {
      if (mixerInputInterface->invert(audioPortID, mixerOutputNumber,mixerInputNumber + 1) != state) {
        mixerInputInterface->invert(audioPortID, mixerOutputNumber,mixerInputNumber + 1, state);
      }
    }
    else if (stereoLinked && outputStereoLinked) {
      if (mixerInputInterface->invert(audioPortID, mixerOutputNumber+1,mixerInputNumber + 1) != state) {
        mixerInputInterface->invert(audioPortID, mixerOutputNumber+1,mixerInputNumber + 1, state);
      }
    }
  }
  else {
    if (mixerOutputInterface->invert(audioPortID, mixerOutputNumber + 1) != state)
      mixerOutputInterface->invert(audioPortID, mixerOutputNumber + 1, state);
  }

}

void MixerChannelWidget::soloStateChanged(bool state)
{
  if (mixerType == in) {
    if (mixerInputInterface->solo(audioPortID, mixerOutputNumber,mixerInputNumber) != state)
      mixerInputInterface->solo(audioPortID, mixerOutputNumber,mixerInputNumber, state);
  }
}

void MixerChannelWidget::soloPFLStateChanged(bool state)
{
  if (mixerType == in) {
    if (mixerInputInterface->soloPFL(audioPortID, mixerOutputNumber,mixerInputNumber) != state)
      mixerInputInterface->soloPFL(audioPortID, mixerOutputNumber,mixerInputNumber, state);
  }
  else {
    if (mixerOutputInterface->soloPFL(audioPortID, mixerOutputNumber) != state)
      mixerOutputInterface->soloPFL(audioPortID, mixerOutputNumber, state);
  }
}

void MixerChannelWidget::channelDropDownChanged(int state)
{
  if (state != 0) {
    QPair<int,int> selected = allAvailableInChannels.at(state - 1);
    mixerInterface->audioPortIDForInput(audioPortID, mixerInputNumber, selected.first);
    mixerInterface->channelIDForInput(audioPortID, mixerInputNumber, selected.second);
    disabled = false;
    updateAll();
  }
  else {
    mixerInterface->audioPortIDForInput(audioPortID, mixerInputNumber, 0);
    mixerInterface->channelIDForInput(audioPortID, mixerInputNumber, 0);

    //disabled = true;
    updateAll();
  }
}

void MixerChannelWidget::channelDropDownChangedLinked(int state)
{
  if (state != 0) {
    QPair<int,int> selected = allAvailableInChannels.at(state - 1);
    mixerInterface->audioPortIDForInput(audioPortID, mixerInputNumber+1, selected.first);
    mixerInterface->channelIDForInput(audioPortID, mixerInputNumber+1, selected.second);
    disabled = false;
    updateAll();
  }
  else {
    mixerInterface->audioPortIDForInput(audioPortID, mixerInputNumber+1, 0);
    mixerInterface->channelIDForInput(audioPortID, mixerInputNumber+1, 0);

    //disabled = true;
    updateAll();
  }
}

void MixerChannelWidget::channelConfigPressed()
{
  /*MixerChannelConfigDialog *dialog = new MixerChannelConfigDialog(this, device, audioPortID, mixerOutputNumber);
  dialog->exec();*/

  QPushButton* butt = dynamic_cast<QPushButton*>(sender());

  QFrame* popup1 = new QFrame(this, Qt::Popup | Qt::Window );
  popup1->setObjectName("popup");
  popup1->setStyleSheet("QFrame#popup { background-color: #000000; }");

  MixerChannelConfigWidget *widget = new MixerChannelConfigWidget(this, device, audioPortID, mixerOutputNumber, mixerInputNumber, popup1);

  popup1->move(QCursor::pos());
  popup1->resize(250,widget->minimumHeight() + 20);
  widget->setGeometry(10,10, 230, minimumHeight());
  popup1->show();
}


void MixerChannelWidget::notifyChannelChange()
{
  disabled = false;
  this->updateChannelNameLabel();
  this->updateVolumeValue();
}

void MixerChannelWidget::refreshWidget()
{
  updateAll();
}

void MixerChannelWidget::refreshMeters()
{
  updateMeters();
}

void MixerChannelWidget::buildAll()
{
  buildChannelNameLabel();
  buildPanDial();
  if (mixerType == out)
    buildSoloDial();
  buildVolumeSlider();
  buildMutePushButton();
  if (mixerType == in)
    buildSoloPushButton();
  buildInvertPushButton();
  if (stereoLinked || outputStereoLinked)
    buildRightInvertPushButton();
  buildSoloPFLPushButton();
}

void MixerChannelWidget::buildChannelNameLabel()
{
  if (mixerType == in) {
    outputConfigPushButton = new QPushButton();
    outputConfigPushButton->setEnabled(true);
    if (stereoLinked) {
      outputConfigPushButton->setMinimumWidth(70);
      outputConfigPushButton->setMaximumWidth(70);
    }
    else {
      outputConfigPushButton->setMinimumWidth(60);
      outputConfigPushButton->setMaximumWidth(60);
    }

#ifdef _WIN32
    outputConfigPushButton->setStyleSheet(QString("QPushButton { font-size: 7pt; text-decoration: underline; color:#ffffff; padding:3px; border: 0px; } \
                                          QPushButton:checked { color: #DC143C;  } \
                                          QPushButton:pressed { color: #DC143C;  } \
                                          QPushButton:hover { color: #DC143C;  }"));
#else
    outputConfigPushButton->setStyleSheet(QString("QPushButton { font-size: 9pt; text-decoration: underline; color:#ffffff; padding:3px; border: 0px; } \
                                                                                        QPushButton:checked { color: #DC143C;  } \
                                                                                        QPushButton:pressed { color: #DC143C;  } \
                                                                                        QPushButton:hover { color: #DC143C;  }"));
#endif

    connect(outputConfigPushButton, SIGNAL(clicked()),this,SLOT(channelConfigPressed()));

    QHBoxLayout *lay = new QHBoxLayout();
    QWidget *wid = new QWidget();
    lay->addWidget(outputConfigPushButton); lay->setMargin(0);
    wid->setLayout(lay); wid->setContentsMargins(0,0,0,0);
    layout()->addWidget(wid);
    layout()->setAlignment(wid, Qt::AlignCenter);

    allAvailableInChannels.clear();
    portNames.clear();

    const auto &audioGlobalParm = device->get<AudioGlobalParm>();

    for (Word audioPortID = 1; audioPortID <= audioGlobalParm.numAudioPorts(); ++audioPortID) {
      const auto &portParm = device->get<AudioPortParm>(audioPortID);
      // TODO: Should be real names.
      //portNames.append(QString::fromStdString(portParm.portName()));
      switch(audioPortID) {
      case 1:
        portNames.append(QString("U1"));
        break;
      case 2:
        portNames.append(QString("U2"));
        break;
      case 3:
        portNames.append(QString("A"));
        break;
      default:
        portNames.append(QString(""));
        break;
      }

      for (Byte inChannelID = 1; inChannelID <= portParm.numOutputChannels(); ++ inChannelID) {
        allAvailableInChannels.append(QPair<int,int>(audioPortID, inChannelID));
      }
    }

    bool isAssigned = false;
    bool isAssigned2 = false;
    QString title = "";
    QString title2 = "";
    for (int i=0; i< allAvailableInChannels.size(); i++) {
      if (mixerInterface->audioPortIDForInput(audioPortID,mixerInputNumber) == allAvailableInChannels.at(i).first &&
          mixerInterface->channelIDForInput(audioPortID,mixerInputNumber) == allAvailableInChannels.at(i).second) {
        title = (portNames.at(allAvailableInChannels.at(i).first - 1) + " : "
                                 + QString::number(allAvailableInChannels.at(i).second));
        isAssigned = true;
      }
      if (stereoLinked) {
        if (mixerInterface->audioPortIDForInput(audioPortID,mixerInputNumber + 1) == allAvailableInChannels.at(i).first &&
            mixerInterface->channelIDForInput(audioPortID,mixerInputNumber + 1) == allAvailableInChannels.at(i).second) {
          title2 = (portNames.at(allAvailableInChannels.at(i).first - 1) + " : "
                                   + QString::number(allAvailableInChannels.at(i).second));
          isAssigned2 = true;
        }
      }
    }
    if (!isAssigned) {
      title = ("No SRC");
    }
    if (!isAssigned2) {
      title2 = ("No SRC");
    }

    if (stereoLinked)
      outputConfigPushButton->setText(title + " | " + title2);
    else
      outputConfigPushButton->setText(title);
  }
  else {
    outputConfigPushButton = new QPushButton();
    outputConfigPushButton->setEnabled(true);
    outputConfigPushButton->setMinimumWidth(100);
    outputConfigPushButton->setMaximumWidth(100);

#ifdef _WIN32
    outputConfigPushButton->setStyleSheet(QString("QPushButton { font-size: 7pt; text-decoration: underline; color:#ffffff; padding:3px; border: 0px; } \
                                          QPushButton:checked { color: #DC143C;  } \
                                          QPushButton:pressed { color: #DC143C;  } \
                                          QPushButton:hover { color: #DC143C;  }"));
#else
    outputConfigPushButton->setStyleSheet(QString("QPushButton { font-size: 9pt; text-decoration: underline; color:#ffffff; padding:3px; border: 0px; } \
                                          QPushButton:checked { color: #DC143C;  } \
                                          QPushButton:pressed { color: #DC143C;  } \
                                          QPushButton:hover { color: #DC143C;  }"));
#endif

    connect(outputConfigPushButton, SIGNAL(clicked()),this,SLOT(channelConfigPressed()));

    QHBoxLayout *lay = new QHBoxLayout();
    QWidget *wid = new QWidget();
    lay->addWidget(outputConfigPushButton); lay->setMargin(0);
    wid->setLayout(lay); wid->setContentsMargins(0,0,0,0);
    layout()->addWidget(wid);
    layout()->setAlignment(wid, Qt::AlignCenter);

    allAvailableInChannels.clear();
    portNames.clear();

    const auto &audioGlobalParm = device->get<AudioGlobalParm>();

    for (Word audioPortID = 1; audioPortID <= audioGlobalParm.numAudioPorts(); ++audioPortID) {
      const auto &portParm = device->get<AudioPortParm>(audioPortID);
      // TODO: Should be real names.
      //portNames.append(QString::fromStdString(portParm.portName()));
      switch(audioPortID) {
      case 1:
        portNames.append(QString("U1"));
        break;
      case 2:
        portNames.append(QString("U2"));
        break;
      case 3:
        portNames.append(QString("A"));
        break;
      default:
        portNames.append(QString(""));
        break;
      }

      for (Byte inChannelID = 1; inChannelID <= portParm.numInputChannels(); ++ inChannelID) {
        allAvailableInChannels.append(QPair<int,int>(audioPortID, inChannelID));
      }
    }

    QString title = "";
    QString title2 = "";
    if (mixerInterface->channelIDsForOutput(audioPortID,mixerOutputNumber).size() == 0) {
      title = "No DST";
    }
    else if (mixerInterface->channelIDsForOutput(audioPortID,mixerOutputNumber).size() > 1) {
      title = "Multiple";
    }
    if (mixerInterface->channelIDsForOutput(audioPortID,mixerOutputNumber + 1).size() == 0) {
      title2 = "No DST";
    }
    else if (mixerInterface->channelIDsForOutput(audioPortID,mixerOutputNumber + 1).size() > 1) {
      title2 = "Multiple";
    }

    if (title == "" || title2 == "") {
      for (int i=0; i< allAvailableInChannels.size(); i++) {
        if (title == "") {
          if (mixerInterface->channelIDsForOutput(audioPortID,mixerOutputNumber).size() == 1)
          {
            if (audioPortID == allAvailableInChannels.at(i).first && mixerInterface->channelIDsForOutput(audioPortID,mixerOutputNumber).at(0) == allAvailableInChannels.at(i).second) {
              title = (portNames.at(allAvailableInChannels.at(i).first - 1) + " : "
                       + QString::number(allAvailableInChannels.at(i).second));
            }
          }
        }
        if (stereoLinked) {
          if (title2 == "") {
            if (mixerInterface->channelIDsForOutput(audioPortID,mixerOutputNumber + 1).size() == 1)
            {
              if (audioPortID == allAvailableInChannels.at(i).first && mixerInterface->channelIDsForOutput(audioPortID,mixerOutputNumber + 1).at(0) == allAvailableInChannels.at(i).second) {
                title2 = (portNames.at(allAvailableInChannels.at(i).first - 1) + " : "
                         + QString::number(allAvailableInChannels.at(i).second));
              }
            }
          }
        }
      }
    }

    outputConfigPushButton->setText(title + " | " + title2);
  }
}

void MixerChannelWidget::setProgressBarColor(QProgressBar* pb, double value) {
  if (value < 0) {
    pb->setStyleSheet("QProgressBar{ margin-right: 2px; border: 0px; background-color: #545454; border-radius: 0px; text-align: center} \
    QProgressBar::chunk { background-color: #11e000; }");
  }
  else if (value < 5.9) {
    pb->setStyleSheet("QProgressBar{ margin-right: 2px; border: 0px; background-color: #545454; border-radius: 0px; text-align: center} \
    QProgressBar::chunk { background-color: #FFCC00; }");
  }
  else{
    pb->setStyleSheet("QProgressBar{ margin-right: 2px; border: 0px; background-color: #545454; border-radius: 0px; text-align: center} \
    QProgressBar::chunk { background-color: #ff0033; }");
  }
}

void MixerChannelWidget::labelDone(double value)
{
  int valToSend = value * 256;
  int pixels = toPixelsFromICA(valToSend);
  if (pixels >= 299)
    valToSend = pixelsToIntForICA(299);
  else if (pixels <= 0) {
    valToSend = 0x8000;
  }

  if (mixerType == in) {
    mixerInputInterface->volumeCurrent(audioPortID, mixerOutputNumber, mixerInputNumber, valToSend);
  }
  else {
    mixerOutputInterface->volumeCurrent(audioPortID,  mixerOutputNumber, valToSend);
  }
}

void MixerChannelWidget::buildVolumeSlider()
{
  QWidget* spacerWidget = new QWidget();
  spacerWidget->setMinimumHeight(9);
  spacerWidget->setMaximumHeight(9);
  layout()->addWidget(spacerWidget);

  int volMin = 0;
  int volMax = 0;
  int volRes = 0;
  bool volEditable = false;
  bool volAvailable = false;
  if (mixerType == in) {
    volMin = mixerInputInterface->volumeMin(audioPortID);
    volMax = mixerInputInterface->volumeMax(audioPortID);
    volRes = mixerInputInterface->volumeResolution(audioPortID);
    volEditable = mixerInputInterface->isVolumeEditable(audioPortID);
    volAvailable = mixerInputInterface->isVolumeAvailable(audioPortID);
    //printf("in min: %d, max: %d, res: %d\n", volMin, volMax, volRes);
  }
  else {
    volMin = mixerOutputInterface->volumeMin(audioPortID);
    volMax = mixerOutputInterface->volumeMax(audioPortID);
    volRes = mixerOutputInterface->volumeResolution(audioPortID);
    volEditable = mixerOutputInterface->isVolumeEditable(audioPortID);
    volAvailable = mixerOutputInterface->isVolumeAvailable(audioPortID);
    //printf("out min: %d, max: %d, res: %d\n", volMin, volMax, volRes);
  }
  if (volAvailable)
  {
    volumeSlider = new QClickySlider();
    //volumeSlider->setStyleSheet("margin-right:20px;");

    volumeSlider->setDefaultValue(toPixelsFromICA(0));

    volumeSlider->setMinimum(0);
    volumeSlider->setMaximum(299);

    volumeSlider->setSingleStep(1);
    volumeSlider->setPageStep(1);
    volumeSlider->setEnabled(volEditable);
    volumeSlider->setFocusPolicy(Qt::StrongFocus);
    volumeSlider->setValue(volumeSlider->minimum());
    volumeSlider->setMaximumHeight(320); // 300 pixels of usable space on OSX
    volumeSlider->setMinimumHeight(320);

    double unpaddedMin = volMin *
                         dbConversionFactor;
    double unpaddedMax = volMax *
                         dbConversionFactor;

    QString unpaddedMinStr = QString::number(unpaddedMin);
    QString unpaddedMaxStr = QString::number(unpaddedMax);

    volumeSlider->setStatusTip("Max: " + (unpaddedMinStr) + tr(" dB") + "\n" +
                               "Min: " + (unpaddedMaxStr) + tr(" dB"));

    QWidget *scaleWidget = new QWidget();
    scaleWidget->setMinimumWidth(14);
    scaleWidget->setMaximumWidth(14);
    scaleWidget->setMinimumHeight(320);
    scaleWidget->setMaximumHeight(320);
    scaleWidget->setObjectName("scaleWidget");
    scaleWidget->setStyleSheet("border: none; background-image: url(:/Meters/Images/scale.png);");

    currentVolumeLabel = new QClickyDbLabel(this);

    auto volFooter = new QLabel("dB");
#ifdef _WIN32
    volFooter->setStyleSheet("font-size: 7pt; font-family:Consolas,Monaco,Lucida Console,Liberation Mono,DejaVu Sans Mono,Bitstream Vera Sans Mono,Courier New, monospace;");
#else
    volFooter->setStyleSheet("font-size: 9pt; font-family:Consolas,Monaco,Lucida Console,Liberation Mono,DejaVu Sans Mono,Bitstream Vera Sans Mono,Courier New, monospace;");
#endif
    volFooter->setAlignment(Qt::AlignCenter);
    volFooter->setMaximumWidth(20);
    volFooter->setMinimumWidth(20);

    meterBar1 = new QProgressBar();
    setProgressBarColor(meterBar1, -20);

    meterBar1->setTextVisible(false);
    meterBar1->setMinimumWidth(8);
    meterBar1->setMaximumWidth(8);

    meterBar1->setOrientation(Qt::Vertical);
    meterBar1->setMaximumHeight(300);
    meterBar1->setMinimumHeight(300);
    meterBar1->setTextDirection(QProgressBar::BottomToTop);

    meterBar1->setMaximum(299);
    meterBar1->setMinimum(0);

    auto meterBar1spacer1 = new QWidget();
    meterBar1spacer1->setMaximumHeight(10);
    meterBar1spacer1->setMinimumHeight(10);
    meterBar1spacer1->setContentsMargins(0,0,0,0);

    auto meterBar1spacer1layout = new QHBoxLayout();
    meterBar1spacer1layout->setContentsMargins(0,0,0,0);
    meterBar1spacer1layout->setSpacing(0);

    clippingPushButton1 = new QPushButton();
    clippingPushButton1->setEnabled(false);
    clippingPushButton1->setMaximumWidth(4);
    clippingPushButton1->setMinimumHeight(4);
    clippingPushButton1->setMaximumHeight(4);

    clippingPushButton1->setCheckable(true);
    clippingPushButton1->setChecked(false);
    clippingPushButton1->setStyleSheet(QString("QPushButton {border: 0px; border-radius: 0px; background-image: url(:/mixer_graphics/Images/mixer_graphics/clip_vert_off.png);} \
                                          QPushButton:checked { background-image: url(:/mixer_graphics/Images/mixer_graphics/clip_vert_on.png); }"));
    meterBar1spacer1layout->addWidget(clippingPushButton1);
    meterBar1spacer1layout->setAlignment(clippingPushButton1, Qt::AlignCenter);
    meterBar1spacer1->setLayout(meterBar1spacer1layout);

    auto meterBar1spacer2 = new QWidget();
    meterBar1spacer2->setMaximumHeight(10);
    meterBar1spacer2->setMinimumHeight(10);
    meterBar1spacer2->setUpdatesEnabled(false);

    QVBoxLayout* meterBar1BoxLayout = new QVBoxLayout();
    meterBar1BoxLayout->setSpacing(0);
    meterBar1BoxLayout->setContentsMargins(0,0,0,0);
    meterBar1BoxLayout->addWidget(meterBar1spacer1);
    meterBar1BoxLayout->addWidget(meterBar1);
    meterBar1BoxLayout->addWidget(meterBar1spacer2);
    QWidget* meterBar1Box = new QWidget();
    meterBar1Box->setContentsMargins(0,0,0,0);
    meterBar1Box->setLayout(meterBar1BoxLayout);

    QWidget* meterBar2Box = 0;

    meterBar2 = new QProgressBar();
    setProgressBarColor(meterBar2, -20);
    meterBar2->setTextVisible(false);
    meterBar2->setMinimumWidth(8);
    meterBar2->setMaximumWidth(8);

    meterBar2->setOrientation(Qt::Vertical);
    meterBar2->setMaximumHeight(300);
    meterBar2->setMinimumHeight(300);
    meterBar2->setTextDirection(QProgressBar::BottomToTop);

    meterBar2->setMaximum(299);
    meterBar2->setMinimum(0);

    auto meterBar2spacer1 = new QWidget();
    meterBar2spacer1->setMaximumHeight(10);
    meterBar2spacer1->setMinimumHeight(10);
    meterBar2spacer1->setContentsMargins(0,0,0,0);

    auto meterBar2spacer1layout = new QHBoxLayout();
    meterBar2spacer1layout->setContentsMargins(0,0,0,0);
    meterBar2spacer1layout->setSpacing(0);

    clippingPushButton2 = new QPushButton();
    clippingPushButton2->setEnabled(false);
    clippingPushButton2->setMaximumWidth(4);
    clippingPushButton2->setMinimumHeight(4);
    clippingPushButton2->setMaximumHeight(4);

    clippingPushButton2->setCheckable(true);
    clippingPushButton2->setChecked(false);
    clippingPushButton2->setStyleSheet(QString("QPushButton {border: 0px; border-radius: 0px; background-image: url(:/mixer_graphics/Images/mixer_graphics/clip_vert_off.png);} \
                                          QPushButton:checked { background-image: url(:/mixer_graphics/Images/mixer_graphics/clip_vert_on.png); }"));
    meterBar2spacer1layout->addWidget(clippingPushButton2);
    meterBar2spacer1layout->setAlignment(clippingPushButton2, Qt::AlignCenter);
    meterBar2spacer1->setLayout(meterBar2spacer1layout);

    auto meterBar2spacer2 = new QWidget();
    meterBar2spacer2->setMaximumHeight(10);
    meterBar2spacer2->setMinimumHeight(10);
    meterBar2spacer2->setUpdatesEnabled(false);

    QVBoxLayout* meterBar2BoxLayout = new QVBoxLayout();
    meterBar2BoxLayout->setSpacing(0);
    meterBar2BoxLayout->setContentsMargins(0,0,0,0);
    meterBar2BoxLayout->addWidget(meterBar2spacer1);
    meterBar2BoxLayout->addWidget(meterBar2);
    meterBar2BoxLayout->addWidget(meterBar2spacer2);
    meterBar2Box = new QWidget();
    meterBar2Box->setContentsMargins(0,0,0,0);
    meterBar2Box->setLayout(meterBar2BoxLayout);

    QVBoxLayout *volBoxLayout = new QVBoxLayout();
    QHBoxLayout *volBoxLayoutTop = new QHBoxLayout();
    QHBoxLayout *volBoxLayoutBottom = new QHBoxLayout();

    QWidget *volBox = new QWidget();
    QWidget *volBoxTop = new QWidget();
    QWidget *volBoxBottom = new QWidget();

    volBoxLayout->setSpacing(0);
    volBoxLayout->setContentsMargins(0,0,0,0);
    volBoxLayoutTop->setSpacing(0);
    volBoxLayoutTop->setContentsMargins(0,0,0,0);
    volBoxLayoutBottom->setSpacing(0);
    volBoxLayoutBottom->setContentsMargins(0,0,0,0);

    volBox->setContentsMargins(0,0,0,0);
    volBoxTop->setContentsMargins(0,0,0,0);
    volBoxBottom->setContentsMargins(0,0,0,0);

    volBoxLayoutTop->addWidget(volumeSlider);
    volBoxLayoutTop->addWidget(meterBar1Box);
    if (meterBar2Box)
      volBoxLayoutTop->addWidget(meterBar2Box);
    volBoxLayoutTop->addWidget(scaleWidget);
    volBoxTop->setLayout(volBoxLayoutTop);

    volBoxLayoutBottom->addWidget(currentVolumeLabel);
    volBoxLayoutBottom->addWidget(volFooter);
    volBoxBottom->setLayout(volBoxLayoutBottom);

    volBoxLayout->addWidget(volBoxTop);
    volBoxLayout->addWidget(volBoxBottom);
    volBox->setLayout(volBoxLayout);

    layout()->addWidget(volBox);
    layout()->setAlignment(volBox, Qt::AlignHCenter);

    QWidget* spacer2Widget = new QWidget();
    spacer2Widget->setMinimumHeight(2);
    spacer2Widget->setMaximumHeight(2);
    spacer2Widget->setUpdatesEnabled(false);
    layout()->addWidget(spacer2Widget);

    updateVolumeValue();

    connect(volumeSlider, SIGNAL(valueChanged(int)), this,
            SLOT(volumeSliderChanged(int)));
  }
}

void MixerChannelWidget::buildPanDial()
{
  int panMin = 0;
  int panMax = 0;
  bool panEditable = false;
  bool panAvailable = false;
  if (mixerType == in) {
    panMin = mixerInputInterface->panMin(audioPortID);
    panMax = mixerInputInterface->panMax(audioPortID);
    panAvailable = mixerInputInterface->isPanAvailable(audioPortID);
    panEditable = mixerInputInterface->isPanEditable(audioPortID);
  }
  else {
    panMin = mixerOutputInterface->panMin(audioPortID);
    panMax = mixerOutputInterface->panMax(audioPortID);
    panAvailable = mixerOutputInterface->isPanAvailable(audioPortID);
    panEditable = mixerOutputInterface->isPanEditable(audioPortID);
  }

  if (panAvailable) {
    panDial = new QClickyDial();
    panDial->setStyleSheet("QClickyDial {background-color: #65ceff};");


    panDial->setMinimum(panMin);
    panDial->setMaximum(panMax);
    panDial->setSingleStep(10);
    panDial->setPageStep(10);
/*    panDial->setMaximumWidth(50);
    panDial->setMinimumWidth(50);*/
    panDial->setNotchesVisible(true);

    panDial->setEnabled(panEditable);
    panDial->setFocusPolicy(Qt::StrongFocus);
    auto vBoxLayoutPan = new QVBoxLayout();
    panDial->setStatusTip("Max: " + QString::number(panMax) + tr("") + "\n" +
                          "Min: " + QString::number(panMin) + tr(""));

    currentPanLabel = new QLabel();
    currentPanLabel->setAlignment(Qt::AlignHCenter);
    currentPanLabel->setStyleSheet("font-size: 9pt; font-family:Consolas,Monaco,Lucida Console,Liberation Mono,DejaVu Sans Mono,Bitstream Vera Sans Mono,Courier New, monospace;");

    auto panLabelButton = new QPushButton();
    if (stereoLinked)
      panLabelButton->setText(tr("BAL"));
    else
      panLabelButton->setText(tr("PAN"));
    panLabelButton->setEnabled(false);
    panLabelButton->setMinimumWidth(50);
    panLabelButton->setMaximumWidth(50);

    QHBoxLayout *lay = new QHBoxLayout();
    QWidget *wid = new QWidget();
    lay->addWidget(panLabelButton); lay->setMargin(0);
    wid->setLayout(lay); wid->setContentsMargins(0,0,0,0);
    layout()->addWidget(wid);
    layout()->setAlignment(wid, Qt::AlignRight);

    panLabelButton->setStyleSheet(QString("QPushButton { font-size: 10pt; color:#ffffff; padding:3px; border: 0px; } \
                                          QPushButton:pressed { color: #ffffff;  }"));

    vBoxLayoutPan->setSpacing(0);
    vBoxLayoutPan->setContentsMargins(0, 0, 0, 0);
    vBoxLayoutPan->addWidget(wid);
    vBoxLayoutPan->addWidget(panDial);
    vBoxLayoutPan->addWidget(currentPanLabel);
    vBoxLayoutPan->setAlignment(panDial, Qt::AlignCenter);
    auto vBoxWidgetPan = new QWidget();
    vBoxWidgetPan->setMinimumWidth(50);
    vBoxWidgetPan->setMaximumWidth(50);
    vBoxWidgetPan->setMinimumHeight(80);
    vBoxWidgetPan->setMaximumHeight(80);

    vBoxWidgetPan->setLayout(vBoxLayoutPan);

    panBoxLayout->addWidget(vBoxWidgetPan);
    if (mixerType == in) {
      panBox->setLayout(panBoxLayout);
      layout()->addWidget(panBox);
    }
    connect(panDial, SIGNAL(valueChanged(int)), this,
            SLOT(panDialChanged(int)));

    updatePanValue();
  }
}

void MixerChannelWidget::buildSoloDial()
{
  int soloMin = 0;
  int soloMax = 0;
  int soloRes = 0;
  bool soloEditable = false;
  bool soloAvailable = false;
  if (mixerType == out) {
    soloMin = mixerOutputInterface->volumeMin(audioPortID);
    soloMax = mixerOutputInterface->volumeMax(audioPortID);
    soloRes = mixerOutputInterface->volumeResolution(audioPortID);
    soloAvailable = mixerOutputInterface->isSoloAvailable(audioPortID);
    soloEditable = mixerOutputInterface->isSoloEditable(audioPortID);
  }

  if (soloAvailable) {
    soloDial = new QClickyDial();

    soloDial->setStyleSheet("QDial {background-color: #ff9b26};");

    soloDial->setMinimum(soloMin*
                         dbConversionFactor/
                         (soloRes*
                          dbConversionFactor));
    soloDial->setMaximum(soloMax*
                         dbConversionFactor/
                         (soloRes*
                          dbConversionFactor));

    soloDial->setSingleStep(soloRes);
    soloDial->setPageStep(soloRes);

    soloDial->setNotchesVisible(true);
    soloDial->setMinimumWidth(50);
    soloDial->setMaximumWidth(50);

    soloDial->setEnabled(soloEditable);
    soloDial->setFocusPolicy(Qt::StrongFocus);
    auto vBoxLayoutSolo = new QVBoxLayout();

    double unpaddedMin = soloMin *
                         dbConversionFactor;
    double unpaddedMax = soloMax *
                         dbConversionFactor;

    QString unpaddedMinStr = QString::number(unpaddedMin);
    QString unpaddedMaxStr = QString::number(unpaddedMax);

    soloDial->setStatusTip("Max: " + (unpaddedMinStr) + tr(" dB") + "\n" +
                           "Min: " + (unpaddedMaxStr) + tr(" dB"));


    soloLightPushButton = new QPushButton();
    soloLightPushButton->setText(tr("SOLO"));
    soloLightPushButton->setEnabled(false);
    soloLightPushButton->setMinimumWidth(60);
    soloLightPushButton->setMaximumWidth(60);

    QHBoxLayout *lay = new QHBoxLayout();
    QWidget *wid = new QWidget();
    lay->addWidget(soloLightPushButton); lay->setMargin(0);
    wid->setLayout(lay); wid->setContentsMargins(0,0,0,0);
    layout()->addWidget(wid);
    layout()->setAlignment(wid, Qt::AlignRight);

    soloLightPushButton->setCheckable(true);
    soloLightPushButton->setStyleSheet(QString("QPushButton { font-size: 10pt; color:#ffffff; padding:3px; border: 0px; } \
                                          QPushButton:checked { color: #DC143C;  } \
                                          QPushButton:pressed { color: #DC143C;  }"));

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateSoloLightPushButtonValue()));
    timer->start(500);

    currentSoloLabel = new QLabel();
    currentSoloLabel->setAlignment(Qt::AlignHCenter);
    currentSoloLabel->setStyleSheet("font-size: 9pt; font-family:Consolas,Monaco,Lucida Console,Liberation Mono,DejaVu Sans Mono,Bitstream Vera Sans Mono,Courier New, monospace;");

    vBoxLayoutSolo->setSpacing(0);
    vBoxLayoutSolo->setContentsMargins(0, 0, 0, 0);
    vBoxLayoutSolo->addWidget(wid);
    vBoxLayoutSolo->addWidget(soloDial);
    vBoxLayoutSolo->addWidget(currentSoloLabel);
    vBoxLayoutSolo->setAlignment(soloDial, Qt::AlignCenter);

    auto vBoxWidgetSolo = new QWidget();
    vBoxWidgetSolo->setLayout(vBoxLayoutSolo);

    vBoxWidgetSolo->setMinimumWidth(60);
    vBoxWidgetSolo->setMaximumWidth(60);
    vBoxWidgetSolo->setMinimumHeight(80);
    vBoxWidgetSolo->setMaximumHeight(80);

    //panBoxLayout->addWidget(stereoLinkPushButton);
    panBoxLayout->addWidget(vBoxWidgetSolo);
    panBoxLayout->setAlignment(vBoxWidgetSolo, Qt::AlignHCenter | Qt::AlignTop);
    panBox->setLayout(panBoxLayout);
    panBox->setMinimumWidth(120);
    panBox->setMaximumWidth(120);
    layout()->addWidget(panBox);

    connect(soloDial, SIGNAL(valueChanged(int)), this,
            SLOT(soloDialChanged(int)));

    updateSoloDialValue();
  }
}

void MixerChannelWidget::buildMutePushButton()
{
  bool muteAvailable = false;
  bool muteEditable = false;

  if (mixerType == in) {
    muteAvailable = mixerInputInterface->isMuteAvailable(audioPortID);
    muteEditable = mixerInputInterface->isMuteEditable(audioPortID);
  }
  else {
    muteAvailable = mixerOutputInterface->isMuteAvailable(audioPortID);
    muteEditable = mixerOutputInterface->isMuteEditable(audioPortID);
  }

  if (muteAvailable) {
    mutePushButton = new QPushButton();
    //mutePushButton->setText(tr("Mute"));
    mutePushButton->setEnabled(muteEditable);
    if ((mixerType == in) && (!stereoLinked)) {
      mutePushButton->setMinimumWidth(49);
      mutePushButton->setMaximumWidth(49);
      mutePushButton->setStyleSheet(QString("QPushButton {background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_mute_off.png); border: 0px;} \
                                            QPushButton:checked { background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_mute_on.png); }"));
    }
    else {
      mutePushButton->setMinimumWidth(62);
      mutePushButton->setMaximumWidth(62);
      mutePushButton->setStyleSheet(QString("QPushButton {margin: 0px; padding: 0px; spacing: 0px; background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_mute_link_off.png); border: 0px;} \
                                            QPushButton:checked { background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_mute_link_on.png); }"));
    }

    mutePushButton->setMinimumHeight(21);
    mutePushButton->setMaximumHeight(21);

    QHBoxLayout *lay = new QHBoxLayout();
    QWidget *wid = new QWidget();
    lay->addWidget(mutePushButton); lay->setMargin(0);
    wid->setLayout(lay); wid->setContentsMargins(0,0,0,0);
    layout()->addWidget(wid);
    layout()->setAlignment(wid, Qt::AlignCenter);

    mutePushButton->setCheckable(true);

    updateMuteValue();

    connect(mutePushButton, SIGNAL(toggled(bool)), this,
            SLOT(muteStateChanged(bool)));
  }
}

void MixerChannelWidget::buildSoloPushButton()
{
  bool soloAvailable = false;
  bool soloEditable = false;
  if (mixerType == in) {
    soloAvailable = mixerInputInterface->isSoloAvailable(audioPortID);
    soloEditable = mixerInputInterface->isSoloEditable(audioPortID);
  }
  else {
    soloAvailable = mixerOutputInterface->isSoloAvailable(audioPortID);
    soloEditable = mixerOutputInterface->isSoloEditable(audioPortID);
  }
  if (soloAvailable) {
    soloPushButton = new QPushButton();
    soloPushButton->setEnabled(soloEditable);
    if ((mixerType == in) && (!stereoLinked)) {
      soloPushButton->setMinimumWidth(49);
      soloPushButton->setMaximumWidth(49);
      soloPushButton->setStyleSheet(QString("QPushButton {background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_solo_off.png); border: 0px;} \
                                            QPushButton:checked { background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_solo_on.png); }"));
    }
    else {
      soloPushButton->setMinimumWidth(62);
      soloPushButton->setMaximumWidth(62);
      soloPushButton->setStyleSheet(QString("QPushButton {background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_solo_link_off.png); border: 0px;} \
                                            QPushButton:checked { background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_solo_link_on.png); }"));
    }

    soloPushButton->setMinimumHeight(21);
    soloPushButton->setMaximumHeight(21);


    QHBoxLayout *lay = new QHBoxLayout();
    QWidget *wid = new QWidget();
    lay->addWidget(soloPushButton); lay->setMargin(0);
    wid->setLayout(lay); wid->setContentsMargins(0,0,0,0);
    layout()->addWidget(wid);
    layout()->setAlignment(wid, Qt::AlignCenter);

    soloPushButton->setCheckable(true);

    updateSoloPushButtonValue();

    connect(soloPushButton, SIGNAL(toggled(bool)), this,
      SLOT(soloStateChanged(bool)));
  }
}

void MixerChannelWidget::buildSoloPFLPushButton()
{
  bool soloPFLAvailable = false;
  bool soloPFLEditable = false;
  if (mixerType == in) {
    soloPFLAvailable = mixerInputInterface->isSoloPFLAvailable(audioPortID);
    soloPFLEditable = mixerInputInterface->isSoloPFLEditable(audioPortID);
  }
  else {
    soloPFLAvailable = mixerOutputInterface->isSoloPFLAvailable(audioPortID);
    soloPFLEditable = mixerOutputInterface->isSoloPFLEditable(audioPortID);
  }

  if (soloPFLAvailable) {
    soloPFLPushButton = new QPushButton();
    soloPFLPushButton->setEnabled(soloPFLEditable);
    if ((mixerType == in) && (!stereoLinked)) {
      soloPFLPushButton->setMinimumWidth(49);
      soloPFLPushButton->setMaximumWidth(49);
      soloPFLPushButton->setStyleSheet(QString("QPushButton {background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_pfl_off.png); border: 0px;} \
                                            QPushButton:checked { background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_pfl_on.png); }"));
    }
    else {
      soloPFLPushButton->setMinimumWidth(62);
      soloPFLPushButton->setMaximumWidth(62);
      soloPFLPushButton->setStyleSheet(QString("QPushButton {background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_pfl_link_off.png); border: 0px;} \
                                            QPushButton:checked { background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_pfl_link_on.png); }"));
    }

    soloPFLPushButton->setMinimumHeight(21);
    soloPFLPushButton->setMaximumHeight(21);

    QHBoxLayout *lay = new QHBoxLayout();
    QWidget *wid = new QWidget();
    lay->addWidget(soloPFLPushButton); lay->setMargin(0);
    wid->setLayout(lay); wid->setContentsMargins(0,0,0,0);
    layout()->addWidget(wid);
    layout()->setAlignment(wid, Qt::AlignCenter);

    soloPFLPushButton->setCheckable(true);

    updateSoloPFLValue();

    connect(soloPFLPushButton, SIGNAL(toggled(bool)), this,
            SLOT(soloPFLStateChanged(bool)));
  }
}

void MixerChannelWidget::buildInvertPushButton()
{
  bool invertAvailable = false;
  bool invertEditable = false;
  if (mixerType == in) {
    invertAvailable = mixerInputInterface->isInvertAvailable(audioPortID);
    invertEditable = mixerInputInterface->isInvertEditable(audioPortID);
  }
  else {
    invertAvailable = mixerOutputInterface->isInvertAvailable(audioPortID);
    invertEditable = mixerOutputInterface->isInvertEditable(audioPortID);
  }

  if (invertAvailable) {

    invertPushButton = new QPushButton();

    invertPushButton->setEnabled(invertEditable);
    if ((mixerType == in) && (!stereoLinked)) {
      invertPushButton->setMinimumWidth(24);
      invertPushButton->setMaximumWidth(24);
      invertPushButton->setStyleSheet(QString("QPushButton {background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_phase_left_off.png); border: 0px;} \
                                            QPushButton:checked { background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_phase_left_on.png); }"));
    }
    else {
      invertPushButton->setMinimumWidth(31);
      invertPushButton->setMaximumWidth(31);
      invertPushButton->setStyleSheet(QString("QPushButton {background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_phase_left_link_off.png); border: 0px;} \
                                            QPushButton:checked { background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_phase_left_link_on.png); }"));
    }


    invertPushButton->setMinimumHeight(21);
    invertPushButton->setMaximumHeight(21);

    QHBoxLayout *lay = new QHBoxLayout();
    QWidget *wid = new QWidget();
    lay->addWidget(invertPushButton); lay->setMargin(0);
    wid->setLayout(lay); wid->setContentsMargins(0,0,0,0);
    layout()->addWidget(wid);
    layout()->setAlignment(wid, Qt::AlignCenter);

    invertBoxLayout->addWidget(wid);

    invertPushButton->setCheckable(true);
    updateInvertValue();

    connect(invertPushButton, SIGNAL(toggled(bool)), this,
            SLOT(invertStateChanged(bool)));
  }
}

void MixerChannelWidget::buildRightInvertPushButton()
{
  bool invertAvailable = false;
  bool invertEditable = false;
  if (mixerType == in) {
    invertAvailable = mixerInputInterface->isInvertAvailable(audioPortID);
    invertEditable = mixerInputInterface->isInvertEditable(audioPortID);
  }
  else {
    invertAvailable = mixerOutputInterface->isInvertAvailable(audioPortID);
    invertEditable = mixerOutputInterface->isInvertEditable(audioPortID);
  }

  if (invertAvailable) {

    rightInvertPushButton = new QPushButton();
    rightInvertPushButton->setEnabled(invertEditable);
    if ((mixerType == in) && (!stereoLinked)) {
      invertBox->setMaximumWidth(50);
      invertBox->setMinimumWidth(50);
      rightInvertPushButton->setMinimumWidth(25);
      rightInvertPushButton->setMaximumWidth(25);
      rightInvertPushButton->setStyleSheet(QString("QPushButton {background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_phase_right_off.png); border: 0px;} \
                                            QPushButton:checked { background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_phase_right_on.png); }"));
    }
    else {
      invertBox->setMaximumWidth(62);
      invertBox->setMinimumWidth(62);
      rightInvertPushButton->setMinimumWidth(31);
      rightInvertPushButton->setMaximumWidth(31);
      rightInvertPushButton->setStyleSheet(QString("QPushButton {background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_phase_right_link_off.png); border: 0px;} \
                                            QPushButton:checked { background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_phase_right_link_on.png); }"));
    }

    rightInvertPushButton->setMinimumHeight(21);
    rightInvertPushButton->setMaximumHeight(21);


    QHBoxLayout *lay = new QHBoxLayout();
    QWidget *wid = new QWidget();
    lay->addWidget(rightInvertPushButton); lay->setMargin(0);
    wid->setLayout(lay); wid->setContentsMargins(0,0,0,0);
    layout()->addWidget(wid);
    layout()->setAlignment(wid, Qt::AlignCenter);

    invertBoxLayout->addWidget(wid);

    invertBoxLayout->setMargin(0);
    invertBox->setLayout(invertBoxLayout);

    rightInvertPushButton->setCheckable(true);
    invertBox->setStyleSheet(QString("QWidget { padding:0px; margin: 0px;}"));
    layout()->addWidget(invertBox);
    layout()->setAlignment(invertBox, Qt::AlignCenter);

    invertBox->setContentsMargins(0, 0, 0, 0);

    updateRightInvertValue();

    connect(rightInvertPushButton, SIGNAL(toggled(bool)), this,
            SLOT(rightInvertStateChanged(bool)));
  }
}


void MixerChannelWidget::updateAll()
{
  static int count = 0;

  if (count == 0)
    updateChannelNameLabel();
//  updateVolumeValue();
//  updatePanValue();
//  updateMuteValue();
//  updateInvertValue();
//  if (stereoLinked || outputStereoLinked)
//    updateRightInvertValue();
//  if (mixerType == in)
//    updateSoloPushButtonValue();
//  else
//    updateSoloDialValue();
//  updateSoloPFLValue();
//  if (mixerType == in && (mixerInputNumber % 2))
//    updateStereoLinkValue();
  updateMeters();
  count++;
  if (count >= 10)
    count = 0;
}

void MixerChannelWidget::updateMeters() {
  bool volAvailable = false;
  int meterCurrent1 = 0;
  int meterCurrent2 = 0;
  if (!disabled) {
    if (mixerType == in) {
      volAvailable = mixerInputInterface->isVolumeAvailable(audioPortID);
      meterCurrent1 = mixerInputInterface->meterCurrent(audioPortID, mixerOutputNumber, mixerInputNumber);
      if (stereoLinked)
        meterCurrent2 = mixerInputInterface->meterCurrent(audioPortID, mixerOutputNumber + 1, mixerInputNumber + 1);
      else
        meterCurrent2 = mixerInputInterface->meterCurrent(audioPortID, mixerOutputNumber + 1, mixerInputNumber);
    }
    else {
      volAvailable = mixerOutputInterface->isVolumeAvailable(audioPortID);
      meterCurrent1 = mixerOutputInterface->meterCurrent(audioPortID, mixerOutputNumber);
      meterCurrent2 = mixerOutputInterface->meterCurrent(audioPortID, mixerOutputNumber + 1);
    }
    if (volAvailable) {
      int meterPng1 = toPixelsFromICA(((double)20 * log10((double)meterCurrent1 / 8192.0)) * 256.0);
      int meterPng2 = toPixelsFromICA(((double)20 * log10((double)meterCurrent2 / 8192.0)) * 256.0);

      if ((((double)20 * log10((double)meterCurrent1 / 8192.0)) >= 0.0)) {
        turnOnClipping1();
      }
      if ((((double)20 * log10((double)meterCurrent2 / 8192.0)) >= 0.0)) {
        turnOnClipping2();
      }

//      //printf("meter1: %d, %d, %d | meter2: %d, %d, %d\n", meterPng1, meterCurrent1, (int)(((double)20 * log10((double)meterCurrent1 / 8192.0)) * 256.0),
//             meterPng2, meterCurrent2, (int)(((double)20 * log10((double)meterCurrent2 / 8192.0)) * 256.0));

      if (meterCurrent1 == 0) {
        meterBar1->setValue(0);
        setProgressBarColor(meterBar1, -20);
      }
      else {
        meterBar1->setValue(meterPng1);
        setProgressBarColor(meterBar1, ((double)20 * log10((double)meterCurrent1 / 8192.0)));
      }
      if (meterCurrent2 == 0) {
        meterBar2->setValue(0);
        setProgressBarColor(meterBar2, -20);
      }
      else {
        meterBar2->setValue(meterPng2);
        setProgressBarColor(meterBar2, ((double)20 * log10((double)meterCurrent2 / 8192.0)));
      }
    }
  }
}

void MixerChannelWidget::updateChannelNameLabel()
{
  if (mixerType == in) {
    bool isAssigned = false;
    bool isAssigned2 = false;
    QString title = "";
    QString title2 = "";
    for (int i=0; i< allAvailableInChannels.size(); i++) {
      if (mixerInterface->audioPortIDForInput(audioPortID,mixerInputNumber) == allAvailableInChannels.at(i).first &&
          mixerInterface->channelIDForInput(audioPortID,mixerInputNumber) == allAvailableInChannels.at(i).second) {
        title = (portNames.at(allAvailableInChannels.at(i).first - 1) + " : "
                                 + QString::number(allAvailableInChannels.at(i).second));
        isAssigned = true;
      }
      if (stereoLinked) {
        if (mixerInterface->audioPortIDForInput(audioPortID,mixerInputNumber + 1) == allAvailableInChannels.at(i).first &&
            mixerInterface->channelIDForInput(audioPortID,mixerInputNumber + 1) == allAvailableInChannels.at(i).second) {
          title2 = (portNames.at(allAvailableInChannels.at(i).first - 1) + " : "
                                   + QString::number(allAvailableInChannels.at(i).second));
          isAssigned2 = true;
        }
      }
    }
    if (!isAssigned) {
      title = ("No SRC");
    }
    if (!isAssigned2) {
      title2 = ("No SRC");
    }

    if (stereoLinked)
      outputConfigPushButton->setText(title + " | " + title2);
    else
      outputConfigPushButton->setText(title);
  }
  else {
    QString title = "";
    QString title2 = "";
    if (mixerInterface->channelIDsForOutput(audioPortID,mixerOutputNumber).size() == 0) {
      title = "No DST";
    }
    else if (mixerInterface->channelIDsForOutput(audioPortID,mixerOutputNumber).size() > 1) {
      title = "Multiple";
    }
    if (mixerInterface->channelIDsForOutput(audioPortID,mixerOutputNumber + 1).size() == 0) {
      title2 = "No DST";
    }
    else if (mixerInterface->channelIDsForOutput(audioPortID,mixerOutputNumber + 1).size() > 1) {
      title2 = "Multiple";
    }

    if (title == "" || title2 == "") {
      for (int i=0; i< allAvailableInChannels.size(); i++) {
        if (mixerInterface->channelIDsForOutput(audioPortID,mixerOutputNumber).size() == 1)
        {
          if (title == "") {
            if (audioPortID == allAvailableInChannels.at(i).first && mixerInterface->channelIDsForOutput(audioPortID,mixerOutputNumber).at(0) == allAvailableInChannels.at(i).second) {
              title = (portNames.at(allAvailableInChannels.at(i).first - 1) + " : "
                       + QString::number(allAvailableInChannels.at(i).second));
            }
          }
        }
        if (stereoLinked) {
          if (mixerInterface->channelIDsForOutput(audioPortID,mixerOutputNumber + 1).size() == 1)
          {
            if (title2 == "") {
              if (audioPortID == allAvailableInChannels.at(i).first && mixerInterface->channelIDsForOutput(audioPortID,mixerOutputNumber + 1).at(0) == allAvailableInChannels.at(i).second) {
                title2 = (portNames.at(allAvailableInChannels.at(i).first - 1) + " : "
                         + QString::number(allAvailableInChannels.at(i).second));
              }
            }
          }
        }
      }
    }

    outputConfigPushButton->setText(title + " | " + title2);
  }
}

void MixerChannelWidget::updateVolumeValue()
{
  if (!disabled) {
    bool volAvailable = false;
    int volCurrent = 0;
    int volRes = 0;
    if (mixerType == in) {
      volAvailable = mixerInputInterface->isVolumeAvailable(audioPortID);
      volCurrent = mixerInputInterface->volumeCurrent(audioPortID, mixerOutputNumber, mixerInputNumber);
      volRes = mixerOutputInterface->volumeResolution(audioPortID);
    }
    else {
      volAvailable = mixerOutputInterface->isVolumeAvailable(audioPortID);
      volCurrent = mixerOutputInterface->volumeCurrent(audioPortID, mixerOutputNumber);
      volRes = mixerOutputInterface->volumeResolution(audioPortID);
    }

    volumeSlider->blockSignals(true);
    if (volAvailable) {
      if (!((volumeSlider->value() <= toPixelsFromICA(volCurrent)+1) &&
            (volumeSlider->value() >= toPixelsFromICA(volCurrent)-1))){
        volumeSlider->setValue(toPixelsFromICA(volCurrent));
      }
      currentVolumeLabel->setValue(volCurrent * dbConversionFactor);
      if (((uint16_t)volCurrent) == 0x8000) {
        currentVolumeLabel->setValue(-100000);
      }
    } else {
      if (volumeSlider->value()!=0) {
        volumeSlider->setValue(0);
        currentVolumeLabel->setValue(-100001);
      }
    }
    volumeSlider->blockSignals(false);
  }
}

void MixerChannelWidget::updatePanValue()
{
  if (!disabled) {
    bool panAvailable = false;
    int panCurrent = 0;
    int panMax = 0;
    if (mixerType == in) {
      panAvailable = mixerInputInterface->isPanAvailable(audioPortID);
      panCurrent = mixerInputInterface->panCurrent(audioPortID, mixerOutputNumber, mixerInputNumber);
      panMax = mixerInputInterface->panMax(audioPortID);
    }
    else {
      panAvailable = mixerOutputInterface->isPanAvailable(audioPortID);
      panCurrent = mixerOutputInterface->panCurrent(audioPortID, mixerOutputNumber);
      panMax = mixerOutputInterface->panMax(audioPortID);
    }
    panDial->blockSignals(true);
    if (panAvailable) {
      if (panDial->value() != panCurrent) {
        panDial->setValue(panCurrent);
      }
      QString toDisplay;
      if (panCurrent == 0) {
        toDisplay = QString("C");
      }
      else if (panCurrent < 0) {
        toDisplay = QString("L ") + QString::number(abs(panCurrent)).rightJustified(3, ' ').replace(" ", "&nbsp;");
      }
      else {
        toDisplay = QString("R ") + QString::number(panCurrent).rightJustified(3, ' ').replace(" ", "&nbsp;");
      }
      currentPanLabel->setText("<b>" + toDisplay + "</b>");
    }
    else {
      if (panDial->value()!=0) {
        panDial->setValue(0);
        currentPanLabel->setText("<b>N/A</b>");
      }
    }
    panDial->blockSignals(false);
  }
}

void MixerChannelWidget::updateMuteValue()
{
  if (!disabled) {
    bool mute = false;

    if (mixerType == in) {
      mute = mixerInputInterface->mute(audioPortID,mixerOutputNumber,mixerInputNumber);
    }
    else {
      mute = mixerOutputInterface->mute(audioPortID,mixerOutputNumber);
    }

    if (mutePushButton)
      mutePushButton->setChecked(mute);
  }
}

void MixerChannelWidget::updateStereoLinkValue()
{/*
  bool stereoLink = false;

  if (mixerType == in) {
    stereoLink = mixerInputInterface->stereoLink(audioPortID,mixerOutputNumber,mixerInputNumber);
  }
  else {
    stereoLink = mixerOutputInterface->stereoLink(audioPortID,mixerOutputNumber);
  }

  if (stereoLinkPushButton)
    stereoLinkPushButton->setChecked(stereoLink);*/

}

void MixerChannelWidget::updateSoloPushButtonValue()
{
  if (!disabled) {
    bool solo = false;

    if (mixerType == in) {
      solo = mixerInputInterface->solo(audioPortID,mixerOutputNumber,mixerInputNumber);
    }

    if (soloPushButton)
      soloPushButton->setChecked(solo);
  }
}

void MixerChannelWidget::updateSoloLightPushButtonValue()
{
  if (soloLightOn) {
    soloLightOn = false;
  }
  else {
    for (int inCh = 1; inCh <= mixerInterface->numberInputs(audioPortID); ++inCh) {
      if ((mixerInterface->audioPortIDForInput(audioPortID,inCh) != 0) &&
          (mixerInterface->channelIDForInput(audioPortID, inCh))){
        if (mixerInputInterface->solo(audioPortID, mixerOutputNumber, inCh)) {
          soloLightOn = true;
          break;
        }
      }
    }
  }
  soloLightPushButton->setChecked(soloLightOn);
}

void MixerChannelWidget::turnOnClipping1()
{
  clippingPushButton1->setChecked(true);
  clippingTimer1->stop();
  clippingTimer1->setSingleShot(true);
  clippingTimer1->setInterval(1000);
  clippingTimer1->start();
}

void MixerChannelWidget::turnOffClipping1()
{
  clippingPushButton1->setChecked(false);
}

void MixerChannelWidget::turnOnClipping2()
{
  clippingPushButton2->setChecked(true);
  clippingTimer2->stop();
  clippingTimer2->setSingleShot(true);
  clippingTimer2->setInterval(1000);
  clippingTimer2->start();
}

void MixerChannelWidget::turnOffClipping2()
{
  clippingPushButton2->setChecked(false);
}

void MixerChannelWidget::updateSoloDialValue()
{
  if (!disabled) {
    bool soloAvailable = false;
    int soloCurrent = 0;
    int soloRes = 0;
    if (mixerType == out) {
      soloAvailable = mixerOutputInterface->isSoloAvailable(audioPortID);
      soloCurrent = mixerOutputInterface->solo(audioPortID, mixerOutputNumber);
      soloRes = mixerOutputInterface->volumeResolution(audioPortID);
    }
    soloDial->blockSignals(true);
    if (soloAvailable) {
      if (soloDial->value() != soloCurrent * dbConversionFactor/ (soloRes* dbConversionFactor)) {
        soloDial->setValue(soloCurrent * dbConversionFactor/
                           (soloRes* dbConversionFactor));
      }
      currentSoloLabel->setText("<b>" +QString::number(soloCurrent * dbConversionFactor,'f',1).rightJustified(5, ' ').replace(" ", "&nbsp;") + " dB</b>");
      if (((uint16_t)soloCurrent) == 0x8000) {
        currentSoloLabel->setText("<b>" + QString("- &infin;").rightJustified(5, ' ').replace(" ", "&nbsp;") +" dB</b>");
      }
    }
    else {
      if (soloDial->value()!=0) {
        soloDial->setValue(0);
        currentSoloLabel->setText("<b>" + QString("N/A").rightJustified(5, ' ').replace(" ", "&nbsp;") + "</b>");
      }
    }
    soloDial->blockSignals(false);
  }
}

void MixerChannelWidget::updateSoloPFLValue()
{
  if (!disabled) {
    bool soloPFL = false;

    if (mixerType == in) {
      soloPFL = mixerInputInterface->soloPFL(audioPortID,mixerOutputNumber,mixerInputNumber);
    }
    else {
      soloPFL = mixerOutputInterface->soloPFL(audioPortID,mixerOutputNumber);
    }

    if (soloPFLPushButton)
      soloPFLPushButton->setChecked(soloPFL);
  }
}

void MixerChannelWidget::updateInvertValue()
{
  if (!disabled) {
    bool invert = false;

    if (mixerType == in) {
      invert = mixerInputInterface->invert(audioPortID,mixerOutputNumber,mixerInputNumber);
    }
    else {
      invert = mixerOutputInterface->invert(audioPortID,mixerOutputNumber);
    }

    if (invertPushButton)
      invertPushButton->setChecked(invert);
  }
}

void MixerChannelWidget::updateRightInvertValue()
{
  if (!disabled) {
    bool invert = false;

    if (mixerType == in) {
      if (!stereoLinked && outputStereoLinked)
        invert = mixerInputInterface->invert(audioPortID,mixerOutputNumber + 1,mixerInputNumber);
      else if (stereoLinked && !outputStereoLinked)
        invert = mixerInputInterface->invert(audioPortID,mixerOutputNumber,mixerInputNumber + 1);
      else if (stereoLinked && outputStereoLinked) {
        invert = mixerInputInterface->invert(audioPortID,mixerOutputNumber+1,mixerInputNumber + 1);
        //printf("invert lookup on %d:%d:%d is %d\n", audioPortID, mixerOutputNumber+1, mixerInputNumber+1, invert);
      }
    }
    else {
      invert = mixerOutputInterface->invert(audioPortID, mixerOutputNumber + 1);
    }

    if (rightInvertPushButton)
      rightInvertPushButton->setChecked(invert);
  }
}

double MixerChannelWidget::pixelsToDb(Word pixels)
{
  int toReturn = (double)pixelsToIntForICA(pixels);
  return  toReturn * dbConversionFactor;
}

int16_t MixerChannelWidget::pixelsToIntForICA(Word pixels)
{
  pixels = 299 - pixels;
  if (pixels < 75) {
    if (pixels == 0)
      return 6*256;
    else {
      return ((int)(6*256 - ((double)6*(double)256/(double)75 * pixels))) / 16 * 16;
    }
  }
  else if (pixels < 150) {
    if (pixels == 75)
      return 0;
    else {
      return ((int)(0*256 - ((double)6*(double)256/(double)75 * (pixels-75)))) / 16 * 16;
    }
  }
  else if (pixels < 188) {
    if (pixels == 150)
      return -6 * 256;
    else {
      return ((int)(-6*256 - ((double)6*(double)256/(double)38 * (pixels-150)))) / 16 * 16;
    }
  }
  else if (pixels < 206) {
    if (pixels == 188)
      return -12 * 256;
    else {
      return ((int)(-12*256 - ((double)6*(double)256/(double)18 * (pixels-188)))) / 16 * 16;
    }

  }
  else if (pixels < 224) {
    if (pixels == 206)
      return -18 * 256;
    else {
      return ((int)(-18*256 - ((double)6*(double)256/(double)18 * (pixels-206)))) / 16 * 16;
    }

  }
  else if (pixels < 243) {
    if (pixels == 224)
      return -24 * 256;
    else {
      return ((int)(-24*256 - ((double)12*(double)256/(double)19 * (pixels-224)))) / 16 * 16;
    }

  }
  else if (pixels < 263) {
    if (pixels == 243)
      return -36 * 256;
    else {
      return ((int)(-36*256 - ((double)12*(double)256/(double)19 * (pixels-243)))) / 16 * 16;
    }

  }
  else if (pixels < 282) {
    if (pixels == 263)
      return -48 * 256;
    else {
      return ((int)(-48*256 - ((double)12*(double)256/(double)19 * (pixels-263)))) / 16 * 16;
    }

  }
  else if (pixels < 300) {
    if (pixels == 282)
      return -60 * 256;
    else if (pixels == 299)
      return 0x8000;
    else {
      return ((int)(-60*256 - ((double)20*(double)256/(double)18 * (pixels-282)))) / 16 * 16;
    }
  }
  else return 0;
}

Word MixerChannelWidget::toPixelsFromICA(int16_t theInt)
{
  if (theInt <= -60*256) {
    if (theInt <= -80*256)
      return (299 - 299);
    else {
      return ((int)((299 - 299) + ((double)(theInt - -80*256)/((double)(256*20) / 18))))+1;
    }
  }
  else if (theInt < -48*256) {
    if (theInt == -60*256)
      return (299 - 282);
    else {
      return ((int)((299 - 282) + ((double)(theInt - -60*256)/((double)(256*12) / 19))))+1;
    }
  }
  else if (theInt < -36*256) {
    if (theInt == -48*256)
      return (299 - 263);
    else {
      return ((int)((299 - 263) + ((double)(theInt - -48*256)/((double)(256*12) / 19))))+1;
    }
  }
  else if (theInt < -24*256) {
    if (theInt == -36*256)
      return (299 - 243);
    else {
      return ((int)((299 - 243) + ((double)(theInt - -36*256)/((double)(256*12) / 19))))+1;
    }
  }
  else if (theInt < -18*256) {
    if (theInt == -24*256)
      return (299 - 224);
    else {
      return ((int)((299 - 224) + ((double)(theInt - -24*256)/((double)(256*6) / 18))))+1;
    }
  }
  else if (theInt < -12*256) {
    if (theInt == -18*256)
      return (299 - 206);
    else {
      return ((int)((299 - 206) + ((double)(theInt - -18*256)/((double)(256*6) / 18))))+1;
    }
  }
  else if (theInt < -6*256) {
    if (theInt == -12*256)
      return (299 - 188);
    else {
      return ((int)((299 - 188) + ((double)(theInt - -12*256)/((double)(256*6) / 38))))+1;
    }
  }
  else if (theInt < 0*256) {
    if (theInt == -6*256)
      return (299 - 150);
    else {
      return ((int)((299 - 150) + ((double)(theInt - -6*256)/((double)(256*6) / 75))))+1;
    }
  }
  else if (theInt < 6*256) {
    if (theInt == 0*256)
      return (299 - 75);
    else {
      return ((int)((299 - 75) + ((double)(theInt - 0*256)/((double)(256*6) / 75))))+1;
    }
  }
  else if (theInt >= 6*256) {
    return 299;
  }
//  else return 0;
}
