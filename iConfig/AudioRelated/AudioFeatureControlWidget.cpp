/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "AudioFeatureControlWidget.h"
#include "AudioChannelsControlWidget.h"
#include "AudioChannelConfigWidget.h"
#include "AudioGlobalParm.h"
#include "AudioPortParm.h"
#include "AudioPatchbayParm.h"
#include "../MixerRelated/MixerChannelWidget.h"
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>

#include <math.h>

using namespace GeneSysLib;

AudioFeatureControlWidget::AudioFeatureControlWidget(
    IAudioControlFeatureSourcePtr audioFeatureSource, Word audioPortID, Byte channelID,
    DeviceInfoPtr device,
    QWidget *parent, int totalOutputChannelInPort)  //Bugfixing for PlayAudio, zx-03-02
    : QWidget(parent),
      device(device),
      audioFeatureSource(audioFeatureSource),
      channelID(channelID),
      audioPortID(audioPortID) {
  m_TotalOutputChannelInPort = totalOutputChannelInPort;
  auto vBoxLayout = new QVBoxLayout();
  vBoxLayout->setContentsMargins(0,0,0,0);
  vBoxLayout->setSpacing(2);
  vBoxLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
  setLayout(vBoxLayout);

  panBoxLayout = new QHBoxLayout();
  panBoxLayout->setContentsMargins(0,0,0,0);
  panBoxLayout->setAlignment(Qt::AlignCenter);
  panBox = new QWidget();
  panBox->setContentsMargins(0,0,0,0);
//  panBox->setMaximumHeight(80);
//  panBox->setMinimumHeight(80);

  this->setMinimumWidth(60);
  this->setMaximumWidth(60);

  stereoLinked = audioFeatureSource->stereoLink(channelID);

  if (stereoLinked) {
      this->setMaximumWidth(80);
      this->setMinimumWidth(80);
  }

  visibilitySet = false;

  this->setStyleSheet(QString("QComboBox {font-size:9pt;}; QPushButton { font-size:9pt}; QLabel { font-size:9pt;};"));

  if (parent) {
    parentCCW = (AudioChannelsControlWidget*) parent;
  }

  clippingTimer1 = new QTimer();
  clippingTimer2 = new QTimer();
  connect(clippingTimer1,SIGNAL(timeout()), this, SLOT(turnOffClipping1()));
  connect(clippingTimer2,SIGNAL(timeout()), this, SLOT(turnOffClipping2()));

  buildAll();
}

void AudioFeatureControlWidget::turnOnClipping1()
{
  clippingPushButton1->setChecked(true);
  clippingTimer1->stop();
  clippingTimer1->setSingleShot(true);
  clippingTimer1->setInterval(1000);
  clippingTimer1->start();
}

void AudioFeatureControlWidget::turnOffClipping1()
{
  clippingPushButton1->setChecked(false);
}

void AudioFeatureControlWidget::turnOnClipping2()
{
  clippingPushButton2->setChecked(true);
  clippingTimer2->stop();
  clippingTimer2->setSingleShot(true);
  clippingTimer2->setInterval(1000);
  clippingTimer2->start();
}

void AudioFeatureControlWidget::turnOffClipping2()
{
  clippingPushButton2->setChecked(false);
}

void AudioFeatureControlWidget::labelDone(double value)
{
  if (value > volumeSlider->maximum())
    value = volumeSlider->maximum();
  else if (value < volumeSlider->minimum())
    value = volumeSlider->minimum();
  volumeSliderChanged(value);
}

Byte AudioFeatureControlWidget::getPairedChannelID(Byte channelID) {
  if (channelID % 2) {
    return channelID + 1;
  }
  else {
    return channelID - 1;
  }
}

void AudioFeatureControlWidget::panDialChanged(int value) {
  int16_t leftVal, rightVal;
  if (value < 0) {
    leftVal = 0;
    rightVal = value;
  }
  else if (value > 0) {
    rightVal = 0;
    leftVal = 0 - value;
  }
  else { // == 0
    leftVal = rightVal = 0;
  }

  leftVal /= IAudioControlFeatureSource::VolumedBConversionFactor;
  rightVal /= IAudioControlFeatureSource::VolumedBConversionFactor;

  audioFeatureSource->trimCurrent(channelID, ((int)(leftVal * (audioFeatureSource->volumeResolution(channelID)*
                                                      IAudioControlFeatureSource::VolumedBConversionFactor))) & 0xFFFF);
  audioFeatureSource->trimCurrent(channelID + 1, ((int)(rightVal * (audioFeatureSource->volumeResolution(channelID)*
                                                      IAudioControlFeatureSource::VolumedBConversionFactor))) & 0xFFFF);

  if (audioFeatureSource->trimCurrent(channelID) < 0) {
    value = 0 - audioFeatureSource->trimCurrent(channelID);
    currentPanLabel->setText("<b>R" + QString::number(value * IAudioControlFeatureSource::VolumedBConversionFactor,'f',1).rightJustified(5, ' ').replace(" ", "&nbsp;") + "</b>");
  }
  else if (audioFeatureSource->trimCurrent(channelID + 1) < 0){
    value = 0 - audioFeatureSource->trimCurrent(channelID + 1);
    currentPanLabel->setText("<b>L" + QString::number(value * IAudioControlFeatureSource::VolumedBConversionFactor,'f',1).rightJustified(5, ' ').replace(" ", "&nbsp;") + "</b>");
  }
  else {
    value = 0;
    currentPanLabel->setText("<b>" + QString("  C  ").replace(" ", "&nbsp;") + "</b>");
  }
}

void AudioFeatureControlWidget::volumeSliderChanged(int value) {
  if (audioFeatureSource->channelType(channelID) == 2 && value == volumeSlider->minimum()) {
    value = 0x8000;
    if (audioFeatureSource->volumeCurrent(channelID) != value) {
      audioFeatureSource->volumeCurrent(channelID, value);
    }
  }
  else if (audioFeatureSource->volumeCurrent(channelID) != (int)(value / IAudioControlFeatureSource::VolumedBConversionFactor *
                                                            (audioFeatureSource->volumeResolution(channelID)*
                                                                                        IAudioControlFeatureSource::VolumedBConversionFactor))) {
    audioFeatureSource->volumeCurrent(channelID, ((int)(value / IAudioControlFeatureSource::VolumedBConversionFactor *
                                                        (audioFeatureSource->volumeResolution(channelID)*
                                                                                    IAudioControlFeatureSource::VolumedBConversionFactor))) & 0xFFFF);
  }
  if (audioFeatureSource->channelType(channelID) == 2 && value == 0x8000) {
    currentVolumeLabel->setValue(-100000);
  }
  else {
    currentVolumeLabel->setValue(audioFeatureSource->volumeCurrent(channelID) *
                                              IAudioControlFeatureSource::VolumedBConversionFactor);
  }
}

