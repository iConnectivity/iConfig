/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "MixerPortWidget.h"
#include "AudioPortParm.h"
#include "CommandList.h"
#include "IRefreshWidget.h"
#include "MixerWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>

using namespace GeneSysLib;

MixerPortWidget::MixerPortWidget(DeviceInfoPtr device, Word audioPortID, Byte mixerOutputNumber, MixerWidget *parent)
    : device(device), audioPortID(audioPortID), mixerOutputNumber(mixerOutputNumber), parent(parent) {

  topLayout = new QHBoxLayout();
  topLayout->setContentsMargins(0, 0, 0, 0);
  topLayout->setSpacing(0);
  topLayout->setAlignment(Qt::AlignTop);

  signalMapper = new QSignalMapper(this);
  connect(signalMapper, SIGNAL(mapped(int)),
          this,         SLOT(linkButtonClicked(int)));

  this->setLayout(topLayout);
  setContentsMargins(0,0,0,0);

  this->setObjectName("mixerPortWidget");
  this->setStyleSheet("QWidget#mixerPortWidget { background-color: #424242; }");

  mixerInterface = new MixerInterface(device);
  mixerInputInterface = new MixerInputInterface(device);
  mixerOutputInterface = new MixerOutputInterface(device);

  refreshable = true;

  doneBuilding = false;
  buildAll();
  savedNumberOfOutputs = 0;
  doneBuilding = true;

  QTimer::singleShot(100,this, SLOT(callDoAdjustSize()));
}

MixerPortWidget::~MixerPortWidget() {
  clear();
}

void MixerPortWidget::callDoAdjustSize() {
  parent->doAdjustSize();
}

