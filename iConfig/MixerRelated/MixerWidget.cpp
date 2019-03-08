/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "MixerWidget.h"
#include "MixerPortParm.h"
#include "AudioGlobalParm.h"
#include "AudioPortParm.h"
#include "AudioDeviceParm.h"
#include "AudioControlParm.h"
#include "AudioControlDetail.h"
#include "IRefreshWidget.h"

#include "MixerInterface.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSizePolicy>
#include <QPushButton>
#include <QInputDialog>
#include <QDialog>

using namespace GeneSysLib;

const QString MixerWidget::DefaultWindowTitle =
    tr("Audio Mixer");

CommandQList MixerWidget::Query() {
  CommandQList query;

  query << Command::RetAudioGlobalParm;
  query << Command::RetAudioPortParm;
  query << Command::RetAudioPatchbayParm;
  query << Command::RetAudioDeviceParm;
  query << Command::RetAudioControlParm;
  query << Command::RetAudioControlDetail;
  query << Command::RetAudioControlDetailValue;
  query << Command::RetAudioPortMeterValue;

  query << Command::RetMixerParm;
  query << Command::RetMixerPortParm;
  query << Command::RetMixerInputParm;
  query << Command::RetMixerOutputParm;
  query << Command::RetMixerInputControl;
  query << Command::RetMixerOutputControl;
  query << Command::RetMixerInputControlValue;
  query << Command::RetMixerOutputControlValue;
  query << Command::RetMixerMeterValue;

  return query;
}

static Word savedAudioPortID = 0;
static Byte savedMixerOutputNumber = 0;

MixerWidget::MixerWidget(DeviceInfoPtr device, QWidget *parent)
  : RefreshObject(parent), device(device) {

    inputTabWidget = NULL;
    outputTabWidget = NULL;

    inputTabHolder = NULL;
    outputTabHolder = NULL;

  mixerScrollLayout = new QHBoxLayout();
  mixerScrollLayout->setSpacing(0);
  mixerScrollLayout->setContentsMargins(0,0,0,0);

  mixerScrollWidget = 0;
  minimumSize = 0;

  auto hBox = new QHBoxLayout();
  hBox->setSpacing(0);
  setLayout(hBox);
  setContentsMargins(0,0,0,0);

  setWindowTitle(DefaultWindowTitle);

  mixerInterface = new MixerInterface(device);
  mixerInputInterface = new MixerInputInterface(device);
  mixerOutputInterface = new MixerOutputInterface(device);

  mixerTabWidget = 0;

  audioPortID = savedAudioPortID;
  mixerOutputNumber = savedMixerOutputNumber;

  //printf("restoring saved values: %d/%d\n", audioPortID, mixerOutputNumber);

  const auto& audioGlobalParm = device->get<AudioGlobalParm>();
  if (audioPortID > audioGlobalParm.numAudioPorts()) {
    savedAudioPortID = audioPortID = 0;
    savedAudioPortID = audioPortID = audioGlobalParm.numAudioPorts(); // default analog
  }
  else if (audioPortID < 1) {
    savedAudioPortID = audioPortID = audioGlobalParm.numAudioPorts(); // default analog
  }
  else {
    if (mixerOutputNumber > mixerInterface->numberOutputs(audioPortID)) {
      savedMixerOutputNumber = mixerOutputNumber = 0;
    }
    else if (mixerOutputNumber < 1) {
      savedMixerOutputNumber = mixerOutputNumber = 0;
    }
  }
//  if (audioPortID == 0 || mixerOutputNumber == 0) {
//    savedAudioPortID = audioPortID = 0;
//    savedMixerOutputNumber = mixerOutputNumber = 0;
//  }

  //printf("saved values: %d/%d\n", audioPortID, mixerOutputNumber);

  //buildPortComboBox();

  tabLayout = new QVBoxLayout();
  tabLayout->setSpacing(0);
  tabLayout->setContentsMargins(0,0,0,0);
  tabHolderTop = new QWidget();
  tabHolderTop->setLayout(tabLayout);
  tabHolderTop->setContentsMargins(0,0,0,0);

  QSizePolicy exp = QSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
  tabHolderTop->setSizePolicy(exp);

  buildInputControls();
  mixerScrollLayout->addWidget(tabHolderTop);
  buildOutputControls();

  buildMixerHeader();
  buildMixerTabs();
  buildMixerPortWidget();

//  QWidget* belowMixerSpace = new QWidget();
//  belowMixerSpace->setMinimumHeight(0);
  tabHolderTop->setObjectName("tabHolderTop");
  tabHolderTop->setStyleSheet("QWidget#tabHolderTop { background-color: #424242; }");
//  belowMixerSpace->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
//  belowMixerSpace->setContentsMargins(0,0,0,0);
//  tabLayout->addWidget(belowMixerSpace);
//  tabLayout->setAlignment(belowMixerSpace, Qt::AlignTop);

  buildMixerEditButton();

  mixerScrollWidget = new QWidget();
  mixerScrollWidget->setLayout(mixerScrollLayout);
  mixerScrollWidget->setObjectName("mixerScrollWidget");
  mixerScrollWidget->setStyleSheet("QWidget#mixerScrollWidget {background: #202020;}");

  mixerScroll = new QScrollArea();
  mixerScroll->setFrameShape(QFrame::NoFrame);
  mixerScroll->setObjectName("mixerScroll");
  mixerScroll->setStyleSheet("QScrollArea#mixerScroll { background-color: #202020; }");
  QSizePolicy scrollSP = QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

  mixerScroll->setWidget(mixerScrollWidget);
  mixerScroll->setSizePolicy(scrollSP);
  layout()->setMargin(0);
  layout()->setSpacing(0);
  this->setObjectName("mixerWidget");
  this->setStyleSheet("QWidget {color: #ffffff}");

  layout()->addWidget(mixerScroll);

}