void AudioFeatureControlWidget::muteStateChanged(bool state) {
  if (audioFeatureSource->mute(channelID) != state)
    audioFeatureSource->mute(channelID, state);
}

void AudioFeatureControlWidget::stereoLinkStateChanged(bool state) {
  if (audioFeatureSource->stereoLink(channelID) != state)
    audioFeatureSource->stereoLink(channelID, state);
  //printf("notifying parent\n");
  parentCCW->notifyStereoLinkHappened();
}

void AudioFeatureControlWidget::phantomPowerStateChanged(bool state) {
  audioFeatureSource->phantomPower(channelID, state);
}

void AudioFeatureControlWidget::highImpedanceStateChanged(bool state)
{
  audioFeatureSource->highImpedance(channelID, state);
}

void AudioFeatureControlWidget::rightPhantomPowerStateChanged(bool state)
{
  audioFeatureSource->phantomPower(channelID + 1, state);
}

void AudioFeatureControlWidget::rightHighImpedanceStateChanged(bool state)
{
  audioFeatureSource->highImpedance(channelID + 1, state);
}

void AudioFeatureControlWidget::buildAll() {
  buildChannelNameLabel();
  //buildStereoLinkPushButton();
  buildPanDial();
  buildVolumeSlider();
  buildMutePushButton();

  if (audioFeatureSource->isPhantomPowerAvailable(channelID)) {
    buildPhantomPowerPushButton();
  }

  if (audioFeatureSource->isHighImpedanceAvailable(channelID)) {
    buildHighImpedancePushButton();
  }

  setWidgetVisibility();
}

void AudioFeatureControlWidget::setWidgetVisibility() {
  if (!visibilitySet) {
    volumeSlider->setVisible(audioFeatureSource->isVolumeAvailable(channelID));
    if (stereoLinked)
      panDial->setVisible(audioFeatureSource->isVolumeAvailable(channelID));
    mutePushButton->setVisible(audioFeatureSource->isMuteAvailable(channelID));
    if (audioFeatureSource->isPhantomPowerAvailable(channelID)) {
      phantomPowerPushButton->setVisible(audioFeatureSource->isPhantomPowerAvailable(channelID));
      if (stereoLinked)
        rightPhantomPowerPushButton->setVisible(audioFeatureSource->isPhantomPowerAvailable(channelID + 1));
    }
    if (audioFeatureSource->isHighImpedanceAvailable(channelID)) {
      highImpedancePushButton->setVisible(audioFeatureSource->isHighImpedanceAvailable(channelID));
    }
    visibilitySet = true;
  }
}