void MixerPortWidget::paintEvent(QPaintEvent *)
{
  QStyleOption opt;
  opt.init(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void MixerPortWidget::buildAll() {
  const auto &audioPortParm = device->get<AudioPortParm>(audioPortID);

  //this->setTitle(QString::fromStdString(audioPortParm.portName()) + " : " +
  //               QString::fromStdString(mixerInterface->mixName(audioPortID,mixerOutputNumber)));
  buildChannelWidgets();
}

void MixerPortWidget::handleDoneButton() {
  notifyChannelChange();
}

void MixerPortWidget::notifyChannelChange() {
  remove(layout());
  refreshList.clear();
  savedInputs.clear();
  savedNumberOfOutputs = 0;
  buildChannelWidgets();

  refreshable = true;
}

void MixerPortWidget::notifyStereoLinkHappened()
{
  stereoLinkHappened = true;
  refreshList.clear();
  device->rereadMixerControls();
  this->setEnabled(false);
  //parent->notifyStereoLinkHappened();
  //printf("MixerPortWidget notify'd\n");
}

void MixerPortWidget::linkButtonClicked(int whichButton) {
  mixerInputInterface->stereoLink(audioPortID, mixerOutputNumber, whichButton, !mixerInputInterface->stereoLink(audioPortID, mixerOutputNumber, whichButton));
  this->notifyStereoLinkHappened();
}

void MixerPortWidget::buildChannelWidgets() {
  topLayout->setSpacing(0);
  setContentsMargins(0,0,0,0);

  auto hBoxInputLayout = new QHBoxLayout();
  hBoxInputLayout->setContentsMargins(0,0,0,0);
  hBoxInputLayout->setSpacing(0);
  hBoxInputLayout->setAlignment(Qt::AlignTop);

  auto hBoxOutputLayout = new QHBoxLayout();
  hBoxOutputLayout->setContentsMargins(0,0,0,0);
  hBoxOutputLayout->setSpacing(0);
  hBoxOutputLayout->setAlignment(Qt::AlignTop);

  //this->setStyleSheet("QGroupBox { margin-top: 40px; } ");

  QVBoxLayout* vBoxLayout;
  QHBoxLayout* hBoxLayout;

  for (int inCh = 1; inCh <= mixerInterface->numberInputs(audioPortID); ++inCh) {
    auto channelWidget =
        new MixerChannelWidget(device, audioPortID, inCh, mixerOutputNumber, in, this);
    if (inCh % 2) {
      vBoxLayout = new QVBoxLayout();
      hBoxLayout = new QHBoxLayout();
      vBoxLayout->setContentsMargins(0,0,0,0);
      hBoxLayout->setContentsMargins(0,0,0,0);
      vBoxLayout->setSpacing(0);
      hBoxLayout->setSpacing(0);
    }

    hBoxLayout->addWidget(channelWidget);
    hBoxLayout->setAlignment(channelWidget,Qt::AlignTop | Qt::AlignHCenter);

    refreshList.push_back(channelWidget);
    channelWidgets.push_back(channelWidget);
    if (mixerInterface->audioPortIDForInput(audioPortID,inCh) != 0) {
      savedInputs.append(inCh);
    }

    if (mixerInputInterface->stereoLink(audioPortID, mixerOutputNumber, inCh)) {
      inCh++;
    }
    if (!(inCh % 2) || (inCh == mixerInterface->numberInputs(audioPortID))) {
      QWidget* vBox = new QWidget();
      QWidget* hBox = new QWidget();
      hBox->setLayout(hBoxLayout);
      hBox->setContentsMargins(0,0,0,0);
      vBoxLayout->addWidget(hBox);
      if (!(inCh % 2)) {
        QPushButton* linkButton = new QPushButton();
        connect(linkButton, SIGNAL(clicked()), signalMapper, SLOT(map()));
        signalMapper->setMapping(linkButton, inCh - 1);
        linkButton->setEnabled(mixerInputInterface->isStereoLinkEditable(audioPortID));
        linkButton->setMinimumWidth(32);
        linkButton->setMaximumWidth(32);
        linkButton->setMinimumHeight(19);
        linkButton->setMaximumHeight(19);
        if (mixerInputInterface->stereoLink(audioPortID, mixerOutputNumber, inCh - 1))
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
      }
      vBoxLayout->setAlignment(hBox, Qt::AlignHCenter | Qt::AlignTop);
      vBox->setLayout(vBoxLayout);
      vBox->setContentsMargins(0,0,0,0);

      if (!(inCh % 2))  {
        if (mixerInputInterface->stereoLink(audioPortID, mixerOutputNumber, inCh - 1)) {
          hBox->setMinimumWidth(90);
          hBox->setMaximumWidth(90);
          vBox->setMinimumWidth(90);
          vBox->setMaximumWidth(90);
        }
        else {
          hBox->setMinimumWidth(160);
          hBox->setMinimumWidth(160);
          vBox->setMaximumWidth(160);
          vBox->setMaximumWidth(160);
        }
      }
      else {
        hBox->setMinimumWidth(60);
        hBox->setMinimumWidth(60);
        vBox->setMaximumWidth(60);
        vBox->setMaximumWidth(60);
      }

      //vBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
      hBoxInputLayout->addWidget(vBox);
    }
  }

  auto channelWidget =
      new MixerChannelWidget(device, audioPortID, 0, mixerOutputNumber, out, this);
  hBoxOutputLayout->addWidget(channelWidget);
  refreshList.push_back(channelWidget);
  savedNumberOfOutputs = mixerInterface->channelIDsForOutput(audioPortID, mixerOutputNumber).size();

  auto inputWidget = new QGroupBox();
  auto outputWidget = new QGroupBox();

  inputWidget->setLayout(hBoxInputLayout);
  inputWidget->setContentsMargins(0,0,0,0);
  int minimumWidth = 0;
  for (int i = 1; i <= mixerInterface->numberInputs(audioPortID); i++) {
    if (!mixerInputInterface->stereoLink(audioPortID, mixerOutputNumber, i)) {
      minimumWidth += 80;
    }
    else {
      minimumWidth += 90;
      i++;
    }
  }
  inputWidget->setMinimumWidth(minimumWidth);
  inputWidget->setMaximumWidth(minimumWidth);
  this->setMinimumWidth(minimumWidth + 120);
  this->setMaximumWidth(minimumWidth + 120);
  outputWidget->setMinimumWidth(120);
  outputWidget->setMaximumWidth(120);
  outputWidget->setLayout(hBoxOutputLayout);
  outputWidget->setContentsMargins(0,0,0,0);

  inputWidget->setStyleSheet("QGroupBox { border: none; margin: 0px; } QGroupBox::title { subcontrol-origin: margin; margin: 0px;}");
  outputWidget->setStyleSheet("QGroupBox { border: none; margin: 0px; } QGroupBox::title { subcontrol-origin: margin; margin: 0px;}");

  QSizePolicy expIn = QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  QSizePolicy expOut = QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  inputWidget->setSizePolicy(expIn);
  outputWidget->setSizePolicy(expOut);

  auto vBoxInputLayout = new QVBoxLayout();
  vBoxInputLayout->setContentsMargins(0, 0, 0, 0);
  vBoxInputLayout->setSpacing(0);
  vBoxInputLayout->setAlignment(Qt::AlignTop);

  auto vBoxOutputLayout = new QVBoxLayout();
  vBoxOutputLayout->setContentsMargins(0, 0, 0, 0);
  vBoxOutputLayout->setSpacing(0);
  vBoxOutputLayout->setAlignment(Qt::AlignTop);

  auto inputHeaderLayout = new QVBoxLayout();
  inputHeaderLayout->setContentsMargins(0, 0, 0, 0);
  inputHeaderLayout->setSpacing(0);

  auto outputHeaderLayout = new QVBoxLayout();
  outputHeaderLayout->setContentsMargins(0, 0, 0, 0);
  outputHeaderLayout->setSpacing(0);

  auto inputLabel = new QLabel("SOURCES");
  inputLabel->setStyleSheet("QLabel { color: #65ceff; margin-left: 10px; margin-top: 10px;}");
  auto outputLabel = new QLabel("DESTINATION");
  outputLabel->setStyleSheet("QLabel { color: #65ceff; margin-left: 10px; margin-top: 10px;}");

  inputHeaderLayout->addWidget(inputLabel);

  outputHeaderLayout->addWidget(outputLabel);

  auto inputHeader = new QWidget();
  auto outputHeader = new QWidget();

  inputHeader->setLayout(inputHeaderLayout);
  inputHeader->setContentsMargins(0,0,0,0);

  outputHeader->setLayout(outputHeaderLayout);
  outputHeader->setContentsMargins(0,0,0,0);

  vBoxInputLayout->addWidget(inputHeader);
  vBoxInputLayout->addWidget(inputWidget);

  vBoxOutputLayout->addWidget(outputHeader);
  vBoxOutputLayout->addWidget(outputWidget);

  auto input = new QWidget();
  auto output = new QWidget();

  input->setLayout(vBoxInputLayout);
  input->setContentsMargins(0,0,0,0);

  output->setLayout(vBoxOutputLayout);
  output->setContentsMargins(0,0,0,0);

  QSizePolicy sp = QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
  input->setSizePolicy(sp);
  output->setSizePolicy(sp);

  topLayout->addWidget(input);
  topLayout->addWidget(output);
}

void MixerPortWidget::clear()
{
  remove(layout());
  refreshList.clear();
  savedInputs.clear();
  savedNumberOfOutputs = 0;
}

void MixerPortWidget::remove(QLayout* layout)
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
            child->widget()->deleteLater();
        }

        delete child;
    }
}

void MixerPortWidget::refreshWidget() {
  if (refreshable) {
    if (stereoLinkHappened){
      remove(layout());
      refreshList.clear();
      savedInputs.clear();
      channelWidgets.clear();
      savedNumberOfOutputs = 0;
      stereoLinkHappened = false;
      buildChannelWidgets();
      this->setEnabled(true);
      for (MixerChannelWidget* m : channelWidgets) {
        m->adjustSize();
      }
      parent->doAdjustSize();
    }
    else {
      for (auto *refresh : refreshList) {
        if (refresh)
          refresh->refreshWidget();
      }
    }
  }
}

void MixerPortWidget::refreshMeters()
{
  for (auto *refresh : refreshList) {
    if (refresh)
      refresh->refreshMeters();
  }
}