MixerWidget::~MixerWidget() {
  remove(tabLayout);
}

void MixerWidget::handleEditButton() {

  bool ok = true;
  QString currentName = QString::fromStdString(mixerInterface->mixName(audioPortID, mixerOutputNumber));
  QString newName = QInputDialog::getText(0, tr("New mix name"),tr("Name:"), QLineEdit::Normal,currentName, &ok);

  if (ok) {
    mixerInterface->mixName(audioPortID, mixerOutputNumber, newName.toStdString());
    int which = -1;
    for (int i = 0; i < outputLookup.size(); i++) {
      if (outputLookup.at(i).first == audioPortID && outputLookup.at(i).second == mixerOutputNumber) {
        which = i;
        break;
      }
    }
    if (which != -1) {
      QString prefix;

      const auto &audioPortParm = device->get<AudioPortParm>(audioPortID);
      prefix = QString::fromStdString(audioPortParm.portName()) + ": ";

      mixerTabWidget->setTabText(which, prefix + QString::fromStdString(mixerInterface->mixName(audioPortID, mixerOutputNumber)));
    }
  }
  this->refreshWidget();
}

//void MixerWidget::notifyStereoLinkHappened()
//{
//  refreshList.clear();
//  readdAnalogControlsToRefreshList();
//  if (mixerOutputNumber % 2 == 0) {
//    mixerOutputNumber = mixerOutputNumber - 1;
//  }
//  QTimer::singleShot(400, this, SLOT(redrawAll()));
//}

//void MixerWidget::redrawAll() {
//  remove(tabLayout);
//  mixerTabWidget = 0;
//  //buildMixerHeader();
//  buildMixerTabs();
//  buildMixerPortWidget();
//  //printf("calling doAdjustSize();\n");
//  doAdjustSize();
//  //buildMixerEditButton();
//}

void MixerWidget::buildPortComboBox() {
/*  auto hBoxLayout = new QHBoxLayout();
  hBoxLayout->setSpacing(4);
  hBoxLayout->setContentsMargins(4, 4, 4, 4);
  hBoxLayout->setAlignment(Qt::AlignTop | Qt::AlignCenter);
  portComboBox = new QComboBox();
  auto selectLabel = new QLabel("Audio Domain: ");

  const auto &globalParm = device->get<AudioGlobalParm>();

  for (int i = 1; i <= globalParm.numAudioPorts(); i++) {
    const auto &audioPortParm = device->get<AudioPortParm>(i);
    portComboBox->addItem(QString::fromStdString(audioPortParm.portName()));
    if (audioPortID == 0)
      if (mixerInterface->numberOutputs(i))
        audioPortID = i;
  }
  portComboBox->blockSignals(false);
  connect(portComboBox, SIGNAL(currentIndexChanged(int)),this, SLOT(portComboBoxChanged(int)));

  hBoxLayout->addWidget(selectLabel);
  hBoxLayout->addWidget(portComboBox);

  auto hBoxWidget = new QWidget();
  QSizePolicy mini = QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  hBoxWidget->setLayout(hBoxLayout);
  hBoxWidget->setSizePolicy(mini);
  layout()->addWidget(hBoxWidget);*/
}

