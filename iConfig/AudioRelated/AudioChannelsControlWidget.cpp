/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "AudioChannelsControlWidget.h"
#include "IRefreshWidget.h"

#include <QHBoxLayout>
#include <QDebug>
 //Bugfixing for PlayAudio, zx-03-02
AudioChannelsControlWidget::AudioChannelsControlWidget(
    IAudioControlFeatureSourcePtr audioControlFeatureSource, DeviceInfoPtr device, Word audioPortID, MixerWidget *parent, int nTotalOutputChannel)
    : QWidget(parent), device(device), audioPortID(audioPortID), audioControlFeatureSource(audioControlFeatureSource), parent(parent), m_TotalOutputChannelInPort(nTotalOutputChannel)  {

  auto hBoxLayout = new QHBoxLayout();
  hBoxLayout->setContentsMargins(0, 0, 0, 0);
  hBoxLayout->setSpacing(0);
  //setAlignment(Qt::AlignTop | Qt::AlignHCenter);
  setLayout(hBoxLayout);
  setContentsMargins(0,0,0,0);

  signalMapper = new QSignalMapper(this);
  connect(signalMapper, SIGNAL(mapped(int)),
          this,         SLOT(linkButtonClicked(int)));

  //this->setStyleSheet("QGroupBox { margin-top: 40px; } ");
  //this->setStyleSheet("QGroupBox { border: 1px solid gray; border-radius: 5px; margin-top: 1.5em; } QGroupBox::title { subcontrol-origin: margin; left: 10px; margin-top:3px;}");
  buildAll();
}

void AudioChannelsControlWidget::notifyStereoLinkHappened()
{
//  stereoLinkHappened = true;
  refreshList.clear();
  this->setEnabled(false);

  QTimer::singleShot(100,this,SLOT(finishStereoLinkHappened()));
  //printf("AudioChannelsControlWidget notify'd\n");
}

void AudioChannelsControlWidget::finishStereoLinkHappened() {
  remove(layout());
  buildAll();
  adjustSize();
  parent->doAdjustSize();
  this->setEnabled(true);
}

void AudioChannelsControlWidget::remove(QLayout* layout)
{
  QLayoutItem* child;
  while((child = layout->takeAt(0)) != 0)
  {
    if(child->layout() != 0)
    {
      remove(child->layout());
    }

    if(child->widget() != 0)
    {
      delete child->widget();
    }

    delete child;
  }
}

void AudioChannelsControlWidget::buildAll() {
  //this->setTitle(QString(audioControlFeatureSource->controllerName().c_str()));
  buildChannelWidgets();
}

void AudioChannelsControlWidget::buildChannelWidgets() {
  QVBoxLayout* vBoxLayout;
  QHBoxLayout* hBoxLayout;

  int totalWidth = 0;
  size_t nOutputChannelCount = audioControlFeatureSource->numChannels();
//   printf("Current Output channels # %i, Total OutChannel %i\n", nOutputChannelCount, m_TotalOutputChannelInPort);
  qDebug() << "Current Output channels: " << nOutputChannelCount << " Totoal Output Channels: " << m_TotalOutputChannelInPort;

 // for (size_t ch = 1; ch <= nOutputChannelCount; ++ch) {
 for (size_t ch = 1; ch <= audioControlFeatureSource->numChannels(); ++ch) {
    auto controlWidget =
        new AudioFeatureControlWidget(audioControlFeatureSource, audioPortID, ch, device, this, m_TotalOutputChannelInPort);
    if (ch % 2) {
      vBoxLayout = new QVBoxLayout();
      hBoxLayout = new QHBoxLayout();
      vBoxLayout->setContentsMargins(0,0,0,0);
      hBoxLayout->setContentsMargins(0,0,0,0);
      vBoxLayout->setSpacing(0);
      hBoxLayout->setSpacing(0);
    }
    hBoxLayout->addWidget(controlWidget);
    totalWidth+=controlWidget->minimumWidth();
    hBoxLayout->setAlignment(controlWidget,Qt::AlignTop);
    refreshList.push_back(controlWidget);
    audioFeatureControls.push_back(controlWidget);
    if (audioControlFeatureSource->stereoLink(ch))
      ch++;
    if (!(ch % 2)) {
      QWidget* vBox = new QWidget();
      QWidget* hBox = new QWidget();
      hBox->setLayout(hBoxLayout);
      vBoxLayout->addWidget(hBox);

      QPushButton* linkButton = new QPushButton();
      connect(linkButton, SIGNAL(clicked()), signalMapper, SLOT(map()));
      signalMapper->setMapping(linkButton, ch - 1);
      linkButton->setEnabled(audioControlFeatureSource->isStereoLinkEditable(ch));
      linkButton->setMinimumWidth(32);
      linkButton->setMaximumWidth(32);
      linkButton->setMinimumHeight(19);
      linkButton->setMaximumHeight(19);
      if (audioControlFeatureSource->stereoLink(ch - 1))
        linkButton->setStyleSheet(QString("QPushButton {margin: 0px; padding: 0px; spacing: 0px; background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_link_on.png); border: 0px;}"));
      else
        linkButton->setStyleSheet(QString("QPushButton {margin: 0px; padding: 0px; spacing: 0px; background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_link_off.png); border: 0px;}"));

      QHBoxLayout* linkButtonHolderLayout = new QHBoxLayout();
      linkButtonHolderLayout->setSpacing(0);
      linkButtonHolderLayout->setContentsMargins(0,0,0,0);
      linkButtonHolderLayout->addWidget(linkButton);
      linkButtonHolderLayout->setAlignment(linkButton, Qt::AlignHCenter | Qt::AlignTop);
      QWidget* linkButtonHolder = new QWidget();
      linkButtonHolder->setLayout(linkButtonHolderLayout);
      linkButtonHolder->setContentsMargins(0,0,0,0);

      vBoxLayout->addWidget(linkButtonHolder);
      vBoxLayout->setAlignment(linkButtonHolder, Qt::AlignHCenter | Qt::AlignTop);

      vBox->setLayout(vBoxLayout);
      vBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
      layout()->addWidget(vBox);
    }
  }
  setMaximumWidth(totalWidth + 10);
  setMinimumWidth(totalWidth + 10);
}

void AudioChannelsControlWidget::linkButtonClicked(int whichButton) {
  audioControlFeatureSource->stereoLink(whichButton,!audioControlFeatureSource->stereoLink(whichButton));
  this->notifyStereoLinkHappened();
}

AudioFeatureControlWidget* AudioChannelsControlWidget::getAudioFeatureControlWidget(Byte channelID) {
  return audioFeatureControls[channelID - 1];
}

void AudioChannelsControlWidget::refreshWidget() {
  for (auto *refresh : refreshList) {
    refresh->refreshWidget();
  }
}


void AudioChannelsControlWidget::refreshMeters() {
  for (auto *refresh : refreshList) {
    refresh->refreshMeters();
  }
}