void AudioFeatureControlWidget::buildChannelNameLabel() {
  channelNameLabel = 0;

  int trueChannelID = channelID;

  if (!audioFeatureSource->controllerName().compare("Headphones")) {
    trueChannelID += m_TotalOutputChannelInPort -2;  //4; //Bugfixing for PlayAudio, zx, 2017-03-02
    auto headphonesImage = new QWidget();
    headphonesImage->setContentsMargins(0,0,0,0);
    headphonesImage->setMinimumWidth(19);
    headphonesImage->setMaximumWidth(19);
    headphonesImage->setMinimumHeight(16);
    headphonesImage->setMaximumHeight(16);
    headphonesImage->setStyleSheet("border: none; background-image: url(:/mixer_graphics/Images/mixer_graphics/headphones.png);");

    auto spacer = new QWidget();
    spacer->setContentsMargins(0,0,0,0);
    spacer->setMinimumHeight(3);
    spacer->setMaximumHeight(3);
    auto spacer2 = new QWidget();
    spacer2->setContentsMargins(0,0,0,0);
    spacer2->setMinimumHeight(3);
    spacer2->setMaximumHeight(3);
    layout()->addWidget(spacer);
    layout()->addWidget(headphonesImage);
    layout()->addWidget(spacer2);
    layout()->setAlignment(headphonesImage, Qt::AlignCenter);
  }
  else {
    channelNameLabel = new QLabel();
    channelNameLabel->setAlignment(Qt::AlignCenter);
    channelNameLabel->setMinimumHeight(26);
    channelNameLabel->setMaximumHeight(26);
    channelNameLabel->setStyleSheet("QLabel { color: #65ceff; margin-top: 10px;}");
    layout()->addWidget(channelNameLabel);
    layout()->setAlignment(channelNameLabel, Qt::AlignCenter);
  }

  if ((!audioFeatureSource->controllerName().compare("Line Outputs")) ||
      (!audioFeatureSource->controllerName().compare("Headphones"))){
    outputConfigPushButton = new QPushButton();
    outputConfigPushButton->setEnabled(true);
    if (stereoLinked) {
      outputConfigPushButton->setMinimumWidth(80);
      outputConfigPushButton->setMaximumWidth(80);
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

    MixerInterface* mixerInterface = new MixerInterface(device);

    const auto &audioGlobalParm = device->get<AudioGlobalParm>();

    for (Word aPid = 1; aPid <= audioGlobalParm.numAudioPorts(); ++aPid) {
      const auto &portParm = device->get<AudioPortParm>(aPid);
      // TODO: Should be real names.
      //portNames.append(QString::fromStdString(portParm.portName()));
      switch(aPid) {
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
    }

    for (Word audioPortID = 1; audioPortID <= audioGlobalParm.numAudioPorts(); ++audioPortID) {
      int numOutputs = mixerInterface->numberOutputs(audioPortID);
      QString mixName = "";
      for (int j = 1; j <= numOutputs; j = j + 2) {
        if (audioPortID == 1) { // TODO: hack for now
          mixName = "U1:M" + QString::number((j+1)/2);
        }
        else if (audioPortID == 2) {
          mixName = "U2:M" + QString::number((j+1)/2);
        }
        else {
          mixName = "A:M" + QString::number((j+1)/2);
        }
        portNames.append(mixName);
      }
    }

    const auto &portParm = device->get<AudioPortParm>(audioPortID);
    for (Byte outChannelID = 1; outChannelID <= portParm.numOutputChannels(); ++ outChannelID) {
      allAvailableInChannels.append(QPair<int,int>(audioPortID, outChannelID));
    }

    for (Word aPid = 1; aPid <= audioGlobalParm.numAudioPorts(); ++aPid) {
      int numOutputs = mixerInterface->numberOutputs(aPid);
      for (int j = 1; j <= numOutputs; j++) {
        allAvailableInChannels.append(QPair<int,int>(3 + aPid, j));
      }
    }

    QString title = "";
    QString title2 = "";

    int count = 3;
    for (Word aPid = 1; aPid <= audioGlobalParm.numAudioPorts(); ++aPid) {
      int numOutputs = mixerInterface->numberOutputs(aPid);
      for (int j = 1; j <= numOutputs; j++) {
        std::vector<int8_t> channels = mixerInterface->channelIDsForOutput(aPid, j);
        for (int8_t x : channels) {
          if (x == trueChannelID && aPid == audioPortID) {
            if (j % 2)
              title = portNames.at(count) + ":L";
            else
              title = portNames.at(count) + ":R";
          }
          if (stereoLinked) {
            if (x == (trueChannelID+1) && aPid == audioPortID) {
              if (j % 2)
                title2 = portNames.at(count) + ":L";
              else {
                title2 = portNames.at(count) + ":R";
              }
            }
          }
        }
        if (!(j % 2))
          count++;
      }
    }
    if (title == "" || title2 == "") {
      for (Word aPid = 1; aPid <= audioGlobalParm.numAudioPorts(); ++aPid) {
        auto &patchBay = device->get<AudioPatchbayParm>(aPid);
        auto flatList = patchBay.flatList();
        for (auto &f : flatList) {
          int p1 = f.outPortID;
          int c1 = f.outChannelNumber;
          int p2 = f.inPortID;
          int c2 = f.inChannelNumber;

          if (p1 > 0) {
            if (p2 == audioPortID && c2 == trueChannelID) {
              title = (portNames.at(p1 - 1) + ":" + QString::number(c1));
            }
            if (stereoLinked) {
              if (p2 == audioPortID && c2 == trueChannelID+1) {
                title2 = (portNames.at(p1 - 1) + ":" + QString::number(c1));
              }
            }
          }
        }
      }
    }

    if (title == "") {
      title = ("No SRC");
    }
    if (title2 == "") {
      title2 = ("No SRC");
    }

    if (stereoLinked)
      outputConfigPushButton->setText(title + " | " + title2);
    else
      outputConfigPushButton->setText(title);
  }

  updateChannelNameLabel();

}

void AudioFeatureControlWidget::channelConfigPressed()
{
  /*MixerChannelConfigDialog *dialog = new MixerChannelConfigDialog(this, device, audioPortID, mixerOutputNumber);
  dialog->exec();*/

  QFrame* popup1 = new QFrame(this, Qt::Popup | Qt::Window );
  popup1->setObjectName("popup");
  popup1->setStyleSheet("QFrame#popup { background-color: #000000; }");

  AudioChannelConfigWidget *widget = new AudioChannelConfigWidget(this, device, audioFeatureSource, audioPortID, 0, channelID, popup1, m_TotalOutputChannelInPort); //Bugfixing for PlayAudio, zx-03-02
  popup1->move(QCursor::pos());
  popup1->move(QCursor::pos() - QPoint(420,0));
  popup1->resize(420,widget->minimumHeight() + 20);
  widget->setGeometry(10,10, 400, minimumHeight());
  popup1->show();
}

void AudioFeatureControlWidget::buildPanDial() {
  auto vBoxWidgetPan = new QWidget();
  vBoxWidgetPan->setMinimumWidth(50);
  vBoxWidgetPan->setMaximumWidth(50);
  vBoxWidgetPan->setMinimumHeight(80);
  vBoxWidgetPan->setMaximumHeight(80);

  if (stereoLinked) {
    panDial = new QClickyDial();

    panDial->setStyleSheet("QClickyDial {background-color: #65ceff};");

    panDial->setMinimum(audioFeatureSource->trimMin(channelID)*
                           IAudioControlFeatureSource::VolumedBConversionFactor/
                           (audioFeatureSource->volumeResolution(channelID)*
                                                       IAudioControlFeatureSource::VolumedBConversionFactor));
    panDial->setMaximum(0 - audioFeatureSource->trimMin(channelID)*
                           IAudioControlFeatureSource::VolumedBConversionFactor/
                           (audioFeatureSource->volumeResolution(channelID)*
                                                       IAudioControlFeatureSource::VolumedBConversionFactor));
    panDial->setValue(0);
    panDial->setSingleStep(1);
    panDial->setPageStep(1);
    panDial->setNotchesVisible(true);

    panDial->setEnabled(audioFeatureSource->isVolumeEditable(channelID));
    panDial->setFocusPolicy(Qt::StrongFocus);
    auto vBoxLayoutPan = new QVBoxLayout();
    double panMin = audioFeatureSource->trimMin(channelID) *
                         IAudioControlFeatureSource::VolumedBConversionFactor;
    double panMax = 0 - audioFeatureSource->trimMin(channelID) *
                         IAudioControlFeatureSource::VolumedBConversionFactor;
    panDial->setStatusTip("Max: " + QString::number(panMax) + tr(" dB") + "\n" +
                             "Min: " + QString::number(panMin) + tr(" dB"));

    currentPanLabel = new QLabel("");
    int value = 0;
    if (audioFeatureSource->trimCurrent(channelID) < 0) {
      value = 0 - audioFeatureSource->trimCurrent(channelID);
      currentPanLabel->setText("<b>R" + QString::number(value * IAudioControlFeatureSource::VolumedBConversionFactor,'f',1).rightJustified(5, ' ').replace(" ", "&nbsp;") + "</b>");
    }
    else if (audioFeatureSource->trimCurrent(channelID + 1) < 0){
      value = 0 - audioFeatureSource->trimCurrent(channelID + 1);
      currentPanLabel->setText("<b>L" + QString::number(value * IAudioControlFeatureSource::VolumedBConversionFactor,'f',1).rightJustified(5, ' ').replace(" ", "&nbsp;") + "</b>");
    }
    else {
      value = 0;
      currentPanLabel->setText("<b>" + QString("  C  ").replace(" ", "&nbsp;") + "</b>");
    }
    currentPanLabel->setAlignment(Qt::AlignHCenter);
    currentPanLabel->setStyleSheet("font-size: 9pt; font-family:Consolas,Monaco,Lucida Console,Liberation Mono,DejaVu Sans Mono,Bitstream Vera Sans Mono,Courier New, monospace;");
    currentPanLabel->setMinimumWidth(25);

    auto panLabelButton = new QPushButton();
    if (stereoLinked)
      panLabelButton->setText(tr("BAL"));
    else
      panLabelButton->setText(tr("PAN"));
    panLabelButton->setEnabled(false);
    panLabelButton->setMinimumWidth(50);
    panLabelButton->setMaximumWidth(50);

    panLabelButton->setStyleSheet(QString("QPushButton { font-size: 10pt; color:#ffffff; padding:3px; border: 0px; } \
                                          QPushButton:pressed { color: #ffffff;  }"));

    vBoxLayoutPan->setSpacing(0);
    vBoxLayoutPan->setContentsMargins(0, 0, 0, 0);
    vBoxLayoutPan->addWidget(panLabelButton);
    vBoxLayoutPan->addWidget(panDial);
    vBoxLayoutPan->addWidget(currentPanLabel);
    vBoxLayoutPan->setAlignment(panDial, Qt::AlignTop);
    vBoxLayoutPan->setAlignment(currentPanLabel, Qt::AlignTop);
    vBoxWidgetPan->setLayout(vBoxLayoutPan);
  }

  panBoxLayout->addWidget(vBoxWidgetPan);
  panBoxLayout->setAlignment(vBoxWidgetPan,Qt::AlignTop);

  panBox->setLayout(panBoxLayout);
  layout()->addWidget(panBox);
  layout()->setAlignment(panBox,Qt::AlignTop);

  if (stereoLinked) {
    updatePanValue();
    connect(panDial, SIGNAL(valueChanged(int)), this,
            SLOT(panDialChanged(int)));
  }
}

void AudioFeatureControlWidget::buildVolumeSlider() {
  if (audioFeatureSource->channelType(channelID) == 1) {
    QWidget* spacerWidget = new QWidget();
    spacerWidget->setMinimumHeight(15);
    spacerWidget->setMaximumHeight(15);
    layout()->addWidget(spacerWidget);
  }

  volumeSlider = new QClickySlider();
  volumeSlider->setMinimum(audioFeatureSource->volumeMin(channelID)*
                           IAudioControlFeatureSource::VolumedBConversionFactor/
                           (audioFeatureSource->volumeResolution(channelID)*
                                                       IAudioControlFeatureSource::VolumedBConversionFactor));
  volumeSlider->setMaximum(audioFeatureSource->volumeMax(channelID)*
                           IAudioControlFeatureSource::VolumedBConversionFactor/
                           (audioFeatureSource->volumeResolution(channelID)*
                                                       IAudioControlFeatureSource::VolumedBConversionFactor));
  volumeSlider->setSingleStep(1);
  volumeSlider->setPageStep(1);
  volumeSlider->setEnabled(audioFeatureSource->isVolumeEditable(channelID));
  volumeSlider->setFocusPolicy(Qt::StrongFocus);
  volumeSlider->setMaximumHeight(320); // 300 pixels of usable space on OSX
  volumeSlider->setMinimumHeight(320);
  //volumeSlider->setStyleSheet("margin-right:30px;");

  double unpaddedMin = audioFeatureSource->volumeMin(channelID) *
                       IAudioControlFeatureSource::VolumedBConversionFactor;
  double unpaddedMax = audioFeatureSource->volumeMax(channelID) *
                       IAudioControlFeatureSource::VolumedBConversionFactor;

  double paddedMin =
      (double)((int32_t) audioFeatureSource->volumeMin(channelID) - (int32_t)
               audioFeatureSource->volumePad(channelID)) *
      IAudioControlFeatureSource::VolumedBConversionFactor;
  double paddedMax =
      (double)((int32_t) audioFeatureSource->volumeMax(channelID) - (int32_t)
               audioFeatureSource->volumePad(channelID)) *
      IAudioControlFeatureSource::VolumedBConversionFactor;

  bool containsPadding = (audioFeatureSource->volumePad(channelID) != 0);

  QString unpaddedMinStr = QString::number(unpaddedMin);
  QString unpaddedMaxStr = QString::number(unpaddedMax);

  QString paddedMinStr = unpaddedMinStr + tr("(unpadded) / ") + QString::number(paddedMin) + " (padded)";
  QString paddedMaxStr = unpaddedMaxStr + tr("(unpadded) / ") + QString::number(paddedMax) + " (padded)";

  volumeSlider->setStatusTip("Max: " + (containsPadding ? paddedMinStr : unpaddedMinStr) + tr(" dB") + "\n" +
                             "Min: " + (containsPadding ? paddedMaxStr : unpaddedMaxStr) + tr(" dB"));

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

  QVBoxLayout* meterBar1BoxLayout = new QVBoxLayout();
  meterBar1BoxLayout->setSpacing(0);
  meterBar1BoxLayout->setContentsMargins(0,0,0,0);
  meterBar1BoxLayout->addWidget(meterBar1spacer1);
  meterBar1BoxLayout->addWidget(meterBar1);
  meterBar1BoxLayout->addWidget(meterBar1spacer2);
  meterBar1BoxLayout->setAlignment(meterBar1spacer1, Qt::AlignCenter);
  meterBar1BoxLayout->setAlignment(meterBar1, Qt::AlignCenter);
  QWidget* meterBar1Box = new QWidget();
  meterBar1Box->setContentsMargins(0,0,0,0);
  meterBar1Box->setLayout(meterBar1BoxLayout);

  QWidget* meterBar2Box = 0;

  if (stereoLinked) {
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

    QVBoxLayout* meterBar2BoxLayout = new QVBoxLayout();
    meterBar2BoxLayout->setSpacing(0);
    meterBar2BoxLayout->setContentsMargins(0,0,0,0);
    meterBar2BoxLayout->addWidget(meterBar2spacer1);
    meterBar2BoxLayout->addWidget(meterBar2);
    meterBar2BoxLayout->addWidget(meterBar2spacer2);

    meterBar2BoxLayout->setAlignment(meterBar2spacer1, Qt::AlignCenter);
    meterBar2BoxLayout->setAlignment(meterBar2, Qt::AlignCenter);

    meterBar2Box = new QWidget();
    meterBar2Box->setContentsMargins(0,0,0,0);
    meterBar2Box->setLayout(meterBar2BoxLayout);
  }

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
  if (meterBar2Box) {
    volBoxLayoutTop->addWidget(meterBar2Box);
    QWidget* spacer = new QWidget();
    spacer->setMinimumWidth(12);
    spacer->setMaximumWidth(12);
    volBoxLayoutTop->addWidget(spacer);
  }
  volBoxTop->setLayout(volBoxLayoutTop);

  volBoxLayoutBottom->addWidget(currentVolumeLabel);
  volBoxLayoutBottom->addWidget(volFooter);
  volBoxBottom->setLayout(volBoxLayoutBottom);

  volBoxLayout->addWidget(volBoxTop);
  volBoxLayout->addWidget(volBoxBottom);
  volBox->setLayout(volBoxLayout);

  layout()->addWidget(volBox);
  layout()->setAlignment(volBox, Qt::AlignHCenter);

  updateVolumeValue();

  connect(volumeSlider, SIGNAL(valueChanged(int)), this,
          SLOT(volumeSliderChanged(int)));
}

void AudioFeatureControlWidget::buildMutePushButton() {
  mutePushButton = new QPushButton();
  //mutePushButton->setText(tr("Mute"));
  mutePushButton->setEnabled(audioFeatureSource->isMuteEditable(channelID));
  mutePushButton->setCheckable(true);
  if (!stereoLinked) {
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

  updateMuteValue();

  connect(mutePushButton, SIGNAL(toggled(bool)), this,
          SLOT(muteStateChanged(bool)));
}

void AudioFeatureControlWidget::buildStereoLinkPushButton() {
  /*if (!(channelID % 2) || stereoLinked) {
    stereoLinkPushButton = new QPushButton();
    stereoLinkPushButton->setText("L\ni\nn\nk");
    stereoLinkPushButton->setEnabled(audioFeatureSource->isStereoLinkEditable(channelID));
    stereoLinkPushButton->setMaximumWidth(15);
    stereoLinkPushButton->setMaximumHeight(75);
    stereoLinkPushButton->setContentsMargins(0,0,0,0);

    stereoLinkPushButton->setCheckable(true);
    stereoLinkPushButton->setStyleSheet(QString("QPushButton {font-size: 10pt; padding:3px; border: 1px solid #8f8f91; border-radius: 6px; background-color: white;} \
                                          QPushButton:checked { color: white; background-color: #00BFFF; } \
                                          QPushButton:pressed { color: black; background-color: #FFFAF0; }"));

    if (!(channelID % 2))
      panBoxLayout->addWidget(stereoLinkPushButton);
    updateStereoLinkValue();

    connect(stereoLinkPushButton, SIGNAL(toggled(bool)), this,
            SLOT(stereoLinkStateChanged(bool)));
  }
  else {
    panBoxLayout->addWidget(new QLabel());
  }*/
}

void AudioFeatureControlWidget::buildPhantomPowerPushButton() {
  phantomPowerPushButton = new QPushButton();
  phantomPowerPushButton->setEnabled(
      audioFeatureSource->isPhantomPowerEditable(channelID));

  if (!stereoLinked) {
    phantomPowerPushButton->setMinimumWidth(49);
    phantomPowerPushButton->setMaximumWidth(49);
    phantomPowerPushButton->setStyleSheet(QString("QPushButton {background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_48v_off.png); border: 0px;} \
                                          QPushButton:checked { background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_48v_on.png); }"));
  }
  else {
    phantomPowerPushButton->setMinimumWidth(31);
    phantomPowerPushButton->setMaximumWidth(31);
    phantomPowerPushButton->setStyleSheet(QString("QPushButton {background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_48v_left_link_off.png); border: 0px;} \
                                          QPushButton:checked { background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_48v_left_link_on.png); }"));
  }

  phantomPowerPushButton->setMinimumHeight(21);
  phantomPowerPushButton->setMaximumHeight(21);

  phantomPowerPushButton->setCheckable(true);

  if (stereoLinked) {
    rightPhantomPowerPushButton = new QPushButton();
    rightPhantomPowerPushButton->setEnabled(
        audioFeatureSource->isPhantomPowerEditable(channelID + 1));

    rightPhantomPowerPushButton->setMinimumWidth(31);
    rightPhantomPowerPushButton->setMaximumWidth(31);
    rightPhantomPowerPushButton->setStyleSheet(QString("QPushButton {background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_48v_right_link_off.png); border: 0px;} \
                                          QPushButton:checked { background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_48v_right_link_on.png); }"));


    rightPhantomPowerPushButton->setCheckable(true);
    rightPhantomPowerPushButton->setMinimumHeight(21);
    rightPhantomPowerPushButton->setMaximumHeight(21);

    auto invertBoxLayout = new QHBoxLayout();

    invertBoxLayout->setContentsMargins(0, 10, 0, 0);
    invertBoxLayout->setSpacing(10);

    QSizePolicy exp = QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);

//    invertBoxLayout->setContentsMargins(0, 0, 0, 0);
    auto invertBox = new QWidget();
    invertBox->setSizePolicy(exp);

    invertBoxLayout->addWidget(phantomPowerPushButton);
    invertBoxLayout->addWidget(rightPhantomPowerPushButton);
    invertBoxLayout->setMargin(0);
    invertBox->setLayout(invertBoxLayout);

    invertBox->setStyleSheet(QString("QWidget { padding:0px; margin: 0px;}"));
    layout()->addWidget(invertBox);
    layout()->setAlignment(invertBox, Qt::AlignCenter);

    invertBox->setContentsMargins(0, 0, 0, 0);
  }
  else {
    QHBoxLayout *lay = new QHBoxLayout();
    QWidget *wid = new QWidget();
    lay->addWidget(phantomPowerPushButton); lay->setMargin(0);
    wid->setLayout(lay); wid->setContentsMargins(0,0,0,0);
    layout()->addWidget(wid);
    layout()->setAlignment(wid, Qt::AlignCenter);
  }

  updatePhantomValue();
  updateRightPhantomValue();

  connect(phantomPowerPushButton, SIGNAL(toggled(bool)), this,
          SLOT(phantomPowerStateChanged(bool)));
  if (stereoLinked) {
    connect(rightPhantomPowerPushButton, SIGNAL(toggled(bool)), this,
          SLOT(rightPhantomPowerStateChanged(bool)));
  }
}

void AudioFeatureControlWidget::buildHighImpedancePushButton()
{
  highImpedancePushButton = new QPushButton();
  highImpedancePushButton->setEnabled(
      audioFeatureSource->isHighImpedanceEditable(channelID));

  if (!stereoLinked) {
    highImpedancePushButton->setMinimumWidth(49);
    highImpedancePushButton->setMaximumWidth(49);
    highImpedancePushButton->setStyleSheet(QString("QPushButton {background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_inst_off.png); border: 0px;} \
                                          QPushButton:checked { background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_inst_on.png); }"));
  }
  else {
    highImpedancePushButton->setMinimumWidth(31);
    highImpedancePushButton->setMaximumWidth(31);
    highImpedancePushButton->setStyleSheet(QString("QPushButton {background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_inst_left_link_off.png); border: 0px;} \
                                          QPushButton:checked { background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_inst_left_link_on.png); }"));
  }

  highImpedancePushButton->setMinimumHeight(21);
  highImpedancePushButton->setMaximumHeight(21);

  highImpedancePushButton->setCheckable(true);

  if (stereoLinked) {
    rightHighImpedancePushButton = new QPushButton();
    rightHighImpedancePushButton->setEnabled(
        audioFeatureSource->isHighImpedanceEditable(channelID + 1));

    rightHighImpedancePushButton->setMinimumWidth(31);
    rightHighImpedancePushButton->setMaximumWidth(31);
    rightHighImpedancePushButton->setStyleSheet(QString("QPushButton {background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_inst_right_link_off.png); border: 0px;} \
                                          QPushButton:checked { background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_inst_right_link_on.png); }"));

    rightHighImpedancePushButton->setCheckable(true);
    rightHighImpedancePushButton->setMinimumHeight(21);
    rightHighImpedancePushButton->setMaximumHeight(21);

    auto invertBoxLayout = new QHBoxLayout();

    invertBoxLayout->setContentsMargins(0, 10, 0, 0);
    invertBoxLayout->setSpacing(10);

    QSizePolicy exp = QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);

//    invertBoxLayout->setContentsMargins(0, 0, 0, 0);
    auto invertBox = new QWidget();
    invertBox->setSizePolicy(exp);

    invertBoxLayout->addWidget(highImpedancePushButton);
    invertBoxLayout->addWidget(rightHighImpedancePushButton);
    invertBoxLayout->setMargin(0);
    invertBox->setLayout(invertBoxLayout);

    invertBox->setStyleSheet(QString("QWidget { padding:0px; margin: 0px;}"));
    layout()->addWidget(invertBox);
    layout()->setAlignment(invertBox, Qt::AlignCenter);

    invertBox->setContentsMargins(0, 0, 0, 0);
  }
  else {
    QHBoxLayout *lay = new QHBoxLayout();
    QWidget *wid = new QWidget();
    lay->addWidget(highImpedancePushButton); lay->setMargin(0);
    wid->setLayout(lay); wid->setContentsMargins(0,0,0,0);
    layout()->addWidget(wid);
    layout()->setAlignment(wid, Qt::AlignCenter);
  }

  updateHighImpedanceValue();
  updateRightHighImpedanceValue();

  connect(highImpedancePushButton, SIGNAL(toggled(bool)), this,
          SLOT(highImpedanceStateChanged(bool)));

  if (stereoLinked) {
    connect(rightHighImpedancePushButton, SIGNAL(toggled(bool)), this,
            SLOT(rightHighImpedanceStateChanged(bool)));
  }
}

void AudioFeatureControlWidget::updateAll() {
  static int count = 0;

  if (count == 0) {
    updateChannelNameLabel();
    if (audioFeatureSource->isPhantomPowerAvailable(channelID)) {
      updatePhantomValue();
      if ( stereoLinked ) updateRightPhantomValue();
    }
    if (audioFeatureSource->isHighImpedanceAvailable(channelID)) {
      updateHighImpedanceValue();
      if ( stereoLinked ) updateRightHighImpedanceValue();
    }
  }
  updateVolumeValue();
  if (stereoLinked)
    updatePanValue();
  updateMuteValue();
  updateMeters();

  count++;
  if (count >= 10) {
    count = 0;
    //printf("resetting count\n");
  }
}

void AudioFeatureControlWidget::setProgressBarColor(QProgressBar* pb, double value) {
  if (value < -6) {
    pb->setStyleSheet("QProgressBar{ border: 0px; background-color: #545454; border-radius: 0px; text-align: center; margin-right: 2px;} \
    QProgressBar::chunk { background-color: #11e000; }");
  }
  else if (value < -1) {
    pb->setStyleSheet("QProgressBar{ border: 0px; background-color: #545454; border-radius: 0px; text-align: center; margin-right: 2px;} \
    QProgressBar::chunk { background-color: #FFCC00; }");
  }
  else{
    pb->setStyleSheet("QProgressBar{ border: 0px; background-color: #545454; border-radius: 0px; text-align: center; margin-right: 2px;} \
    QProgressBar::chunk { background-color: #ff0033; }");
  }
}

void AudioFeatureControlWidget::updateMeters() {
  if ((audioFeatureSource->channelType(channelID) == 1) ||
      (audioFeatureSource->channelType(channelID) == 2)){
    int16_t meterCurrent1 = audioFeatureSource->meterCurrent(channelID);
    int16_t meterCurrent2 = 0;
    int16_t meterPng1 = MixerChannelWidget::toPixelsFromICA(((double)20 * log10((double)meterCurrent1 / 8192.0)) * 256.0);
    int16_t meterPng2 = 0;

    if (stereoLinked) {
      meterCurrent2 = audioFeatureSource->meterCurrent(channelID + 1);
      meterPng2 = MixerChannelWidget::toPixelsFromICA(((double)20 * log10((double)meterCurrent2 / 8192.0)) * 256.0);;
    }

    int minus40pixels = MixerChannelWidget::toPixelsFromICA( -40 * 256 );

    if (((double)20 * log10((double)meterCurrent1 / 8192.0)) < -40)
      meterPng1 = 0;
    if (((double)20 * log10((double)meterCurrent2 / 8192.0)) < - 40)
      meterPng2 = 0;

    meterPng1 = (int)floor((double)(meterPng1 - minus40pixels) * 300 / (300 - 75 - minus40pixels)); // max = 0 instead of 6
    meterPng2 = (int)floor((double)(meterPng2 - minus40pixels) * 300 / (300 - 75 - minus40pixels)); // and ignore under -40

    if ((((double)20 * log10((double)meterCurrent1 / 8192.0)) >= -1)) {
      turnOnClipping1();
    }
    if ((((double)20 * log10((double)meterCurrent2 / 8192.0)) >= -1)) {
      turnOnClipping2();
    }

//    //printf("meter1: %d, %d, %d | meter2: %d, %d, %d\n", meterPng1, meterCurrent1, (int)(((double)20 * log10((double)meterCurrent1 / 8192.0)) * 256.0),
//           meterPng2, meterCurrent2, (int)(((double)20 * log10((double)meterCurrent2 / 8192.0)) * 256.0));

    if (meterCurrent1 == 0 || meterPng1 <= 0) {
      meterBar1->setValue(0);
      setProgressBarColor(meterBar1, -20);
    }
    else {
      meterBar1->setValue(meterPng1);
      setProgressBarColor(meterBar1, ((double)20 * log10((double)meterCurrent1 / 8192.0)));
    }
    if (stereoLinked) {
      if (meterCurrent2 == 0 || meterPng2 <= 0) {
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

void AudioFeatureControlWidget::updateChannelNameLabel() {
  if (channelNameLabel) {
    if (stereoLinked) {
      channelNameLabel->setText(QString::number(channelID) + " & " + QString::number(channelID + 1));
    }
    else {
      channelNameLabel->setText(QString::number(channelID));
    }
  }

  channelNameLabel = 0;

  int trueChannelID = channelID;

  if (!audioFeatureSource->controllerName().compare("Headphones")) {
    trueChannelID += 4;
  }

  if ((!audioFeatureSource->controllerName().compare("Line Outputs")) ||
      (!audioFeatureSource->controllerName().compare("Headphones"))){

    MixerInterface* mixerInterface = new MixerInterface(device);

    const auto &audioGlobalParm = device->get<AudioGlobalParm>();

    QString title = "";
    QString title2 = "";

    int count = 3;
    for (Word aPid = 1; aPid <= audioGlobalParm.numAudioPorts(); ++aPid) {
      int numOutputs = mixerInterface->numberOutputs(aPid);
      for (int j = 1; j <= numOutputs; j++) {
        std::vector<int8_t> channels = mixerInterface->channelIDsForOutput(aPid, j);
        for (int8_t x : channels) {
          if (x == trueChannelID && aPid == audioPortID) {
            if (j % 2)
              title = portNames.at(count) + ":L";
            else
              title = portNames.at(count) + ":R";
          }
          if (stereoLinked) {
            if (x == (trueChannelID+1) && aPid == audioPortID) {
              if (j % 2)
                title2 = portNames.at(count) + ":L";
              else {
                title2 = portNames.at(count) + ":R";
              }
            }
          }
        }
        if (!(j % 2))
          count++;
      }
    }
    if (title == "" || title2 == "") {
      for (Word aPid = 1; aPid <= audioGlobalParm.numAudioPorts(); ++aPid) {
        auto &patchBay = device->get<AudioPatchbayParm>(aPid);
        auto flatList = patchBay.flatList();
        for (auto &f : flatList) {
          int p1 = f.outPortID;
          int c1 = f.outChannelNumber;
          int p2 = f.inPortID;
          int c2 = f.inChannelNumber;

          if (p1 > 0) {
            if (p2 == audioPortID && c2 == trueChannelID) {
              title = (portNames.at(p1 - 1) + ":" + QString::number(c1));
            }
            if (stereoLinked) {
              if (p2 == audioPortID && c2 == trueChannelID+1) {
                title2 = (portNames.at(p1 - 1) + ":" + QString::number(c1));
              }
            }
          }
        }
      }
    }

    if (title == "") {
      title = ("No SRC");
    }
    if (title2 == "") {
      title2 = ("No SRC");
    }

    if (stereoLinked)
      outputConfigPushButton->setText(title + " | " + title2);
    else
      outputConfigPushButton->setText(title);
  }
}

void AudioFeatureControlWidget::updateVolumeValue() {
  volumeSlider->blockSignals(true);
  if (audioFeatureSource->isVolumeAvailable(channelID)) {
    if ((audioFeatureSource->channelType(channelID) == 2 && (uint16_t)audioFeatureSource->volumeCurrent(channelID) == 0x8000 && volumeSlider->value() != volumeSlider->minimum()) || (volumeSlider->value() != audioFeatureSource->volumeCurrent(channelID) *
        IAudioControlFeatureSource::VolumedBConversionFactor)) {

      if (audioFeatureSource->channelType(channelID) == 2 && ((uint16_t)audioFeatureSource->volumeCurrent(channelID)) == 0x8000) {
        volumeSlider->setValue(volumeSlider->minimum());
        currentVolumeLabel->setValue(-100000);
      }
      else {
        volumeSlider->setValue(audioFeatureSource->volumeCurrent(channelID) *
                             IAudioControlFeatureSource::VolumedBConversionFactor/
                               (audioFeatureSource->volumeResolution(channelID)*
                                                           IAudioControlFeatureSource::VolumedBConversionFactor));
        currentVolumeLabel->setValue(audioFeatureSource->volumeCurrent(channelID) *
                                                  IAudioControlFeatureSource::VolumedBConversionFactor);
      }
    }
  } else {
    if (volumeSlider->value()!=0) {
      volumeSlider->setValue(0);
      currentVolumeLabel->setValue(-100001);
    }
  }
  volumeSlider->blockSignals(false);
}

void AudioFeatureControlWidget::updatePanValue() {
  panDial->blockSignals(true);
  if (audioFeatureSource->isVolumeAvailable(channelID)) {
    int value;
    if (audioFeatureSource->trimCurrent(channelID) < 0) {
      value = 0 - audioFeatureSource->trimCurrent(channelID);
    }
    else if (audioFeatureSource->trimCurrent(channelID + 1) < 0){
      value = audioFeatureSource->trimCurrent(channelID + 1);
    }
    else {
      value = 0;
    }
    if (panDial->value() != value *
        IAudioControlFeatureSource::VolumedBConversionFactor) {
      panDial->setValue(value *
                         IAudioControlFeatureSource::VolumedBConversionFactor/
                           (audioFeatureSource->volumeResolution(channelID)*
                                                       IAudioControlFeatureSource::VolumedBConversionFactor));
      if (audioFeatureSource->trimCurrent(channelID) < 0) {
        value = 0 - audioFeatureSource->trimCurrent(channelID);
        currentPanLabel->setText("<b>R" + QString::number(value * IAudioControlFeatureSource::VolumedBConversionFactor,'f',1).rightJustified(5, ' ').replace(" ", "&nbsp;") + "</b>");
      }
      else if (audioFeatureSource->trimCurrent(channelID + 1) < 0){
        value = 0 - audioFeatureSource->trimCurrent(channelID + 1);
        currentPanLabel->setText("<b>L" + QString::number(value * IAudioControlFeatureSource::VolumedBConversionFactor,'f',1).rightJustified(5, ' ').replace(" ", "&nbsp;") + "</b>");
      }
      else {
        value = 0;
        currentPanLabel->setText("<b>" + QString("  C  ").replace(" ", "&nbsp;") + "</b>");
      }
    }
  } else {
    if (panDial->value()!=0) {
      panDial->setValue(0);
      currentPanLabel->setText("<b>" + QString("N/A").rightJustified(5, ' ').replace(" ", "&nbsp;") + "</b>");
    }
  }
  panDial->blockSignals(false);
}

void AudioFeatureControlWidget::updateStereoLinkValue() {
/*  if (!(channelID % 2) || stereoLinked) {
    stereoLinkPushButton->blockSignals(true);
    if (audioFeatureSource->isStereoLinkAvailable(channelID)) {
      if (audioFeatureSource->stereoLink(channelID) != (stereoLinkPushButton->isChecked())) {
        stereoLinkPushButton->setChecked(audioFeatureSource->stereoLink(channelID));
      }
    } else {
      if (stereoLinkPushButton->isVisible())
        stereoLinkPushButton->setVisible(false);
    }
    stereoLinkPushButton->blockSignals(false);
  }*/
}

void AudioFeatureControlWidget::updateMuteValue() {
  mutePushButton->blockSignals(true);
  if (audioFeatureSource->isMuteAvailable(channelID)) {
    if (audioFeatureSource->mute(channelID) != (mutePushButton->isChecked())) {
      mutePushButton->setChecked(audioFeatureSource->mute(channelID));
    }
  } else {
    if (mutePushButton->isVisible())
      mutePushButton->setVisible(false);
  }
  mutePushButton->blockSignals(false);
}

void AudioFeatureControlWidget::updatePhantomValue() {
  phantomPowerPushButton->blockSignals(true);
  if (audioFeatureSource->isPhantomPowerAvailable(channelID)) {
    if (audioFeatureSource->phantomPower(channelID) != (phantomPowerPushButton->isChecked())) {
      phantomPowerPushButton->setChecked(audioFeatureSource->phantomPower(channelID));
    }
  } else {
    if (phantomPowerPushButton->isVisible())
      phantomPowerPushButton->setVisible(false);
  }
  phantomPowerPushButton->blockSignals(false);
}

void AudioFeatureControlWidget::updateHighImpedanceValue() {
  highImpedancePushButton->blockSignals(true);
  if (audioFeatureSource->isHighImpedanceAvailable(channelID)) {
    if (audioFeatureSource->highImpedance(channelID) != (highImpedancePushButton->isChecked())) {
      highImpedancePushButton->setChecked(audioFeatureSource->highImpedance(channelID));
    }
  } else {
    if (highImpedancePushButton->isVisible())
      highImpedancePushButton->setVisible(false);
  }
  highImpedancePushButton->blockSignals(false);
}

void AudioFeatureControlWidget::updateRightPhantomValue()
{
  if (stereoLinked) {
    rightPhantomPowerPushButton->blockSignals(true);
    if (audioFeatureSource->isPhantomPowerAvailable(channelID + 1)) {
      if (audioFeatureSource->phantomPower(channelID + 1) != (rightPhantomPowerPushButton->isChecked())) {
        rightPhantomPowerPushButton->setChecked(audioFeatureSource->phantomPower(channelID + 1));
      }
    } else {
      if (rightPhantomPowerPushButton->isVisible())
        rightPhantomPowerPushButton->setVisible(false);
    }
    rightPhantomPowerPushButton->blockSignals(false);
  }
}

void AudioFeatureControlWidget::updateRightHighImpedanceValue()
{
  if (stereoLinked) {
    rightHighImpedancePushButton->blockSignals(true);
    if (audioFeatureSource->isHighImpedanceAvailable(channelID + 1)) {
      if (audioFeatureSource->highImpedance(channelID + 1) != (rightHighImpedancePushButton->isChecked())) {
        rightHighImpedancePushButton->setChecked(audioFeatureSource->highImpedance(channelID + 1));
      }
    } else {
      if (rightHighImpedancePushButton->isVisible())
        rightHighImpedancePushButton->setVisible(false);
    }
    rightHighImpedancePushButton->blockSignals(false);
  }

}

void AudioFeatureControlWidget::refreshWidget() {
  // do refresh

  updateAll();
}

void AudioFeatureControlWidget::refreshMeters()
{
  updateMeters();
}