void MixerWidget::buildMixerEditButton()
{
  editButton = new QPushButton();
  editButton->setMinimumWidth(146);
  editButton->setMaximumWidth(146);
  editButton->setMinimumHeight(21);
  editButton->setMaximumHeight(21);
  editButton->setStyleSheet(QString("QPushButton {background-image: url(:/mixer_graphics/Images/mixer_graphics/btn_edit_mix_name.png); border: 0px;}"));

  QHBoxLayout *lay = new QHBoxLayout();
  QWidget *wid = new QWidget();
  lay->addWidget(editButton); lay->setMargin(0);
  wid->setLayout(lay); wid->setContentsMargins(0,0,10,10);
  layout()->addWidget(wid);
  layout()->setAlignment(wid, Qt::AlignRight);

  connect(editButton,SIGNAL(clicked()),this,SLOT(handleEditButton()));

  tabLayout->addWidget(wid);
  tabLayout->setAlignment(wid,Qt::AlignRight);
}

void MixerWidget::buildInputControls()
{
    //PlayAudio support zx 2017-02-22
    int nInputControlID = 0;

    for(int i = 1; i < 3; ++i)
    {
        bool bHavePort = device->contains<AudioControlParm>(audioPortID, i);
        if(bHavePort == true) {
            const AudioControlParm &audioControl = device->get<AudioControlParm>(audioPortID, i);
            if(0 < audioControl.numDetails())
            {
                if(device->contains<AudioControlDetail>(audioPortID, i, 1) == true)
                {
                    const AudioControlDetail &audioControlDetail = device->get<AudioControlDetail>(audioPortID, i, 1);
                    if(audioControlDetail.feature().channelType() == 1)
                    {
                        nInputControlID = i;
                        break;
                    }
                }
            }
        }
    }

    if(nInputControlID == 0)
        return;

  auto tabLayout = new QVBoxLayout();
  tabLayout->setSpacing(0);
  tabLayout->setContentsMargins(0,0,0,0);
  QHBoxLayout* inputBoxLayout = new QHBoxLayout();
  inputBoxLayout->setContentsMargins(0,0,0,0);
  inputBoxLayout->setSpacing(0);

  QWidget* inputBox = new QWidget();

/*  AudioControlFeatureSourcePtr source =
      AudioControlFeatureSourcePtr(new AudioControlFeatureSource(
          device, 3, 1));
  AudioChannelsControlWidget* audioChannelWidget =  new AudioChannelsControlWidget(source, device, 3, this);
*/
  AudioControlFeatureSourcePtr source =
      AudioControlFeatureSourcePtr(new AudioControlFeatureSource(
          device, audioPortID, nInputControlID));
  AudioChannelsControlWidget* audioChannelWidget =  new AudioChannelsControlWidget(source, device, audioPortID, this);
  audioChannelWidget->setObjectName("audioChannelWidget");
  audioChannelWidget->setStyleSheet("AudioChannelsControlWidget { background-color: #323232; }");

  inputBoxLayout->addWidget(audioChannelWidget);
  refreshList.push_back(audioChannelWidget);
  analogList.push_back(audioChannelWidget);
  inputList.push_back(audioChannelWidget);

  inputBox->setLayout(inputBoxLayout);
  inputBox->setObjectName("inputBox");
  inputBox->setStyleSheet("QWidget#inputBox { background-color: #323232; }");

  inputTabWidget = new QTabWidget();
  inputTabWidget->addTab(inputBox, "Analog Inputs");

  QTabBar *tabBar = inputTabWidget->findChild<QTabBar *>(QLatin1String("qt_tabwidget_tabbar"));

#ifdef _WIN32
  inputTabWidget->setStyleSheet("QTabWidget::pane { left:0px;} QTabWidget::tab-bar { left: 0px; alignment: left; } QTabBar::tab { alignment: left; height: 20px; width: 150px; font-size: 16px; color: #ffffff; padding: 5px 5px 48px 5px; } QTabBar::tab:hover { background: transparent; color: #ffffff; } QTabBar::tab:selected { background-color: transparent; color: #ffffff; }");
#else
  inputTabWidget->setStyleSheet("QTabWidget::pane { left:0px;} QTabWidget::tab-bar { left: 0px; alignment: left; } QTabBar::tab { alignment: left; height: 20px; width: 150px; font-size: 16px; color: #ffffff; padding: 5px 5px 48px 5px; } QTabBar::tab:hover { background: transparent; color: #ffffff; } QTabBar::tab:selected { background-color: transparent; color: #ffffff; }");
#endif

  inputTabWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

  tabLayout->addWidget(inputTabWidget);

  inputTabHolder = new QWidget();
  inputTabHolder->setLayout(tabLayout);
  inputTabHolder->setContentsMargins(0,0,0,0);

  mixerScrollLayout->addWidget(inputTabHolder);
}

