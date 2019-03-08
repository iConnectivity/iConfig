/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "AudioPatchbayForm.h"
#include "ui_AudioPatchbayForm.h"

#include "ACK.h"
#include "AudioInfo.h"
#include "AudioPortCfgInfo.h"
#include "AudioPortInfo.h"
#include "AudioCfgInfo.h"
#include "AudioPortCfgInfo.h"
#include "AudioPortPatchbay.h"
#include "MyAlgorithms.h"
#include "TreeUtils.h"

#include <QLabel>
#include <QMessageBox>
#include <QStandardItemModel>

#ifndef Q_MOC_RUN
#include <boost/bind.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#endif

using namespace boost;
using namespace boost::adaptors;
using namespace TreeUtils;
using namespace MyAlgorithms;
using namespace GeneSysLib;

const QString AudioPatchbayForm::DefaultWindowTitle = "Audio Patchbay";
const float AudioPatchbayForm::kBatchTime = 1500;

QList<CmdEnum> AudioPatchbayForm::Query() {
  QList<CmdEnum> query;
  query << Command::RetAudioInfo << Command::RetAudioCfgInfo
        << Command::RetAudioPortInfo << Command::RetAudioPortCfgInfo
        << Command::RetAudioPortPatchbay;
  return query;
}

AudioPatchbayForm::AudioPatchbayForm(DeviceInfoPtr _device, QWidget *_parent)
    : RefreshObject(_parent),
      device(_device),
      ui(new Ui::AudioPatchbayForm),
      outputSignalMapper(new QSignalMapper) {

  // setup GUI
  ui->setupUi(this);

  // set window title
  this->setWindowTitle(DefaultWindowTitle);

  // check that the comm and device object exist
  Q_ASSERT(device);

  // add ack handler callback
  auto ackHandler = bind(&AudioPatchbayForm::ackCallback, this, _1, _2, _3, _4);
  this->registeredHandlerIDs[Command::ACK] =
      device->registerHandler(Command::ACK, ackHandler);

  // connect mapper
  connect(outputSignalMapper, SIGNAL(mapped(int)), this,
          SLOT(outputChannelChanged(int)), Qt::QueuedConnection);

  // create the tree
  createTree();
}

AudioPatchbayForm::~AudioPatchbayForm() {
  for (const auto &handler : registeredHandlerIDs) {
    device->unRegisterHandler(handler.first, handler.second);
  }
}

void AudioPatchbayForm::outputChannelChanged(int comboBoxID) {
  Q_ASSERT(contains(idComboBoxMap, comboBoxID));
  const auto &channelComboBox = idComboBoxMap[comboBoxID];

  Q_ASSERT(
      contains(choiceToPortChannelPairMap, channelComboBox->currentIndex()));
  const auto &portChannelPair =
      choiceToPortChannelPairMap[channelComboBox->currentIndex()];

  Q_ASSERT(contains(comboBoxToInPortIDChannelPairMap, comboBoxID));
  const auto &inPortChannelPair = comboBoxToInPortIDChannelPairMap[comboBoxID];

  const auto &portID = inPortChannelPair.first;
  auto &portPatchbay = device->get<AudioPortPatchbay>(portID);

  try {
    auto &block = portPatchbay.findInputBlock(inPortChannelPair.second);
    block.portIDOfOutput(portChannelPair.first);
    block.outputChannelNumber(portChannelPair.second);
    device->send<SetAudioPortPatchbayCommand>(portPatchbay);
  }
  catch (...) {

  }
}

QStringList AudioPatchbayForm::generateOutputOptions(
    AudioCfgInfo &audioCfgInfo) {
  QStringList outputOptions;

  outputOptions << tr("Nothing");
  int choice = 0;
  portToChannelToChoiceMap[0][0] = choice;
  choiceToPortChannelPairMap[choice] = std::make_pair((Word) 0, (int) 0);
  choice++;

  device->for_each<AudioPortInfo>([&](const AudioPortInfo & audioPortInfo) {
    const auto &portID = audioPortInfo.portID();

    auto &portCfgInfo = device->get<AudioPortCfgInfo>(portID);
    int selectedConfiguration = audioCfgInfo.currentActiveConfig() - 1;

    Q_ASSERT(selectedConfiguration < (int) portCfgInfo.numConfigBlocks());

    const auto &configBlock = portCfgInfo.block_at(selectedConfiguration);

    for (int i = configBlock.minOutputChannels();
         i <= portCfgInfo.numOutputChannels(); ++i) {
      outputOptions << QString("%1 Ch. %2").arg(QString::fromStdString(
                           audioPortInfo.portName())).arg(QString::number(i));
      portToChannelToChoiceMap[portID][i] = choice;
      choiceToPortChannelPairMap[choice] = std::make_pair(portID, i);
      ++choice;
    }
  });
  return outputOptions;
}