void MixerWidget::buildOutputControls()
{
  auto tabLayout = new QVBoxLayout();
  tabLayout->setSpacing(0);
  tabLayout->setContentsMargins(0,0,0,0);
  QHBoxLayout* outputBoxLayout = new QHBoxLayout();
  outputBoxLayout->setContentsMargins(0,0,0,0);
  outputBoxLayout->setSpacing(0);

  QWidget* outputBox = new QWidget();

  int nTotalOutPutChannels = 0;

  for (int i = 1; i <=3; i ++) {
      bool bHavePort = device->contains<AudioControlParm>(audioPortID, i);
      if(bHavePort == true) {
          const AudioControlParm &audioControl = device->get<AudioControlParm>(audioPortID, i);
          if(0 < audioControl.numDetails())
          {
              if(device->contains<AudioControlDetail>(audioPortID, i, 1) == true)
              {
                  const AudioControlDetail &audioControlDetail = device->get<AudioControlDetail>(audioPortID, i, 1);
                  if(audioControlDetail.feature().channelType() == 2)
                  {
                      nTotalOutPutChannels += audioControl.numDetails();
                  }
              }
          }
      }
  }

  for (int i = 1; i <=3; i ++) {
  //for (int i = 2; i <=3; i ++) {
/*    AudioControlFeatureSourcePtr source =
        AudioControlFeatureSourcePtr(new AudioControlFeatureSource(
            device, 3, i));
    auto audioChannelWidget =  new AudioChannelsControlWidget(source, device, 3, this);
*/
      bool bHavePort = device->contains<AudioControlParm>(audioPortID, i);
      if(bHavePort == true) {
          const AudioControlParm &audioControl = device->get<AudioControlParm>(audioPortID, i);
          if(0 < audioControl.numDetails())
          {
              if(device->contains<AudioControlDetail>(audioPortID, i, 1) == true)
              {
                  const AudioControlDetail &audioControlDetail = device->get<AudioControlDetail>(audioPortID, i, 1);
                  if(audioControlDetail.feature().channelType() != 2)
                  {
                      continue;
                  }
              }
          }
      }




    AudioControlFeatureSourcePtr source = AudioControlFeatureSourcePtr(new AudioControlFeatureSource(device, audioPortID, i));
    auto audioChannelWidget =  new AudioChannelsControlWidget(source, device, audioPortID, this, nTotalOutPutChannels);

    outputBoxLayout->addWidget(audioChannelWidget);

    refreshList.push_back(audioChannelWidget);
    analogList.push_back(audioChannelWidget);
    outputList.push_back(audioChannelWidget);
  }

  outputBox->setLayout(outputBoxLayout);
  outputBox->setObjectName("outputBox");
  outputBox->setStyleSheet("QWidget#outputBox { background-color: #323232; }");

  QLabel *icmLogo = new QLabel();
  QPixmap *icmPix = new QPixmap(":/mixer_graphics/Images/mixer_graphics/icm_logo.png");
  icmLogo->setPixmap(*icmPix);

  QVBoxLayout *vBoxWithImageLayout = new QVBoxLayout();
  vBoxWithImageLayout->setContentsMargins(0,0,10,10);
  vBoxWithImageLayout->setSpacing(0);
  vBoxWithImageLayout->addWidget(outputBox);
  vBoxWithImageLayout->addWidget(icmLogo);
  vBoxWithImageLayout->setAlignment(icmLogo, Qt::AlignRight);

  QWidget *vBoxWithImage = new QWidget();
  vBoxWithImage->setContentsMargins(0,0,0,0);
  vBoxWithImage->setLayout(vBoxWithImageLayout);
  vBoxWithImage->setObjectName("vBoxWithImage");
  vBoxWithImage->setStyleSheet("QWidget#vBoxWithImage {background-color: #323232;}");

  outputTabWidget = new QTabWidget();
  outputTabWidget->addTab(vBoxWithImage, "Analog Outputs");
#ifdef _WIN32
  outputTabWidget->setStyleSheet("QTabWidget::pane { left:0px;} QTabWidget::tab-bar { left: 0px; alignment: left; } QTabBar::tab { alignment: left; height: 20px; width: 150px; font-size: 16px; background: transparent; color: #ffffff; padding: 5px 5px 48px 5px; } QTabBar::tab:hover { background: transparent; color: #ffffff; } QTabBar::tab:selected { background: transparent; color: #ffffff; }");
#else
  outputTabWidget->setStyleSheet("QTabWidget::pane { left:0px;} QTabWidget::tab-bar { left: 0px; alignment: left; } QTabBar::tab { alignment: left; height: 20px; width: 150px; font-size: 16px; background: transparent; color: #ffffff; padding: 5px 5px 48px 5px; } QTabBar::tab:hover { background: transparent; color: #ffffff; } QTabBar::tab:selected { background: transparent; color: #ffffff; }");
#endif
  tabLayout->addWidget(outputTabWidget);
  outputTabHolder = new QWidget();
  outputTabHolder->setLayout(tabLayout);
  outputTabHolder->setContentsMargins(0,0,0,0);

  mixerScrollLayout->addWidget(outputTabHolder);
}

void MixerWidget::readdAnalogControlsToRefreshList()
{
  for (auto *refresh : analogList) {
    refreshList.push_back(refresh);
  }
}

void MixerWidget::remove(QLayout* layout)
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

void MixerWidget::mixerTabChanged(int which) {
  mixerTabWidget->blockSignals(true);
  if (outputLookup.size() > which) {
    refreshList.clear();
    for (int i = 0; i < headerMeters.size(); i++) {
      refreshList.push_back(headerMeters.at(i));
    }

    readdAnalogControlsToRefreshList();
    if (mixerPortWidget) {
      minimumSize = tabHolderTop->width();
      remove(mixerPortWidget->layout());
      delete editButton->parentWidget();
      //delete editButton;
      delete mixerPortWidget;
    }
    mixerTabWidget->clear();
    savedAudioPortID = audioPortID = outputLookup.at(which).first;
    savedMixerOutputNumber = mixerOutputNumber = outputLookup.at(which).second;
    buildMixerTabs();
    buildMixerPortWidget();
    buildMixerEditButton();
  }
  mixerTabWidget->blockSignals(false);
}

void MixerWidget::buildMixerHeader() {
  QLabel* headerLabel = new QLabel("Sub Mix / Bussing");
#ifdef _WIN32
  headerLabel->setStyleSheet("QLabel {font-size: 16px; background: #202020; color: #ffffff; padding: 5px 5px 5px 5px;}");
#else
  headerLabel->setStyleSheet("QLabel {font-size: 16px; background: #202020; color: #ffffff; padding: 5px 5px 9px 5px;}");
#endif
  QHBoxLayout* meters = new QHBoxLayout();
  meters->setSpacing(0);
  meters->setContentsMargins(0,0,0,0);
  QWidget* meterWidget = new QWidget();
  meterWidget->setContentsMargins(0,0,0,0);
  meterWidget->setLayout(meters);
  meterWidget->setObjectName("meterWidget");
  meterWidget->setStyleSheet("QWidget#meterWidget {background-color: #202020;}");

  QWidget* spacer = new QWidget();
  spacer->setMinimumHeight(10);
  spacer->setMaximumHeight(10);
  spacer->setContentsMargins(0,0,0,0);
  spacer->setObjectName("spacer");
  spacer->setStyleSheet("QWidget#spacer { background-color: #202020; }");

  tabLayout->addWidget(headerLabel);
  tabLayout->addWidget(meterWidget);
  tabLayout->addWidget(spacer);

  int numMixes = 0;
  const auto &globalParm = device->get<AudioGlobalParm>();
  for (int i = 1; i <= globalParm.numAudioPorts(); i++) {
    for (int j = 1; j <= mixerInterface->numberOutputs(i); j = j+2) {
      HorizontalMeterWidget* headerMeter = new HorizontalMeterWidget(device,i,j,this);
      headerMeter->setMaximumWidth(100);
      meters->addWidget(headerMeter);
      refreshList.push_back(headerMeter);
      headerMeters.push_back(headerMeter);
    }
  }
}