void AudioPatchbayForm::createTree() {
  ui->treeWidget->clear();

  auto &audioCfgInfo = device->get<AudioCfgInfo>();

  int comboBoxID = 0;
  // generate the combo box choices
  const auto &outputOptions = generateOutputOptions(audioCfgInfo);

  device->for_each<AudioPortInfo>([&](AudioPortInfo & audioPortInfo) {
    createAudioPortBranch(audioCfgInfo, comboBoxID, outputOptions,
                          audioPortInfo);
  });

  ui->treeWidget->expandAll();
}

void AudioPatchbayForm::createAudioPortBranch(AudioCfgInfo &audioCfgInfo,
                                              int &comboBoxID,
                                              const QStringList &outputOptions,
                                              AudioPortInfo &audioPortInfo) {
  auto *const portInfoTreeItem = addTreeWidgetItem(
      ui->treeWidget, QString::fromStdString(audioPortInfo.portName()));

  auto &portCfgInfo = device->get<AudioPortCfgInfo>(audioPortInfo.portID());
  const auto &selectedConfiguration = audioCfgInfo.currentActiveConfig() - 1;
  Q_ASSERT(selectedConfiguration < (int) portCfgInfo.numConfigBlocks());
  const auto &configBlock = portCfgInfo.block_at(selectedConfiguration);

  const auto &portID = portCfgInfo.portID();
  const auto &portPatchbay = device->get<AudioPortPatchbay>(portID);

  for (auto i = configBlock.minInputChannels();
       i <= portCfgInfo.numInputChannels(); ++i) {
    addPortComboBox(portInfoTreeItem, audioPortInfo, portPatchbay,
                    outputOptions, i, comboBoxID);
  }
}

void AudioPatchbayForm::addPortComboBox(QTreeWidgetItem *parent,
                                        AudioPortInfo &audioPortInfo,
                                        const AudioPortPatchbay &portPatchbay,
                                        const QStringList &outputOptions,
                                        int inChannel, int &comboBoxID) {
  int selectedIndex = 0;
  try {
    auto &block = portPatchbay.findInputBlock(inChannel);
    selectedIndex = portToChannelToChoiceMap[block.portIDOfOutput()][
        block.outputChannelNumber()];
  }
  catch (...) {

  }

  auto title = QString("Channel %1").arg(QString::number(inChannel));
  auto *const channelComboBox = addComboBoxItem(ui->treeWidget, parent, title,
                                                outputOptions, selectedIndex);
  channelComboBox->installEventFilter(this);

  outputSignalMapper->setMapping(channelComboBox, comboBoxID);
  connect(channelComboBox, SIGNAL(currentIndexChanged(int)), outputSignalMapper,
          SLOT(map()));
  idComboBoxMap[comboBoxID] = channelComboBox;
  comboBoxToInPortIDChannelPairMap[comboBoxID] =
      std::make_pair(audioPortInfo.portID(), inChannel);
  ++comboBoxID;
}

// update the status bar on successful writes to the patchbay
void AudioPatchbayForm::ackCallback(CmdEnum, DeviceID, Word,
                                    commandData_t _commandData) {
  const auto &ackData = _commandData.get<ACK>();

  if ((ackData.commandID() == Command::SetAudioPortPatchbay) &&
      (ackData.errorCode() == 0)) {
    emit updateMessage(tr("Patchbay successfully set"), kMessageTimeout);
  }
}

// recreate the three on refresh (could be done more elegantly)
void AudioPatchbayForm::refreshWidget() { createTree(); }

// prevent the wheel form changing the combo boxes
bool AudioPatchbayForm::eventFilter(QObject *, QEvent *event) {
  return (event->type() == QEvent::Wheel);
}