void MixerWidget::buildMixerPortWidget() {
  if ((audioPortID != 0)  && (mixerOutputNumber != 0)) {
    mixerPortWidget = new MixerPortWidget(device, audioPortID, mixerOutputNumber, this);

    mixerPortWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(mixerPortWidget);
    mixerTabWidget->currentWidget()->setLayout(layout);
    refreshList.push_back(mixerPortWidget);

    tabHolderTop->setMinimumHeight(625);
    mixerPortWidget->setMinimumHeight(625);
    tabHolderTop->setMinimumWidth(mixerPortWidget->minimumWidth());
    tabHolderTop->setMaximumWidth(mixerPortWidget->minimumWidth());
  }
}

void MixerWidget::doAdjustSize() {
  if (inputTabHolder) {
    int size = 0;
    for (auto i : inputList) {
      size += i->minimumWidth();
    }
    inputTabHolder->setMinimumWidth(size);
    inputTabHolder->setMaximumWidth(size);
    inputTabHolder->adjustSize();
//    inputTabWidget->setStyleSheet("QTabWidget::pane { left:0px;} QTabWidget::tab-bar { left: 0px; alignment: left; } QTabBar::tab { text-align: left; alignment: left; height: 20px; width: " + QString::number(size + 10) + "px; font-size: 20px; background: transparent; color: #ffffff; padding: 5px 5px 48px 5px; } QTabBar::tab:hover { background: transparent; color: #ffffff; } QTabBar::tab:selected { background: transparent; color: #ffffff; }");
  }

  if (outputTabHolder) {
    int size = 0;
    for (auto i : outputList) {
      size += i->minimumWidth();
    }
    outputTabHolder->setMinimumWidth(size);
    outputTabHolder->setMaximumWidth(size);
    outputTabHolder->adjustSize();
//    outputTabWidget->setStyleSheet("QTabWidget::pane { left:0px;} QTabWidget::tab-bar { left: 0px; alignment: left; } QTabBar::tab { alignment: left; height: 20px; width: " + QString::number(size + 10) + "px; font-size: 20px; background: transparent; color: #ffffff; padding: 5px 5px 48px 5px; } QTabBar::tab:hover { background: transparent; color: #ffffff; } QTabBar::tab:selected { background: transparent; color: #ffffff; }");
  }

  if (mixerPortWidget) {
    int numMixes = 0;
    const auto &globalParm = device->get<AudioGlobalParm>();

    for (int i = 1; i <= globalParm.numAudioPorts(); i++) {
      numMixes += mixerInterface->numberOutputs(i) / 2;
    }

    int mixWidth = (int)floor(mixerPortWidget->minimumWidth() / numMixes);
    QString stringMixWidth = QString::number(mixWidth);
    QTabBar *tabBar = mixerTabWidget->findChild<QTabBar *>(QLatin1String("qt_tabwidget_tabbar"));
    tabBar->setStyleSheet("QTabBar { background-color: #202020; } QTabBar::tab { border-top-left-radius: 5px; border-top-right-radius: 5px; font-size: 12px; height: 15px; width: " + stringMixWidth + "px; background: #2B2B2B; padding: 5px 0px 5px 0px; } QTabBar::tab:hover { background-color: #323232; } QTabBar::tab:selected { background-color: #424242;}");

    for (int i = 0; i < headerMeters.size(); i++) {
      headerMeters.at(i)->setMinimumWidth(mixWidth);
      headerMeters.at(i)->setMaximumWidth(mixWidth);
    }

    tabHolderTop->setMinimumWidth(mixerPortWidget->minimumWidth());
    tabHolderTop->setMaximumWidth(mixerPortWidget->minimumWidth());

    if (mixerScrollWidget)
      mixerScrollWidget->adjustSize();
    mixerTabWidget->adjustSize();
    tabHolderTop->adjustSize();
    mixerPortWidget->adjustSize();

    tabHolderTop->setMinimumHeight(625);
    mixerPortWidget->setMinimumHeight(625);
  }
}


void MixerWidget::buildMixerTabs()
{
  if (!mixerTabWidget) {
    mixerTabWidget = new MyTabWidget();
    mixerTabWidget->setContentsMargins(0,0,0,0);
    auto newWidget = new QWidget();
    newWidget->setContentsMargins(0,0,0,0);
    mixerTabWidget->setCurrentWidget(newWidget);
    mixerTabWidget->setStyleSheet("QTabWidget::pane { left:0px; padding: 0px; spacing: 0px; margin:0px;} QTabWidget::tab-bar { background-color: #202020; } QTabBar { background-color: #202020; } QTabBar::tab { border-top-left-radius: 5px; border-top-right-radius: 5px; font-size: 12px; height: 15px; width: 120px; background: #2B2B2B; padding: 5px 0px 5px 0px; } QTabBar::tab:hover { background-color: #323232; } QTabBar::tab:selected { background-color: #424242;}");
  }
  mixerTabWidget->blockSignals(true);
  outputLookup.clear();
  const auto &globalParm = device->get<AudioGlobalParm>();

  int count = 0;
  for (int i = 1; i <= globalParm.numAudioPorts(); i++) {
    int numOutputs = mixerInterface->numberOutputs(i);
    if (mixerInterface->numberOutputs(globalParm.numAudioPorts()) > 0){ // TODO: request hack, if there's an analog mix
      if (audioPortID == 0) {  // and we have no port selected yet..
        savedAudioPortID = audioPortID = globalParm.numAudioPorts();
      }
    }
    for (int j = 1; j <= numOutputs; j++) {
      if (audioPortID == 0)
        savedAudioPortID = audioPortID = i;
      if (mixerOutputNumber == 0)
        savedMixerOutputNumber = mixerOutputNumber = j;
      QWidget* holder = new QWidget();
/*      QString icon;
      int whichIcon = (count % 5) + 1;
      if (i == 1) { // TODO: hack for now
        icon = "U1_" + QString::number(whichIcon);
      }
      else if (i == 2) {
        icon = "U2_" + QString::number(whichIcon);
      }
      else {
        icon = "A_" + QString::number(whichIcon);
      }
      mixerTabWidget->addTab(holder, QIcon(":/mixer_graphics/Images/tabicons/" + icon + ".png"), QString::fromStdString(mixerInterface->mixName(i,j)));
      */
      QString prefix;
      QColor color;
      int whichColour = (count % 5);

      const auto &audioPortParm = device->get<AudioPortParm>(i);
      prefix = QString::fromStdString(audioPortParm.portName()) + ": ";

      switch (whichColour) {
      case 0:
        color = QColor("#da3b77");
        break;
      case 1:
        color = QColor("#fffea2");
        break;
      case 2:
        color = QColor("#21bda2");
        break;
      case 3:
        color = QColor("#e13939");
        break;
      case 4:
        color = QColor("#7bff65");
        break;
      default:
        color = QColor("#ffffff");
        break;
      }

      mixerTabWidget->addTab(holder, prefix + QString::fromStdString(mixerInterface->mixName(i,j)));
      mixerTabWidget->tabBar()->setTabTextColor(count,color);
      outputLookup.append(QPair<Word,Byte>(i,j));

      if (audioPortID == i && mixerOutputNumber == j) {
        mixerTabWidget->setCurrentIndex(count);
      }

      if (mixerOutputInterface->stereoLink(i,j)) {
        j++;
      }

      count++;
    }
  }
  QSizePolicy exp = QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  mixerTabWidget->setSizePolicy(exp);
  tabLayout->addWidget(mixerTabWidget);
  connect(mixerTabWidget,SIGNAL(currentChanged(int)), this, SLOT(mixerTabChanged(int)),Qt::UniqueConnection);
  mixerTabWidget->blockSignals(false);
}

void MixerWidget::refreshWidget() {
//  mixerScrollWidget->setUpdatesEnabled(false);
  for (auto *refresh : refreshList) {
    refresh->refreshWidget();
  }
//  mixerScrollWidget->setUpdatesEnabled(true);
  emit doneAudioRefresh();
}

void MixerWidget::refreshMeters() {
//  mixerScrollWidget->setUpdatesEnabled(false);
  for (auto *refresh : refreshList) {
    refresh->refreshMeters();
  }
//  mixerScrollWidget->setUpdatesEnabled(true);
  emit doneMetersRefresh();
}
