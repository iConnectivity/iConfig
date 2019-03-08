/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "AudioInfoForm.h"
#include "ui_AudioInfoForm.h"

#include <stdafx.h>
#include "ACK.h"
#include "CenteredContainer.h"
#include "CommandList.h"
#include "Communicator.h"
#include "DeviceInfo.h"
#include "IRefreshData.h"
#include "MyAlgorithms.h"
#include "MyLabel.h"
#include "MyLineEdit.h"
#include "Reset.h"
#include "SaveRestore.h"
#include "TreeUtils.h"

#ifndef Q_MOC_RUN
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#endif

#include <QWidget>
#include <QTableWidget>
#include <QMessageBox>

using namespace CenteredContainer;
using namespace GeneSysLib;
using namespace MyAlgorithms;
using namespace TreeUtils;
#ifndef Q_MOC_RUN
using namespace boost::adaptors;
using namespace boost;
#endif
using std::find_if;

namespace AudioUSBDeviceColumns {
typedef enum ENUM {
  Jack,
  Name,
  MacPC,
  iOS,
  InChannels,
  OutChannels,
  HostType,
  HostName
} ENUM;
}  // namespace AudioUSBDeviceColumns;

namespace AudioUSBHostColumns {
typedef enum ENUM {
  Device,
  Name,
  InChannels,
  OutChannels,
  Connected,
  Reserved,
  MaxInChannels,
  MaxOutChannels,
  Vendor,
  Product
} ENUM;
}  // namespace AudioUSBDeviceColumns;

namespace EthernetColumns {
typedef enum ENUM {
  Device,
  Name,
  InChannels,
  OutChannels
} ENUM;
}  // namespace EthernetColumns

namespace AnalogueColumns {
typedef enum ENUM {
  Port,
  Name,
  InChannels,
  OutChannels
} ENUM;
}  // namespace AnalogueColumns

const QString AudioInfoForm::DefaultWindowTitle = "Audio Info";

AudioInfoForm::AudioInfoForm(DeviceInfoPtr _device, QWidget *_parent)
    : RefreshObject(_parent), device(_device), ui(new Ui::AudioInfoForm) {
  ui->setupUi(this);

  changesHaveBeenMade = false;
  refreshing = false;

  this->setWindowTitle(DefaultWindowTitle);
  treeBuilt = false;

  Q_ASSERT(device);

  registeredHandlerIDs[Command::ACK] = device->registerHandler(
      Command::ACK, bind(&AudioInfoForm::ackCallback, this, _1, _2, _3, _4));

  connect(this, SIGNAL(v1InfoAvailable()), this, SLOT(buildV1Info()),
          Qt::QueuedConnection);
  connect(this, SIGNAL(v2InfoAvailable()), this, SLOT(buildV2Info()),
          Qt::QueuedConnection);
  connect(device.get(), SIGNAL(queryCompleted(Screen, CommandQList)), this,
          SLOT(queryCompleted(Screen, CommandQList)), Qt::QueuedConnection);

  auto &commandList = device->get<CommandList>();

  CommandQList query;

  const auto &addContainedQuery = [&](CmdEnum cmd) {
    if (commandList.contains(cmd)) {
      query << cmd;
    }
  }
  ;

  // V1
  addContainedQuery(Command::GetAudioInfo);
  addContainedQuery(Command::GetAudioCfgInfo);
  addContainedQuery(Command::GetAudioPortInfo);
  addContainedQuery(Command::GetAudioPortCfgInfo);
  addContainedQuery(Command::GetAudioClockInfo);

  // V2

  addContainedQuery(Command::GetAudioGlobalParm);
  addContainedQuery(Command::GetAudioPortParm);
  addContainedQuery(Command::GetAudioDeviceParm);
  addContainedQuery(Command::GetAudioClockParm);

  if (commandList.contains(Command::GetMixerParm)) {
    addContainedQuery(Command::GetMixerParm);
    addContainedQuery(Command::GetMixerPortParm);
    mixerInterface = new MixerInterface(device);
  }
  else {
    mixerInterface = 0;
  }
  device->startQuery(AudioInformationFormScreen, query);
}

AudioInfoForm::~AudioInfoForm() {
  for (const auto &handler : registeredHandlerIDs) {
    device->unRegisterHandler(handler.first, handler.second);
  }
}


void AudioInfoForm::buildV1Info() {
  treeBuilt = true;

  auto &audioInfo = device->get<AudioInfo>();

  if (audioInfo.versionNumber() == 0x01) {
    QTreeWidgetItem *audioInfoTreeItem;
    audioInfoTreeItem = addAudioInfoSectionV1(audioInfo);

    // audio frame buffer
    addAudioFrameComboBoxV1(audioInfoTreeItem);

    // audio sync factor
    addSyncFactorComboBoxV1(audioInfoTreeItem);

    // audio configuration
    addAudioConfigSectionV1(audioInfoTreeItem);

    // clock source
    addClockSourceV1(audioInfoTreeItem);

    // create the trees
    createUSBDeviceTableV1(audioInfo);
  }
  ui->treeWidget->expandAll();
  ui->treeWidget->resizeColumnToContents(0);

  saveOldValues();
  ui->frame->hide();
}

void AudioInfoForm::buildV2Info() {
  treeBuilt = true;

  auto &audioGlobalParm = device->get<AudioGlobalParm>();

  if (audioGlobalParm.versionNumber() == 0x01) {
    QTreeWidgetItem *audioInfoTreeItem;
    audioInfoTreeItem = addAudioInfoSectionV2(audioGlobalParm);

    // add frame buffer
    addAudioFrameComboBoxV2(audioInfoTreeItem);

    // audio sync factor
    addSyncFactorComboBoxV2(audioInfoTreeItem);

    // audio configuration
    addAudioConfigSectionV2(audioInfoTreeItem);

    // clock source
    addClockSourceV2(audioInfoTreeItem);

    // create the trees
    createPortTablesV2();

    if (device->containsType<MixerParm>()) {
      auto &mixerParm = device->get<MixerParm>(audioGlobalParm.currentActiveConfig());
      QTreeWidgetItem *mixerInfoTreeItem;
      mixerInfoTreeItem = addMixerInfoSectionV2(mixerParm);

      addMixerConfigSectionV2(mixerInfoTreeItem);
      addMixerInputsAndOutputsConfigSectionV2(mixerInfoTreeItem);
    }
  }
  ui->treeWidget->expandAll();
  ui->treeWidget->resizeColumnToContents(0);

  saveOldValues();
  ui->frame->hide();
}

////////////////////////////////////////////////////////////////////////////////
/// USB Device Tables
////////////////////////////////////////////////////////////////////////////////
void AudioInfoForm::createUSBDeviceTableV1(const AudioInfo &audioInfo) {
  if (device->containsType<AudioPortInfo>()) {
    ui->verticalTablesLayout->addWidget(new QLabel(tr("USB Device Jacks")));

    QStringList labels;
    labels << tr("Jack") << tr("Name") << tr("Mac/PC") << tr("iOS")
           << tr("Out\nChannels") << tr("In\nChannels"); // TODO: is this right??

    auto *const table =
        createInfoTableWidget(labels, device->audioPortInfoCount());
    ui->verticalTablesLayout->addWidget(table);

    auto *const horzHdr = table->horizontalHeader();
    horzHdr->setResizeMode(QHeaderView::ResizeToContents);
    horzHdr->setDefaultSectionSize(kDefaultColumnSize);
    horzHdr->setResizeMode(AudioUSBDeviceColumns::Jack, QHeaderView::Fixed);
    horzHdr->setResizeMode(AudioUSBDeviceColumns::Name, QHeaderView::Stretch);

    const auto &b = startUSBAudioPortID();
    const auto &e = endUSBAudioPortID(audioInfo);
    int row = 0;
    for (Word portID = b; portID < e; ++portID, ++row) {
      addUSBDeviceRowV1(table, row, portID);
    }
  }
}


void AudioInfoForm::createPortTablesV2() {
  if (device->containsType<AudioPortParm>()) {

    const auto &containsAudioParmOfType = [ = ](PortTypeEnum portType)->bool {
      return device->any_of<AudioPortParm>(
         boost::bind(&AudioPortParm::isOfType, _1, portType));
    }
    ;

    if (containsAudioParmOfType(PortType::USBDevice)) {
      createUSBDeviceTableV2();
    }

    if (containsAudioParmOfType(PortType::USBHost)) {
      createUSBHostTablesV2();
    }

    if (containsAudioParmOfType(PortType::Ethernet)) {
      createEthernetTablesV2();
    }

    if (containsAudioParmOfType(PortType::Analogue)) {
      createAnalogueTableV2();
    }
  }
}

void AudioInfoForm::createUSBDeviceTableV2() {
  ui->verticalTablesLayout->addWidget(new QLabel(tr("USB Device Jacks")));

  QStringList labels;
  labels << tr("Jack") << tr("Name") << tr("Mac/PC") << tr("iOS")
         << tr("Out\nChannels") << tr("In\nChannels") << tr("Host Type")
         << tr("Host Name");

    // create a table for all USB Device
  const auto &usbDeviceCount =
      device->count<AudioPortParm>([](const AudioPortParm & audioPortParm) {
    return audioPortParm.isOfType(PortType::USBDevice);
  });
  auto *const table = createInfoTableWidget(labels, usbDeviceCount);
  ui->verticalTablesLayout->addWidget(table);

  auto *const horzHdr = table->horizontalHeader();
  horzHdr->setResizeMode(QHeaderView::ResizeToContents);
  horzHdr->setDefaultSectionSize(kDefaultColumnSize);
  horzHdr->setResizeMode(AudioUSBDeviceColumns::Jack, QHeaderView::Fixed);
  horzHdr->setResizeMode(AudioUSBDeviceColumns::Name, QHeaderView::Stretch);
  horzHdr->setResizeMode(AudioUSBDeviceColumns::HostName, QHeaderView::Stretch);

  int row = 0;
  device->for_each<AudioPortParm>([&](const AudioPortParm & audioPortParm) {
    if (audioPortParm.portType() == PortType::USBDevice) {
      addUSBDeviceRowV2(table, row, audioPortParm);
      ++row;
    }
  });
}

void AudioInfoForm::createUSBHostTablesV2() {
  Byte lastJack = 0xFF;
  device->for_each<AudioPortParm>([&](const AudioPortParm & audioPortParm) {
    if (audioPortParm.portType() == PortType::USBHost) {
      const auto &usbHost = audioPortParm.usbHost();
      if (usbHost.jack() != lastJack) {
        lastJack = usbHost.jack();
        createUSBHostTableV2(usbHost.jack());
      }
    }
  });
}

void AudioInfoForm::createUSBHostTableV2(Byte hostJack) {
  ui->verticalTablesLayout->addWidget(
      new QLabel(QString("USB Host Jack %1").arg(QString::number(hostJack))));

  QStringList labels;
  labels << tr("Device") << tr("Name") << tr("In\nChannels")
         << tr("Out\nChannels") << tr("Connected") << tr("Reserved")
         << tr("Supported\nIn Channels") << tr("Supported\nOut Channels")
         << tr("Vendor") << tr("Product");

    // create a table for all USB Device
  const auto &usbHostCount =
      device->count<AudioPortParm>([](const AudioPortParm & audioPortParm) {
    return audioPortParm.isOfType(PortType::USBHost);
  });
  auto *const table = createInfoTableWidget(labels, usbHostCount);
  ui->verticalTablesLayout->addWidget(table);

  auto *const horzHdr = table->horizontalHeader();
  horzHdr->setResizeMode(QHeaderView::ResizeToContents);
  horzHdr->setDefaultSectionSize(kDefaultColumnSize);
  horzHdr->setResizeMode(AudioUSBHostColumns::Device, QHeaderView::Fixed);
  horzHdr->setResizeMode(AudioUSBHostColumns::Name, QHeaderView::Stretch);
  horzHdr->setResizeMode(AudioUSBHostColumns::Vendor, QHeaderView::Stretch);
  horzHdr->setResizeMode(AudioUSBHostColumns::Product, QHeaderView::Stretch);

  int row = 0;
  device->for_each<AudioPortParm>([&](const AudioPortParm & audioPortParm) {
    if (audioPortParm.portType() == PortType::USBHost) {
      auto usbHost = audioPortParm.usbHost();
      if (usbHost.jack() == hostJack) {
        addUSBHostRowV2(table, row, audioPortParm);
        ++row;
      }
    }
  });
}

void AudioInfoForm::createEthernetTablesV2() {
  Byte lastJack = 0xFF;
  device->for_each<AudioPortParm>([&](const AudioPortParm & audioPortParm) {
    if (audioPortParm.portType() == PortType::Ethernet) {
      const auto &ethernet = audioPortParm.ethernet();
      if (ethernet.jack() != lastJack) {
        lastJack = ethernet.jack();
        createEthernetTableV2(ethernet.jack());
      }
    }
  });
}

void AudioInfoForm::createEthernetTableV2(Byte hostJack) {
  ui->verticalTablesLayout->addWidget(
      new QLabel(QString("Ethernet Jack %1").arg(QString::number(hostJack))));

  QStringList labels;
  labels << tr("Device") << tr("Name") << tr("In\nChannels")
         << tr("Out\nChannels");

    // create a table for all USB Device
  const auto &ethernetCount =
      device->count<AudioPortParm>([](const AudioPortParm & audioPortParm) {
    return audioPortParm.isOfType(PortType::Ethernet);
  });
  auto *const table = createInfoTableWidget(labels, ethernetCount);
  ui->verticalTablesLayout->addWidget(table);

  auto *const horzHdr = table->horizontalHeader();
  horzHdr->setResizeMode(QHeaderView::ResizeToContents);
  horzHdr->setDefaultSectionSize(kDefaultColumnSize);
  horzHdr->setResizeMode(EthernetColumns::Device, QHeaderView::Fixed);
  horzHdr->setResizeMode(EthernetColumns::Name, QHeaderView::Stretch);

  int row = 0;
  device->for_each<AudioPortParm>([&](const AudioPortParm & audioPortParm) {
    if (audioPortParm.portType() == PortType::Ethernet) {
      auto ethernet = audioPortParm.ethernet();
      if (ethernet.jack() == hostJack) {
        addEthernetRowV2(table, row, audioPortParm);
        ++row;
      }
    }
  });
}

void AudioInfoForm::createAnalogueTableV2() {
  ui->verticalTablesLayout->addWidget(new QLabel(tr("Analog Jacks")));

  QStringList labels;
  labels << tr("Port") << tr("Name") << tr("Out\nChannels")
         << tr("In\nChannels");

    // create a table for all USB Device
  const auto &analogueCount =
      device->count<AudioPortParm>([](const AudioPortParm & audioPortParm) {
    return audioPortParm.isOfType(PortType::Analogue);
  });
  auto *const table = createInfoTableWidget(labels, analogueCount);
  ui->verticalTablesLayout->addWidget(table);

  auto *const horzHdr = table->horizontalHeader();
  horzHdr->setResizeMode(QHeaderView::ResizeToContents);
  horzHdr->setDefaultSectionSize(kDefaultColumnSize);
  horzHdr->setResizeMode(AnalogueColumns::Port, QHeaderView::Fixed);
  horzHdr->setResizeMode(AnalogueColumns::Name, QHeaderView::Stretch);

  int row = 0;
  device->for_each<AudioPortParm>([&](const AudioPortParm & audioPortParm) {
    if (audioPortParm.portType() == PortType::Analogue) {
      addAnalogueRowV2(table, row, audioPortParm);
      ++row;
    }
  });
}

void AudioInfoForm::addUSBDeviceRowV1(QTableWidget *table, int row,
                                      Word portID) {
  const auto &audioCfgInfo = device->get<AudioCfgInfo>();
  const auto &selConfig = audioCfgInfo.currentActiveConfig() - 1;

  const auto &portCfgInfo = device->get<AudioPortCfgInfo>(portID);
  Q_ASSERT(selConfig < static_cast<int>(portCfgInfo.numConfigBlocks()));
  const auto &configBlock = portCfgInfo.block_at(selConfig);

  const auto &audioPortInfo = device->get<AudioPortInfo>(portID);

  // set the row height
  table->setRowHeight(row, kDefaultRowSize);

  // jack number
  addLabel(table, row, AudioUSBDeviceColumns::Jack,
           bind(&AudioInfoForm::getJackNumberV1, this, portID));

  // name
  if (getMaxPortNameV1(portID) == 0) {
    addLabel(table, row, AudioUSBDeviceColumns::Name,
             bind(&AudioInfoForm::getPortNameV1, this, portID));
  } else {
    addLineEdit(table, row, AudioUSBDeviceColumns::Name,
                getMaxPortNameV1(portID),
                bind(&AudioInfoForm::getPortNameV1, this, portID),
                bind(&AudioInfoForm::setPortNameV1, this, portID, _1));
  }

  // mac/pc checkbox
  if (audioPortInfo.supportsPCAudio()) {
    addCheckBox(table, row, AudioUSBDeviceColumns::MacPC,
                bind(&AudioInfoForm::getMacPCEnabledV1, this, portID),
                bind(&AudioInfoForm::setMacPCEnabledV1, this, portID, _1));
  } else {
    addLabel(table, row, AudioUSBDeviceColumns::MacPC, tr("N/A"));
  }

  // ios checkbox
  if (audioPortInfo.supportsIOSAudio()) {
    addCheckBox(table, row, AudioUSBDeviceColumns::iOS,
                bind(&AudioInfoForm::getIOSEnabledV1, this, portID),
                bind(&AudioInfoForm::setIOSEnabledV1, this, portID, _1));
  } else {
    addLabel(table, row, AudioUSBDeviceColumns::iOS, tr("N/A"));
  }

  // add input combo box
  if (configBlock.isInputSelectable()) {
    addComboBox(table, row, AudioUSBDeviceColumns::InChannels,
                bind(&AudioInfoForm::getInputOptionsV1, this, portID),
                bind(&AudioInfoForm::getSelectedInputV1, this, portID, _1),
                bind(&AudioInfoForm::setSelectedInputV1, this, portID, _1, _2));
  } else {
    addLabel(table, row, AudioUSBDeviceColumns::InChannels,
             bind(&AudioInfoForm::getNumInputChannelsLabelV1, this, portID));
  }

  // add output combo box
  if (configBlock.isOutputSelectable()) {
    addComboBox(
        table, row, AudioUSBDeviceColumns::OutChannels,
        bind(&AudioInfoForm::getOutputOptionsV1, this, portID),
        bind(&AudioInfoForm::getSelectedOutputV1, this, portID, _1),
        bind(&AudioInfoForm::setSelectedOutputV1, this, portID, _1, _2));
  } else {
    addLabel(table, row, AudioUSBDeviceColumns::OutChannels,
             bind(&AudioInfoForm::getNumOutputChannelsLabelV1, this, portID));
  }
}

void AudioInfoForm::addUSBDeviceRowV2(QTableWidget *table, int row,
                                      const AudioPortParm &audioPortParm) {
  Q_ASSERT(audioPortParm.isOfType(PortType::USBDevice));
  const auto &audioPortID = audioPortParm.audioPortID();
  const auto &usbDevice = audioPortParm.usbDevice();

  // set the row height
  table->setRowHeight(row, kDefaultRowSize);

  // jack number
  addLabel(table, row, AudioUSBDeviceColumns::Jack,
           bind(&AudioInfoForm::getJackNumberV2, this, audioPortID));

  // name
  if (getMaxPortNameV2(audioPortID) == 0) {
    addLabel(table, row, AudioUSBDeviceColumns::Name,
             bind(&AudioInfoForm::getPortNameV2, this, audioPortID));
  } else {
    addLineEdit(table, row, AudioUSBDeviceColumns::Name,
                getMaxPortNameV2(audioPortID),
                bind(&AudioInfoForm::getPortNameV2, this, audioPortID),
                bind(&AudioInfoForm::setPortNameV2, this, audioPortID, _1));
  }

  // mac/pc checkbox
  if (usbDevice.supportsPCAudio()) {
    addCheckBox(table, row, AudioUSBDeviceColumns::MacPC,
                bind(&AudioInfoForm::getMacPCEnabledV2, this, audioPortID),
                bind(&AudioInfoForm::setMacPCEnabledV2, this, audioPortID, _1));
  } else {
    addLabel(table, row, AudioUSBDeviceColumns::MacPC, tr("N/A"));
  }

  // ios checkbox
  if (usbDevice.supportsIOSAudio()) {
    addCheckBox(table, row, AudioUSBDeviceColumns::iOS,
                bind(&AudioInfoForm::getIOSEnabledV2, this, audioPortID),
                bind(&AudioInfoForm::setIOSEnabledV2, this, audioPortID, _1));
  } else {
    addLabel(table, row, AudioUSBDeviceColumns::iOS, tr("N/A"));
  }

  const auto &audioGlobalParm = device->get<AudioGlobalParm>();
  const auto &activeConfigID = audioGlobalParm.currentActiveConfig();
  const auto &configBlock = audioPortParm.block_at(activeConfigID);

  // add input combo box
  if (configBlock.isInputSelectable()) {
    addComboBox(
        table, row, AudioUSBDeviceColumns::InChannels,
        bind(&AudioInfoForm::getInputOptionsV2, this, audioPortID),
        bind(&AudioInfoForm::getSelectedInputV2, this, audioPortID, _1),
        bind(&AudioInfoForm::setSelectedInputV2, this, audioPortID, _1, _2));
  } else {
    addLabel(
        table, row, AudioUSBDeviceColumns::InChannels,
        bind(&AudioInfoForm::getNumInputChannelsLabelV2, this, audioPortID));
  }

  // add output combo box
  if (configBlock.isOutputSelectable()) {
    addComboBox(
        table, row, AudioUSBDeviceColumns::OutChannels,
        bind(&AudioInfoForm::getOutputOptionsV2, this, audioPortID),
        bind(&AudioInfoForm::getSelectedOutputV2, this, audioPortID, _1),
        bind(&AudioInfoForm::setSelectedOutputV2, this, audioPortID, _1, _2));
  } else {
    addLabel(
        table, row, AudioUSBDeviceColumns::OutChannels,
        bind(&AudioInfoForm::getNumOutputChannelsLabelV2, this, audioPortID));
  }

  if (device->contains<AudioDeviceParm>(audioPortID)) {
    // add host type label
    addLabel(table, row, AudioUSBDeviceColumns::HostType,
             bind(&AudioInfoForm::getUSBDeviceHostTypeV2, this, audioPortID));

    // add host name label
    addLabel(table, row, AudioUSBDeviceColumns::HostName,
             bind(&AudioInfoForm::getUSBDeviceHostNameV2, this, audioPortID));
  }
}

void AudioInfoForm::addUSBHostRowV2(QTableWidget *table, int row,
                                    const AudioPortParm &audioPortParm) {
  Q_ASSERT(audioPortParm.isOfType(PortType::USBHost));
  const auto &audioPortID = audioPortParm.audioPortID();

  // set the row height
  table->setRowHeight(row, kDefaultRowSize);

  // device number
  addLabel(table, row, AudioUSBHostColumns::Device,
           bind(&AudioInfoForm::getUSBHostDeviceNumberV2, this, audioPortID));

  // name
  if (getMaxPortNameV2(audioPortID) == 0) {
    addLabel(table, row, AudioUSBHostColumns::Name,
             bind(&AudioInfoForm::getPortNameV2, this, audioPortID));
  } else {
    addLineEdit(table, row, AudioUSBHostColumns::Name,
                getMaxPortNameV2(audioPortID),
                bind(&AudioInfoForm::getPortNameV2, this, audioPortID),
                bind(&AudioInfoForm::setPortNameV2, this, audioPortID, _1));
  }
  const auto &audioGlobalParm = device->get<AudioGlobalParm>();
  const auto &activeConfigID = audioGlobalParm.currentActiveConfig();
  const auto &configBlock = audioPortParm.block_at(activeConfigID);

  // add input combo box
  if (configBlock.isInputSelectable()) {
    addComboBox(
        table, row, AudioUSBHostColumns::InChannels,
        bind(&AudioInfoForm::getInputOptionsV2, this, audioPortID),
        bind(&AudioInfoForm::getSelectedInputV2, this, audioPortID, _1),
        bind(&AudioInfoForm::setSelectedInputV2, this, audioPortID, _1, _2));
  } else {
    addLabel(
        table, row, AudioUSBHostColumns::InChannels,
        bind(&AudioInfoForm::getNumInputChannelsLabelV2, this, audioPortID));
  }

  // add output combo box
  if (configBlock.isOutputSelectable()) {
    addComboBox(
        table, row, AudioUSBHostColumns::OutChannels,
        bind(&AudioInfoForm::getOutputOptionsV2, this, audioPortID),
        bind(&AudioInfoForm::getSelectedOutputV2, this, audioPortID, _1),
        bind(&AudioInfoForm::setSelectedOutputV2, this, audioPortID, _1, _2));
  } else {
    addLabel(
        table, row, AudioUSBHostColumns::OutChannels,
        bind(&AudioInfoForm::getNumOutputChannelsLabelV2, this, audioPortID));
  }

  if (device->contains<AudioDeviceParm>(audioPortID)) {
    // add connected label
    addLabel(table, row, AudioUSBHostColumns::Connected,
             bind(&AudioInfoForm::getConnectedV2, this, audioPortID));

    // add reserved checkbox
    addCheckBox(table, row, AudioUSBHostColumns::Reserved,
                bind(&AudioInfoForm::getReservedV2, this, audioPortID),
                bind(&AudioInfoForm::setReservedV2, this, audioPortID, _1),
                bind(&AudioInfoForm::reservedEnabledV2, this, audioPortID));

    // add max in channels
    addLabel(table, row, AudioUSBHostColumns::MaxInChannels,
             bind(&AudioInfoForm::getMaxInChannelsV2, this, audioPortID));

    // add max out channels
    addLabel(table, row, AudioUSBHostColumns::MaxOutChannels,
             bind(&AudioInfoForm::getMaxOutChannelsV2, this, audioPortID));

    // add vendor name
    addLabel(table, row, AudioUSBHostColumns::Vendor,
             bind(&AudioInfoForm::getUSBHostVendorNameV2, this, audioPortID));

    // add product name
    addLabel(table, row, AudioUSBHostColumns::Product,
             bind(&AudioInfoForm::getUSBHostProductNameV2, this, audioPortID));
  }
}

void AudioInfoForm::addEthernetRowV2(QTableWidget *table, int row,
                                     const AudioPortParm &audioPortParm) {
  Q_ASSERT(audioPortParm.isOfType(PortType::Ethernet));
  const auto &audioPortID = audioPortParm.audioPortID();

  // set the row height
  table->setRowHeight(row, kDefaultRowSize);

  // device number
  addLabel(table, row, EthernetColumns::Device,
           bind(&AudioInfoForm::getEthernetDeviceNumberV2, this, audioPortID));

  // name
  if (getMaxPortNameV2(audioPortID) == 0) {
    addLabel(table, row, EthernetColumns::Name,
             bind(&AudioInfoForm::getPortNameV2, this, audioPortID));
  } else {
    addLineEdit(table, row, EthernetColumns::Name,
                getMaxPortNameV2(audioPortID),
                bind(&AudioInfoForm::getPortNameV2, this, audioPortID),
                bind(&AudioInfoForm::setPortNameV2, this, audioPortID, _1));
  }
  const auto &audioGlobalParm = device->get<AudioGlobalParm>();
  const auto &activeConfigID = audioGlobalParm.currentActiveConfig();
  const auto &configBlock = audioPortParm.block_at(activeConfigID);

  // add input combo box
  if (configBlock.isInputSelectable()) {
    addComboBox(
        table, row, EthernetColumns::InChannels,
        bind(&AudioInfoForm::getInputOptionsV2, this, audioPortID),
        bind(&AudioInfoForm::getSelectedInputV2, this, audioPortID, _1),
        bind(&AudioInfoForm::setSelectedInputV2, this, audioPortID, _1, _2));
  } else {
    addLabel(
        table, row, EthernetColumns::InChannels,
        bind(&AudioInfoForm::getNumInputChannelsLabelV2, this, audioPortID));
  }

  // add output combo box
  if (configBlock.isOutputSelectable()) {
    addComboBox(
        table, row, EthernetColumns::OutChannels,
        bind(&AudioInfoForm::getOutputOptionsV2, this, audioPortID),
        bind(&AudioInfoForm::getSelectedOutputV2, this, audioPortID, _1),
        bind(&AudioInfoForm::setSelectedOutputV2, this, audioPortID, _1, _2));
  } else {
    addLabel(
        table, row, EthernetColumns::OutChannels,
        bind(&AudioInfoForm::getNumOutputChannelsLabelV2, this, audioPortID));
  }
}

void AudioInfoForm::addAnalogueRowV2(QTableWidget *table, int row,
                                     const AudioPortParm &audioPortParm) {
  Q_ASSERT(audioPortParm.isOfType(PortType::Analogue));
  const auto &audioPortID = audioPortParm.audioPortID();

  // set the row height
  table->setRowHeight(row, kDefaultRowSize);

  // port number
  addLabel(table, row, AnalogueColumns::Port,
           bind(&AudioInfoForm::getAnaloguePortV2, this, audioPortID));

  // name
  if (getMaxPortNameV2(audioPortID) == 0) {
    addLabel(table, row, AnalogueColumns::Name,
             bind(&AudioInfoForm::getPortNameV2, this, audioPortID));
  } else {
    addLineEdit(table, row, AnalogueColumns::Name,
                getMaxPortNameV2(audioPortID),
                bind(&AudioInfoForm::getPortNameV2, this, audioPortID),
                bind(&AudioInfoForm::setPortNameV2, this, audioPortID, _1));
  }
  const auto &audioGlobalParm = device->get<AudioGlobalParm>();
  const auto &activeConfigID = audioGlobalParm.currentActiveConfig();
  const auto &configBlock = audioPortParm.block_at(activeConfigID);

  // add input combo box
  if (configBlock.isInputSelectable()) {
    addComboBox(
        table, row, AnalogueColumns::InChannels,
        bind(&AudioInfoForm::getInputOptionsV2, this, audioPortID),
        bind(&AudioInfoForm::getSelectedInputV2, this, audioPortID, _1),
        bind(&AudioInfoForm::setSelectedInputV2, this, audioPortID, _1, _2));
  } else {
    addLabel(
        table, row, AnalogueColumns::InChannels,
        bind(&AudioInfoForm::getNumInputChannelsLabelV2, this, audioPortID));
  }

  // add output combo box
  if (configBlock.isOutputSelectable()) {
    addComboBox(
        table, row, AnalogueColumns::OutChannels,
        bind(&AudioInfoForm::getOutputOptionsV2, this, audioPortID),
        bind(&AudioInfoForm::getSelectedOutputV2, this, audioPortID, _1),
        bind(&AudioInfoForm::setSelectedOutputV2, this, audioPortID, _1, _2));
  } else {
    addLabel(
        table, row, AnalogueColumns::OutChannels,
        bind(&AudioInfoForm::getNumOutputChannelsLabelV2, this, audioPortID));
  }
}

MyLabel *AudioInfoForm::addLabel(QTableWidget *tableWidget, int row, int col,
                                 const QString &value) {
  auto *const label = new MyLabel(value);
  label->setAlignment(Qt::AlignCenter);
  tableWidget->setCellWidget(row, col, label);
  refreshList.push_back(label);
  return label;
}

MyLabel *AudioInfoForm::addLabel(QTableWidget *tableWidget, int row, int col,
                                 const GetQStringFunctor &getFunc) {
  auto *const label = new MyLabel(getFunc);
  label->setAlignment(Qt::AlignCenter);
  tableWidget->setCellWidget(row, col, label);
  refreshList.push_back(label);
  return label;
}

MyLineEdit *AudioInfoForm::addLineEdit(QTableWidget *tableWidget, int row,
                                       int col, int maxLength,
                                       const GetQStringFunctor &getFunc,
                                       const SetQStringFunctor &setFunc) {
  auto *const lineEdit = new MyLineEdit(getFunc, setFunc);
  lineEdit->setMaxLength(maxLength);
  tableWidget->setCellWidget(row, col, lineEdit);
  refreshList.push_back(lineEdit);
  return lineEdit;
}

void AudioInfoForm::addCheckBox(QTableWidget *tableWidget, int row, int col,
                                const GetBoolFunctor &getFunc,
                                const SetBoolFunctor &setFunc) {
  Q_ASSERT(tableWidget);

  auto *const enabledCheckBox = new MyCheckBox(getFunc, setFunc);
  auto *const centeredWidget = centeredContainer(enabledCheckBox);

  tableWidget->setCellWidget(row, col, centeredWidget);
  refreshList.push_back(enabledCheckBox);
}

void AudioInfoForm::addCheckBox(QTableWidget *tableWidget, int row, int col,
                                const GetBoolFunctor &getFunc,
                                const SetBoolFunctor &setFunc,
                                const GetBoolFunctor &enableFunc) {
  Q_ASSERT(tableWidget);

  auto *const enabledCheckBox = new MyCheckBox(getFunc, setFunc, enableFunc);
  auto *const centeredWidget = centeredContainer(enabledCheckBox);

  tableWidget->setCellWidget(row, col, centeredWidget);
  refreshList.push_back(enabledCheckBox);
}

void AudioInfoForm::addComboBox(QTableWidget *tableWidget, int row, int col,
                                const GetComboBoxOptions &options,
                                const GetComboBoxSelectionFunctor &getFunc,
                                const SetComboBoxSelectionFunctor &setFunc) {
  Q_ASSERT(tableWidget);

  auto *const channelComboBox = new MyComboBox(options, getFunc, setFunc);
  tableWidget->setCellWidget(row, col, channelComboBox);
  refreshList.push_back(channelComboBox);
}

bool AudioInfoForm::promptForClose()
{
  if (changesHaveBeenMade) {
    if(QMessageBox::question(this, tr("Save your changes?"), tr("Would you like to save your changes?"), QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton) == QMessageBox::Yes) {
      on_pushButton_clicked();
      return false;
    } else {
      restoreOldValues();
      return true;
    }
  }
  else {
    return true;
  }
}

void AudioInfoForm::queryCompleted(Screen sc, CommandQList) {
  if (sc == AudioInformationFormScreen)
  {
    if (!treeBuilt) {
      if (device->contains<AudioInfo>()) {
        emit v1InfoAvailable();
      } else if (device->contains<AudioGlobalParm>()) {
        emit v2InfoAvailable();
      }
    }
  }
}

void AudioInfoForm::ackCallback(CmdEnum, DeviceID, Word,
                                commandData_t _commandData) {
  const auto &ackData = _commandData.get<ACK>();

  switch (ackData.commandID()) {
    case Command::SetAudioCfgInfo:
    case Command::SetAudioClockInfo:
    case Command::SetAudioGlobalParm:
      // update Audio
      emit updateMessage(
          (ackData.errorCode() == 0x00)
              ? tr("Audio information successfully written.")
              : tr("Error writing information. Please try again."),
          kMessageTimeout);
      emit requestRefresh();
      break;

    case Command::SetAudioPortInfo:
    case Command::SetAudioPortCfgInfo:
    case Command::SetAudioPortParm:
      // update port info
      emit updateMessage((ackData.errorCode() == 0x00)
                             ? tr("Audio Port Information successfully written")
                             : tr("Error writing audio port information."),
                         kMessageTimeout);
      emit requestRefresh();
      break;

    case Command::SaveRestore: {
      // update save restore
      if (ackData.errorCode() == ErrorCode::NoError) {

        // send reset
        if (!sysexMessages.empty()) {
          auto sysexBytes = sysexMessages.front();
          sysexMessages.pop();

          device->send(sysexBytes);
        }
      } else {
        emit updateMessage(tr("Error committing changes."), kMessageTimeout);
      }
      break;
    }

    default:
      break;
  }
}

void AudioInfoForm::refreshWidget() {
  refreshing = true;
  for (auto *refresh : refreshList) {
    refresh->refreshData();
  }
  refreshing = false;
}

void AudioInfoForm::on_pushButton_clicked() {
  if (QMessageBox::warning(
          this, tr("Commit Changes"),
          tr("Settings will be committed to memory and the system will be reset. "
             " Do you wish to continue?"),
          QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok) {

    // Add the Reset to the pending messages
    auto bytes = sysex(ResetCommand(device->getDeviceID(), device->getTransID(),
                                    BootMode::AppMode));
    sysexMessages.push(bytes);

    // Send the commit to flash message
    device->send<SaveRestoreCommand>(SaveRestoreID::SaveToFlash);
  }
}

void AudioInfoForm::on_pushButton_2_clicked()
{
  restoreOldValues();
  device->rereadStored();

  changesHaveBeenMade = false;
  ui->frame->hide();
  QTimer::singleShot(200, this, SLOT(refreshWidget()));
  QTimer::singleShot(200, this, SLOT(saveOldValues()));
}

QTreeWidgetItem *AudioInfoForm::addAudioInfoSectionV1(AudioInfo &audioInfo) {
  auto *const audioInfoTreeItem = new QTreeWidgetItem(ui->treeWidget);
  audioInfoTreeItem->setText(0, tr("Audio Information"));

  if (audioInfo.numberOfAudioPorts() > 0) {
    addTextItem(audioInfoTreeItem, tr("Number of Audio ports"),
                QString::number(audioInfo.numberOfAudioPorts()));
  }
  if (audioInfo.numberOfUSBDeviceJacks() > 0) {
    addTextItem(audioInfoTreeItem, tr("Number of Audio USB Device jacks"),
                QString::number(audioInfo.numberOfUSBDeviceJacks()));
  }
  if (audioInfo.numberOfUSBHostJacks() > 0) {
    addTextItem(audioInfoTreeItem, tr("Number of audio capable USB host jacks"),
                QString::number(audioInfo.numberOfUSBHostJacks()));
  }
  if (audioInfo.numberOfEthernetJacks() > 0) {
    addTextItem(audioInfoTreeItem, tr("Number of audio capable ethernet jacks"),
                QString::number(audioInfo.numberOfEthernetJacks()));
  }
  if (audioInfo.numberOfPortsPerUSBHostJack() > 0) {
    addTextItem(audioInfoTreeItem, tr("Audio ports per USB host jack"),
                QString::number(audioInfo.numberOfPortsPerUSBHostJack()));
  }
  if (audioInfo.numberOfPortsPerEthernetJack() > 0) {
    addTextItem(audioInfoTreeItem, tr("Audio ports per ethernet jack"),
                QString::number(audioInfo.numberOfPortsPerEthernetJack()));
  }

  return audioInfoTreeItem;
}

QTreeWidgetItem *AudioInfoForm::addAudioInfoSectionV2(
    AudioGlobalParm &audioGlobalParm) {
  auto *const audioInfoTreeItem = new QTreeWidgetItem(ui->treeWidget);
  audioInfoTreeItem->setText(0, tr("Audio Information"));

  if (audioGlobalParm.numAudioPorts() > 0) {
    addTextItem(audioInfoTreeItem, tr("Number of audio ports"),
                QString::number(audioGlobalParm.numAudioPorts()));
  }

  return audioInfoTreeItem;
}

QTreeWidgetItem *AudioInfoForm::addMixerInfoSectionV2(
    GeneSysLib::MixerParm& mixerParm) {
  auto *const mixerInfoTreeItem = new QTreeWidgetItem(ui->treeWidget);
  mixerInfoTreeItem->setText(0, tr("Mixer Information"));

  return mixerInfoTreeItem;
}

void AudioInfoForm::addAudioFrameComboBoxV1(QTreeWidgetItem *treeItem) {
  auto *const cbox = addComboBoxItem(
      treeItem, tr("Number of buffered audio frames"),
      bind(&AudioInfoForm::audioFrameOptionsV1, this),
      bind(&AudioInfoForm::getAudioFrameSelectionV1, this, _1),
      bind(&AudioInfoForm::setAudioFrameSelectionV1, this, _1, _2));
  refreshList.push_back(cbox);
}

void AudioInfoForm::addAudioFrameComboBoxV2(QTreeWidgetItem *treeItem) {
  auto *const cbox = addComboBoxItem(
      treeItem, tr("Number of buffered audio frames"),
      bind(&AudioInfoForm::audioFrameOptionsV2, this),
      bind(&AudioInfoForm::getAudioFrameSelectionV2, this, _1),
      bind(&AudioInfoForm::setAudioFrameSelectionV2, this, _1, _2));
  refreshList.push_back(cbox);
}

void AudioInfoForm::addSyncFactorComboBoxV1(QTreeWidgetItem *treeItem) {
  auto *const cbox = addComboBoxItem(
      treeItem, tr("Sync factor value"),
      bind(&AudioInfoForm::audioSyncOptionsV1, this),
      bind(&AudioInfoForm::getAudioSyncSelectionV1, this, _1),
      bind(&AudioInfoForm::setAudioSyncSelectionV1, this, _1, _2));
  refreshList.push_back(cbox);
}

void AudioInfoForm::addSyncFactorComboBoxV2(QTreeWidgetItem *treeItem) {
  auto *const cbox = addComboBoxItem(
      treeItem, tr("Sync factor value"),
      bind(&AudioInfoForm::audioSyncOptionsV2, this),
      bind(&AudioInfoForm::getAudioSyncSelectionV2, this, _1),
      bind(&AudioInfoForm::setAudioSyncSelectionV2, this, _1, _2));
  refreshList.push_back(cbox);
}

void AudioInfoForm::addAudioConfigSectionV1(QTreeWidgetItem *treeItem) {
  auto *const cbox = addComboBoxItem(
      treeItem, tr("Audio configuration"),
      bind(&AudioInfoForm::audioConfigOptionsV1, this),
      bind(&AudioInfoForm::getAudioConfigSelectionV1, this, _1),
      bind(&AudioInfoForm::setAudioConfigSelectionV1, this, _1, _2));
  refreshList.push_back(cbox);
}

void AudioInfoForm::addMixerConfigSectionV2(QTreeWidgetItem *treeItem) {
  auto *const cbox = addComboBoxItem(
      treeItem, tr("Mixer configuration"),
      bind(&AudioInfoForm::mixerConfigOptionsV2, this),
      bind(&AudioInfoForm::getMixerConfigSelectionV2, this, _1),
      bind(&AudioInfoForm::setMixerConfigSelectionV2, this, _1, _2));
  refreshList.push_back(cbox);
}

void AudioInfoForm::addMixerInputsAndOutputsConfigSectionV2(QTreeWidgetItem *treeItem) {
  const auto &audioGlobalParm = device->get<AudioGlobalParm>();

  for (int i = 1; i <= audioGlobalParm.numAudioPorts(); i++) {
    auto *const cbox2 = addComboBoxItem(
        treeItem, tr("Input channels for mixer on %1 domain").arg(getPortNameV2(i)),
        bind(&AudioInfoForm::inputsPerPortConfigOptionsV2, this, i),
        bind(&AudioInfoForm::getInputsPerPortConfigSelectionV2, this, i, _1),
        bind(&AudioInfoForm::setInputsPerPortConfigSelectionV2, this, i, _1, _2));
    refreshList.push_back(cbox2);

    auto *const cbox = addComboBoxItem(
        treeItem, tr("Mixes to outputs on %1 domain").arg(getPortNameV2(i)),
        bind(&AudioInfoForm::outputsPerPortConfigOptionsV2, this, i),
        bind(&AudioInfoForm::getOutputsPerPortConfigSelectionV2, this, i, _1),
        bind(&AudioInfoForm::setOutputsPerPortConfigSelectionV2, this, i, _1, _2));
    refreshList.push_back(cbox);
    outputBoxes.push_back(cbox);
  }
}

void AudioInfoForm::addAudioConfigSectionV2(QTreeWidgetItem *treeItem) {
  auto *const cbox = addComboBoxItem(
      treeItem, tr("Audio configuration"),
      bind(&AudioInfoForm::audioConfigOptionsV2, this),
      bind(&AudioInfoForm::getAudioConfigSelectionV2, this, _1),
      bind(&AudioInfoForm::setAudioConfigSelectionV2, this, _1, _2));
  refreshList.push_back(cbox);
}

void AudioInfoForm::addClockSourceV1(QTreeWidgetItem *treeItem) {
  if (device->contains<AudioClockInfo>()) {
    auto *const cbox =
        addComboBoxItem(treeItem, tr("Clock source"),
                        bind(&AudioInfoForm::clockSourceOptionsV1, this),
                        bind(&AudioInfoForm::getClockSourceV1, this, _1),
                        bind(&AudioInfoForm::setClockSourceV1, this, _1, _2));
    refreshList.push_back(cbox);
  }
}

void AudioInfoForm::addClockSourceV2(QTreeWidgetItem *treeItem) {
  if (device->contains<AudioClockParm>()) {
    auto *const cbox =
        addComboBoxItem(treeItem, tr("Clock source"),
                        bind(&AudioInfoForm::clockSourceOptionsV2, this),
                        bind(&AudioInfoForm::getClockSourceV2, this, _1),
                        bind(&AudioInfoForm::setClockSourceV2, this, _1, _2));
    refreshList.push_back(cbox);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// properties
////////////////////////////////////////////////////////////////////////////////
Word AudioInfoForm::startUSBAudioPortID() const { return 1; }

Word AudioInfoForm::endUSBAudioPortID(const AudioInfo &audioInfo) const {
  return startUSBAudioPortID() + audioInfo.numberOfAudioPorts();
}

QStringList AudioInfoForm::audioFrameOptionsV1() const {
  const auto &audioCfgInfo = device->get<AudioCfgInfo>();
  QStringList audioFrames;
  for (auto i = audioCfgInfo.minNumAudioFrames();
       i <= audioCfgInfo.maxNumAudioFrames(); ++i) {
    audioFrames += QString::number(i);
  }

  return audioFrames;
}

int AudioInfoForm::getAudioFrameSelectionV1(QComboBox *) const {
  const auto &audioCfgInfo = device->get<AudioCfgInfo>();
  return audioCfgInfo.currentNumAudioFrames() -
         audioCfgInfo.minNumAudioFrames();
}

void AudioInfoForm::setAudioFrameSelectionV1(QComboBox *, int value) {
  notifyChanges();

  auto &audioCfgInfo = device->get<AudioCfgInfo>();
  audioCfgInfo.currentNumAudioFrames(value + audioCfgInfo.minNumAudioFrames());
  device->send<SetAudioCfgInfoCommand>(audioCfgInfo);
}

QStringList AudioInfoForm::audioFrameOptionsV2() const {
  const auto &audioGlobalParm = device->get<AudioGlobalParm>();
  QStringList audioFrames;
  for (auto i = audioGlobalParm.minAudioFrames();
       i <= audioGlobalParm.maxAudioFrames(); ++i) {
    audioFrames += QString::number(i);
  }
  return audioFrames;
}

int AudioInfoForm::getAudioFrameSelectionV2(QComboBox *) const {
  const auto &audioGlobalParm = device->get<AudioGlobalParm>();

  //printf("audio frames set to: %d\n", audioGlobalParm.currentAudioFrames());
  return audioGlobalParm.currentAudioFrames() -
         audioGlobalParm.minAudioFrames();
}

void AudioInfoForm::setAudioFrameSelectionV2(QComboBox *, int value) {
  notifyChanges();

  auto &audioGlobalParm = device->get<AudioGlobalParm>();
  audioGlobalParm.currentAudioFrames(value + audioGlobalParm.minAudioFrames());
  device->send<SetAudioGlobalParmCommand>(audioGlobalParm);
}

QStringList AudioInfoForm::audioSyncOptionsV1() const {
  const auto &audioCfgInfo = device->get<AudioCfgInfo>();
  QStringList syncFactors;
  for (auto i = audioCfgInfo.minAllowedSyncFactor();
       i <= audioCfgInfo.maxAllowedSyncFactor(); ++i) {
    syncFactors += QString::number(i);
  }
  return syncFactors;
}

int AudioInfoForm::getAudioSyncSelectionV1(QComboBox *) const {
  const auto &audioCfgInfo = device->get<AudioCfgInfo>();
  return audioCfgInfo.currentSyncFactor() - audioCfgInfo.minAllowedSyncFactor();
}

void AudioInfoForm::setAudioSyncSelectionV1(QComboBox *, int value) {
  notifyChanges();

  auto &audioCfgInfo = device->get<AudioCfgInfo>();
  audioCfgInfo.currentSyncFactor(value + audioCfgInfo.minAllowedSyncFactor());
  device->send<SetAudioCfgInfoCommand>(audioCfgInfo);
}

QStringList AudioInfoForm::audioSyncOptionsV2() const {
  const auto &audioGlobalParm = device->get<AudioGlobalParm>();
  QStringList syncFactors;
  for (auto i = audioGlobalParm.minSyncFactor();
       i <= audioGlobalParm.maxSyncFactor(); ++i) {
    syncFactors += QString::number(i);
  }
  return syncFactors;
}

int AudioInfoForm::getAudioSyncSelectionV2(QComboBox *) const {
  const auto &audioGlobalParm = device->get<AudioGlobalParm>();
  return audioGlobalParm.currentSyncFactor() - audioGlobalParm.minSyncFactor();
}

void AudioInfoForm::setAudioSyncSelectionV2(QComboBox *, int value) {
  notifyChanges();

  auto &audioGlobalParm = device->get<AudioGlobalParm>();
  audioGlobalParm.currentSyncFactor(value + audioGlobalParm.minSyncFactor());
  device->send<SetAudioGlobalParmCommand>(audioGlobalParm);
}

QStringList AudioInfoForm::audioConfigOptionsV1() const {
  const auto &audioCfgInfo = device->get<AudioCfgInfo>();
  QStringList audioConfigs;
  for (int i = 1; i <= audioCfgInfo.numConfigBlocks(); ++i) {
    const auto &configBlock = audioCfgInfo.configBlock(i);

    audioConfigs << tr("%1 - %2Hz, %3 Channels, %4")
                        .arg(QString::number(configBlock.number))
                        .arg(QString::fromStdString(
                            SampleRate::toString(configBlock.sampleRateCode)))
                        .arg(QString::number(configBlock.numAudioChannels))
                        .arg(QString::fromStdString(
                            BitDepth::toString(configBlock.bitDepthCode)));
  }
  return audioConfigs;
}

int AudioInfoForm::getAudioConfigSelectionV1(QComboBox *) const {
  const auto &audioCfgInfo = device->get<AudioCfgInfo>();
  return audioCfgInfo.currentActiveConfig() - 1;
}

void AudioInfoForm::setAudioConfigSelectionV1(QComboBox *, int value) {
  notifyChanges();

  auto &audioCfgInfo = device->get<AudioCfgInfo>();

  audioCfgInfo.currentActiveConfig(value + 1);

  device->send<SetAudioCfgInfoCommand>(audioCfgInfo);

    // set the ins and outs based on the configuration
  device->for_each<AudioPortCfgInfo>([&](AudioPortCfgInfo & portCfgInfo) {
    portCfgInfo.numInputChannels(
        audioCfgInfo.activeConfigBlock().numAudioChannels / 2);
    portCfgInfo.numOutputChannels(
        audioCfgInfo.activeConfigBlock().numAudioChannels / 2);
    device->send<SetAudioPortCfgInfoCommand>(portCfgInfo);
    usleep(2000);
    device->send<GetAudioPortInfoCommand>(portCfgInfo.portID());
  });
  const auto &cmdList = device->get<CommandList>();
  if (cmdList.contains(Command::GetAudioPortPatchbay)) {
    const auto &audioInfo = device->get<AudioInfo>();
    for (auto portID = 1; portID <= audioInfo.numberOfAudioPorts(); ++portID) {
      usleep(2000);
      device->send<GetAudioPortPatchbayCommand>(portID);
    }
  }
}

QStringList AudioInfoForm::mixerConfigOptionsV2() const {
  const auto &audioGlobalParm = device->get<AudioGlobalParm>();
  MixerParm &mixerParm = device->get<MixerParm>(audioGlobalParm.currentActiveConfig());

  QStringList mixerConfigs;

  for (int i = 0; i < mixerParm.mixerBlockCount(); i++) {
    mixerConfigs
        << tr("%1 - %2 Input Buses per Mixer, %3 Mix Buses").arg(QString::number(i + 1))
               .arg(QString::number(mixerParm.mixerBlocks.at(i).maximumInputs()))
               .arg(QString::number(mixerParm.mixerBlocks.at(i).maximumOutputs()));
  }

  return mixerConfigs;
}

int AudioInfoForm::getMixerConfigSelectionV2(QComboBox *) const {
  const auto &audioGlobalParm = device->get<AudioGlobalParm>();
  MixerParm &mixerParm = device->get<MixerParm>(audioGlobalParm.currentActiveConfig());
  return mixerParm.activeMixerConfigurationBlock() - 1;
}

void AudioInfoForm::setMixerConfigSelectionV2(QComboBox *, int value) {
  notifyChanges();

  mixerInterface->activeMixerConfigurationNumber(value + 1);

  /*const auto &audioGlobalParm = device->get<AudioGlobalParm>();
  for (auto audioPortID = 1; audioPortID <= audioGlobalParm.numAudioPorts(); ++audioPortID) {
    mixerInterface->numberInputs(audioPortID, mixerInterface->maximumInputs(value+1));
  }*/
  device->rereadAudioInfo();
  QTimer::singleShot(200, this, SLOT(refreshWidget()));
}

QStringList AudioInfoForm::inputsPerPortConfigOptionsV2(int audioPortID) const {
  QStringList mixerOutputPortConfigs;

  for (int i = 0; i <= mixerInterface->maximumInputs(mixerInterface->activeMixerConfigurationNumber()); i++) {
    mixerOutputPortConfigs
        << tr("%1").arg(QString::number(i));
  }

  return mixerOutputPortConfigs;
}

int AudioInfoForm::getInputsPerPortConfigSelectionV2(int audioPortID, QComboBox*) const {
  MixerPortParm &mixerPortParm = device->get<MixerPortParm>();
  return mixerPortParm.audioPortMixerBlocks.at(audioPortID - 1).numInputs();
}

void AudioInfoForm::setInputsPerPortConfigSelectionV2(int audioPortID, QComboBox*, int value) {
  notifyChanges();

  MixerPortParm &mixerPortParm = device->get<MixerPortParm>();
  mixerPortParm.audioPortMixerBlocks.at(audioPortID - 1).numInputs(value);
  device->send<SetMixerPortParmCommand>(mixerPortParm);
}

QStringList AudioInfoForm::outputsPerPortConfigOptionsV2(int audioPortID) const {
  QStringList mixerOutputPortConfigs;

  for (int i = 0; i <= mixerInterface->maximumOutputs(mixerInterface->activeMixerConfigurationNumber()); i=i+2) {
    mixerOutputPortConfigs
        << tr("%1").arg(QString::number(i));
  }

  return mixerOutputPortConfigs;
}

int AudioInfoForm::getOutputsPerPortConfigSelectionV2(int audioPortID, QComboBox*) const {
  MixerPortParm &mixerPortParm = device->get<MixerPortParm>();
  return mixerPortParm.audioPortMixerBlocks.at(audioPortID - 1).numOutputs()/2;
}

void AudioInfoForm::checkOutputMixTotals(MyComboBox* changedB) {
  MixerPortParm &mixerPortParm = device->get<MixerPortParm>();

  int currentBoxValue = changedB->currentText().toInt();
  int maxValue = mixerInterface->maximumOutputs(mixerInterface->activeMixerConfigurationNumber());
  int totalAllocated = currentBoxValue;

  for (int i = 0; i < outputBoxes.size(); i++) {
    MyComboBox* b = outputBoxes.at(i);
    if (b != changedB) {
      int val = b->currentText().toInt();
      totalAllocated += val;
    }
  }

  if (totalAllocated > maxValue) {
    int newVal = (maxValue - currentBoxValue) / 2;
    int leftover = (maxValue - currentBoxValue) % 2;
    int firstVal = newVal;
    int secondVal = newVal;

    if (firstVal % 2) {
      firstVal++;
      secondVal--;
      if (secondVal < 0) {
        firstVal = secondVal = 0;
      }
    }
    if (!(firstVal == 0)) {
      int count = 0;
      for (int i = 0; i < outputBoxes.size(); i++) {
        MyComboBox* b = outputBoxes.at(i);
        if (b != changedB) {
          if (count == 0) {
            b->blockSignals(true);
            b->setCurrentIndex(firstVal / 2);
            b->blockSignals(false);
            mixerPortParm.audioPortMixerBlocks.at(i).numOutputs(firstVal);
          }
          else {
            b->blockSignals(true);
            b->setCurrentIndex(secondVal / 2);
            b->blockSignals(false);
            mixerPortParm.audioPortMixerBlocks.at(i).numOutputs(secondVal);
          }
          count++;
        }
      }
    }
    else {
      for (int i = 0; i < outputBoxes.size(); i++) {
        MyComboBox* b = outputBoxes.at(i);
        if (b != changedB) {
          b->blockSignals(true);
          b->setCurrentIndex(0);
          mixerPortParm.audioPortMixerBlocks.at(i).numOutputs(0);
          b->blockSignals(false);
        }
      }
    }
  }
}

void AudioInfoForm::setOutputsPerPortConfigSelectionV2(int audioPortID, QComboBox* b, int value) {
  notifyChanges();

  if (!refreshing)
    checkOutputMixTotals((MyComboBox*)b);
  MixerPortParm &mixerPortParm = device->get<MixerPortParm>();
  mixerPortParm.audioPortMixerBlocks.at(audioPortID - 1).numOutputs(b->currentText().toInt());
  device->send<SetMixerPortParmCommand>(mixerPortParm);
}

QStringList AudioInfoForm::audioConfigOptionsV2() const {
  const auto &audioGlobalParm = device->get<AudioGlobalParm>();
  QStringList audioConfigs;

  audioGlobalParm.for_each(
      [&](const AudioGlobalParm::ConfigBlock & configBlock) {
    audioConfigs
        << tr("%1 - %2Hz, %3").arg(QString::number(configBlock.number()))
               .arg(QString::fromStdString(
                   SampleRate::toString(configBlock.sampleRate())))
               .arg(QString::fromStdString(
                   BitDepth::toString(configBlock.bitDepth())));
  });
  return audioConfigs;
}

int AudioInfoForm::getAudioConfigSelectionV2(QComboBox *) const {
  const auto &audioGlobalParm = device->get<AudioGlobalParm>();
  return audioGlobalParm.currentActiveConfig() - 1;
}

void AudioInfoForm::setAudioConfigSelectionV2(QComboBox *, int value) {
  notifyChanges();

  auto &audioGlobalParm = device->get<AudioGlobalParm>();

  audioGlobalParm.currentActiveConfig(value + 1);

  device->send<SetAudioGlobalParmCommand>(audioGlobalParm);

  device->for_each<AudioPortParm>([ = ](AudioPortParm & audioPortParm) {
    usleep(10000);
    const auto &configBlock =
        audioPortParm.block_at(audioGlobalParm.currentActiveConfig());

    Byte avgInChannels =
        (configBlock.minInputChannels() + configBlock.maxInputChannels()) / 2;
    Byte avgOutChannels =
        (configBlock.minOutputChannels() + configBlock.maxOutputChannels()) / 2;
    if (avgInChannels % 2 && avgOutChannels % 2) {
      avgInChannels++;
      avgOutChannels--;
    }
    audioPortParm.numInputChannels(avgInChannels);
    audioPortParm.numOutputChannels(avgOutChannels);

    device->send<SetAudioPortParmCommand>(audioPortParm);
  });
  const auto &cmdList = device->get<CommandList>();
  if (cmdList.contains(Command::GetAudioPatchbayParm)) {
    const auto &audioGlobalParm = device->get<AudioGlobalParm>();
    for (auto audioPortID = 1; audioPortID <= audioGlobalParm.numAudioPorts();
         ++audioPortID) {
      usleep(10000);
      device->send<GetAudioPatchbayParmCommand>(audioPortID);
    }
  }
  device->rereadAudioInfo();
  QTimer::singleShot(200, this, SLOT(refreshWidget()));
}

QString AudioInfoForm::getJackNumberV1(Word portID) const {
  const auto &audioPortInfo = device->get<AudioPortInfo>(portID);
  return QString::number(audioPortInfo.portInfo().common.jack);
}

QString AudioInfoForm::getJackNumberV2(Word audioPortID) const {
  const auto &audioPortParm = device->get<AudioPortParm>(audioPortID);
  return QString::number(audioPortParm.jack());
}

int AudioInfoForm::getMaxPortNameV1(Word portID) const {
  const auto &audioPortInfo = device->get<AudioPortInfo>(portID);
  return audioPortInfo.maxPortName();
}

QString AudioInfoForm::getPortNameV1(Word portID) const {
  const auto &audioPortInfo = device->get<AudioPortInfo>(portID);
  return QString::fromStdString(audioPortInfo.portName());
}

void AudioInfoForm::setPortNameV1(Word portID, QString value) {
  notifyChanges();

  auto &audioPortInfo = device->get<AudioPortInfo>(portID);
  audioPortInfo.portName(value.toStdString());
  device->send<SetAudioPortInfoCommand>(audioPortInfo);
}

int AudioInfoForm::getMaxPortNameV2(Word audioPortID) const {
  const auto &audioPortParm = device->get<AudioPortParm>(audioPortID);
  return audioPortParm.maxPortName();
}

QString AudioInfoForm::getPortNameV2(Word audioPortID) const {
  const auto &audioPortParm = device->get<AudioPortParm>(audioPortID);
  return QString::fromStdString(audioPortParm.portName());
}

void AudioInfoForm::setPortNameV2(Word audioPortID, QString value) {
  notifyChanges();

  auto &audioPortParm = device->get<AudioPortParm>(audioPortID);
  audioPortParm.portName(value.toStdString());
  device->send<SetAudioPortParmCommand>(audioPortParm);
}

bool AudioInfoForm::getMacPCEnabledV1(Word portID) const {
  const auto &audioPortInfo = device->get<AudioPortInfo>(portID);
  return audioPortInfo.isPCAudioEnabled();
}

void AudioInfoForm::setMacPCEnabledV1(Word portID, bool value) {
  notifyChanges();

  auto &audioPortInfo = device->get<AudioPortInfo>(portID);
  audioPortInfo.setPCAudioEnabled(value);
  device->send<SetAudioPortInfoCommand>(audioPortInfo);
}

bool AudioInfoForm::getMacPCEnabledV2(Word audioPortID) const {
  const auto &audioPortParm = device->get<AudioPortParm>(audioPortID);
  const auto &usbDevice = audioPortParm.usbDevice();
  return usbDevice.isPCAudioEnabled();
}

void AudioInfoForm::setMacPCEnabledV2(Word audioPortID, bool value) {
  notifyChanges();

  auto &audioPortParm = device->get<AudioPortParm>(audioPortID);
  auto &usbDevice = audioPortParm.usbDevice();
  usbDevice.setPCAudioEnabled(value);
  device->send<SetAudioPortParmCommand>(audioPortParm);
}

bool AudioInfoForm::getIOSEnabledV1(Word portID) const {
  const auto &audioPortInfo = device->get<AudioPortInfo>(portID);
  return audioPortInfo.isIOSAudioEnabled();
}

void AudioInfoForm::setIOSEnabledV1(Word portID, bool value) {
  notifyChanges();

  auto &audioPortInfo = device->get<AudioPortInfo>(portID);
  audioPortInfo.setIOSAudioEnabled(value);
  device->send<SetAudioPortInfoCommand>(audioPortInfo);
}

bool AudioInfoForm::getIOSEnabledV2(Word audioPortID) const {
  const auto &audioPortParm = device->get<AudioPortParm>(audioPortID);
  const auto &usbDevice = audioPortParm.usbDevice();
  return usbDevice.isIOSAudioEnabled();
}

void AudioInfoForm::setIOSEnabledV2(Word audioPortID, bool value) {
  notifyChanges();

  auto &audioPortParm = device->get<AudioPortParm>(audioPortID);
  auto &usbDevice = audioPortParm.usbDevice();
  usbDevice.setIOSAudioEnabled(value);
  device->send<SetAudioPortParmCommand>(audioPortParm);
}

QString AudioInfoForm::getNumInputChannelsLabelV1(Word portID) const {
  const auto &portCfgInfo = device->get<AudioPortCfgInfo>(portID);
  return QString::number(portCfgInfo.numInputChannels());
}

QStringList AudioInfoForm::getInputOptionsV1(Word portID) const {
  const auto &audioCfgInfo = device->get<AudioCfgInfo>();
  const auto &selConfig = audioCfgInfo.currentActiveConfig() - 1;

  const auto &portCfgInfo = device->get<AudioPortCfgInfo>(portID);
  Q_ASSERT(selConfig < static_cast<int>(portCfgInfo.numConfigBlocks()));
  const auto &configBlock = portCfgInfo.block_at(selConfig);

  QStringList inputOptions;
  for (int i = configBlock.minInputChannels();
       i <= configBlock.maxInputChannels(); ++i) {
    inputOptions << QString::number(i);
  }
  return inputOptions;
}

int AudioInfoForm::getSelectedInputV1(Word portID, QComboBox *) const {
  const auto &audioCfgInfo = device->get<AudioCfgInfo>();
  const auto &selConfig = audioCfgInfo.currentActiveConfig() - 1;

  const auto &portCfgInfo = device->get<AudioPortCfgInfo>(portID);
  Q_ASSERT(selConfig < static_cast<int>(portCfgInfo.numConfigBlocks()));
  const auto &configBlock = portCfgInfo.block_at(selConfig);

  return portCfgInfo.numInputChannels() - configBlock.minInputChannels();
}

void AudioInfoForm::setSelectedInputV1(Word portID, QComboBox *,
                                       int selection) {
  notifyChanges();

  auto &portCfgInfo = device->get<AudioPortCfgInfo>(portID);
  const auto &audioCfgInfo = device->get<AudioCfgInfo>();
  const auto &activeConfig = audioCfgInfo.currentActiveConfig() - 1;
  const auto &block = portCfgInfo.block_at(activeConfig);

  const auto &activeConfigBlock = audioCfgInfo.activeConfigBlock();

  const auto inChannels = (Word)((selection + 1) & 0xFFFF);
  if (portCfgInfo.numInputChannels() != inChannels) {
    portCfgInfo.numInputChannels(inChannels);

    if (portCfgInfo.totalChannels() > activeConfigBlock.numAudioChannels) {
      portCfgInfo.numOutputChannels(
          ((1 + block.maxOutputChannels()) - portCfgInfo.numInputChannels()));
    }

    device->send<SetAudioPortCfgInfoCommand>(portCfgInfo);
    const auto &cmdList = device->get<CommandList>();
    if (cmdList.contains(Command::GetAudioPortPatchbay)) {
      const auto &audioInfo = device->get<AudioInfo>();
      for (auto portID = 1; portID <= audioInfo.numberOfAudioPorts();
           ++portID) {
        usleep(1000);
        device->send<GetAudioPortPatchbayCommand>(portID);
      }
    }
  }
}

QString AudioInfoForm::getNumInputChannelsLabelV2(Word audioPortID) const {
  const auto &audioPortParm = device->get<AudioPortParm>(audioPortID);
  return QString::number(audioPortParm.numInputChannels());
}

QStringList AudioInfoForm::getInputOptionsV2(Word audioPortID) const {
  const auto &audioGlobalParm = device->get<AudioGlobalParm>();
  const auto &currentActiveConfigID = audioGlobalParm.currentActiveConfig();
  const auto &audioPortParm = device->get<AudioPortParm>(audioPortID);
  const auto &configBlock = audioPortParm.block_at(currentActiveConfigID);

  QStringList inputOptions;
  for (int i = configBlock.minInputChannels();
       i <= configBlock.maxInputChannels(); ++i) {
    inputOptions << QString::number(i);
  }
  return inputOptions;
}

int AudioInfoForm::getSelectedInputV2(Word audioPortID, QComboBox *) const {
  const auto &audioGlobalParm = device->get<AudioGlobalParm>();
  const auto &currentActiveConfigID = audioGlobalParm.currentActiveConfig();
  const auto &audioPortParm = device->get<AudioPortParm>(audioPortID);
  const auto &configBlock = audioPortParm.block_at(currentActiveConfigID);

  return audioPortParm.numInputChannels() - configBlock.minInputChannels();
}

void AudioInfoForm::setSelectedInputV2(Word audioPortID, QComboBox *,
                                       int selection) {
  notifyChanges();

  auto &audioGlobalParm = device->get<AudioGlobalParm>();
  const auto &currentActiveConfigID = audioGlobalParm.currentActiveConfig();
  auto &audioPortParm = device->get<AudioPortParm>(audioPortID);
  const auto &configBlock = audioPortParm.block_at(currentActiveConfigID);

  Byte inChannels = ((selection + configBlock.minInputChannels()) & 0x7F);
  if (audioPortParm.numInputChannels() != inChannels) {
    audioPortParm.numInputChannels(inChannels);

    if ((inChannels + audioPortParm.numOutputChannels()) >
        configBlock.maxAudioChannels()) {

      audioPortParm.numOutputChannels(configBlock.maxAudioChannels() -
                                      inChannels);
    }

    device->send<SetAudioPortParmCommand>(audioPortParm);
    const auto &cmdList = device->get<CommandList>();
    if (cmdList.contains(Command::GetAudioPatchbayParm)) {
      const AudioGlobalParm audioGlobal = device->get<AudioGlobalParm>();
      for (auto portID = 1; portID <= audioGlobal.numAudioPorts(); ++portID) {
        usleep(1000);
        device->send<GetAudioPatchbayParmCommand>(portID);
      }
    }
  }
}

QString AudioInfoForm::getNumOutputChannelsLabelV1(Word portID) const {
  const auto &portCfgInfo = device->get<AudioPortCfgInfo>(portID);
  return QString::number(portCfgInfo.numOutputChannels());
}

QStringList AudioInfoForm::getOutputOptionsV1(Word portID) const {
  const auto &audioCfgInfo = device->get<AudioCfgInfo>();
  const auto &selConfig = audioCfgInfo.currentActiveConfig() - 1;

  const auto &portCfgInfo = device->get<AudioPortCfgInfo>(portID);
  Q_ASSERT(selConfig < static_cast<int>(portCfgInfo.numConfigBlocks()));
  const auto &configBlock = portCfgInfo.block_at(selConfig);

  QStringList inputOptions;
  for (int i = configBlock.minInputChannels();
       i <= configBlock.maxInputChannels(); ++i) {
    inputOptions << QString::number(i);
  }
  return inputOptions;
}

int AudioInfoForm::getSelectedOutputV1(Word portID, QComboBox *) const {
  const auto &audioCfgInfo = device->get<AudioCfgInfo>();
  const auto &selConfig = audioCfgInfo.currentActiveConfig() - 1;

  const auto &portCfgInfo = device->get<AudioPortCfgInfo>(portID);
  Q_ASSERT(selConfig < static_cast<int>(portCfgInfo.numConfigBlocks()));
  const auto &configBlock = portCfgInfo.block_at(selConfig);

  return portCfgInfo.numOutputChannels() - configBlock.minOutputChannels();
}

void AudioInfoForm::setSelectedOutputV1(Word portID, QComboBox *,
                                        int selection) {
  notifyChanges();

  auto &audioPortCfgInfo = device->get<AudioPortCfgInfo>(portID);
  const auto &audioCfgInfo = device->get<AudioCfgInfo>();
  const auto &activeConfig = audioCfgInfo.currentActiveConfig() - 1;
  const auto &block = audioPortCfgInfo.block_at(activeConfig);

  const auto &activeConfigBlock = audioCfgInfo.activeConfigBlock();

  const auto outChannels = (Word)((selection + 1) & 0xFFFF);
  if (audioPortCfgInfo.numOutputChannels() != outChannels) {
    audioPortCfgInfo.numOutputChannels(outChannels);

    if (audioPortCfgInfo.totalChannels() > activeConfigBlock.numAudioChannels) {
      audioPortCfgInfo.numInputChannels((1 + block.maxInputChannels()) -
                                        audioPortCfgInfo.numOutputChannels());
    }

    device->send<SetAudioPortCfgInfoCommand>(audioPortCfgInfo);
    const auto &cmdList = device->get<CommandList>();
    if (cmdList.contains(Command::GetAudioPortPatchbay)) {
      const auto &audioInfo = device->get<AudioInfo>();
      for (auto portID = 1; portID <= audioInfo.numberOfAudioPorts();
           ++portID) {
        usleep(1000);
        device->send<GetAudioPortPatchbayCommand>(portID);
      }
    }
  }
}

QString AudioInfoForm::getNumOutputChannelsLabelV2(Word audioPortID) const {
  const auto &audioPortParm = device->get<AudioPortParm>(audioPortID);
  return QString::number(audioPortParm.numOutputChannels());
}

QStringList AudioInfoForm::getOutputOptionsV2(Word audioPortID) const {
  const auto &audioGlobalParm = device->get<AudioGlobalParm>();
  const auto &currentActiveConfigID = audioGlobalParm.currentActiveConfig();
  const auto &audioPortParm = device->get<AudioPortParm>(audioPortID);
  const auto &configBlock = audioPortParm.block_at(currentActiveConfigID);

  QStringList outputOptions;
  for (int i = configBlock.minOutputChannels();
       i <= configBlock.maxOutputChannels(); ++i) {
    outputOptions << QString::number(i);
  }
  return outputOptions;
}

int AudioInfoForm::getSelectedOutputV2(Word audioPortID, QComboBox *) const {
  const auto &audioGlobalParm = device->get<AudioGlobalParm>();
  const auto &currentActiveConfigID = audioGlobalParm.currentActiveConfig();
  const auto &audioPortParm = device->get<AudioPortParm>(audioPortID);
  const auto &configBlock = audioPortParm.block_at(currentActiveConfigID);

  return audioPortParm.numOutputChannels() - configBlock.minOutputChannels();
}

void AudioInfoForm::setSelectedOutputV2(Word audioPortID, QComboBox *,
                                        int selection) {
  notifyChanges();

  auto &audioGlobalParm = device->get<AudioGlobalParm>();
  const auto &currentActiveConfigID = audioGlobalParm.currentActiveConfig();
  auto &audioPortParm = device->get<AudioPortParm>(audioPortID);
  const auto &configBlock = audioPortParm.block_at(currentActiveConfigID);

  Byte outChannels = ((selection + configBlock.minOutputChannels()) & 0x7F);
  if (audioPortParm.numOutputChannels() != outChannels) {
    audioPortParm.numOutputChannels(outChannels);

    if ((outChannels + audioPortParm.numInputChannels()) >
        configBlock.maxAudioChannels()) {

      audioPortParm.numInputChannels(configBlock.maxAudioChannels() -
                                     outChannels);
    }

    device->send<SetAudioPortParmCommand>(audioPortParm);
    const auto &cmdList = device->get<CommandList>();
    if (cmdList.contains(Command::GetAudioPatchbayParm)) {
      const AudioGlobalParm audioGlobal = device->get<AudioGlobalParm>();
      for (auto portID = 1; portID <= audioGlobal.numAudioPorts(); ++portID) {
        usleep(1000);
        device->send<GetAudioPatchbayParmCommand>(portID);
      }
    }
  }
}

QStringList AudioInfoForm::clockSourceOptionsV1() const {
  const auto &clockInfo = device->get<AudioClockInfo>();
  QStringList result;
  clockInfo.for_each([&](const AudioClockInfo::SourceBlock & block) {
    result << QString::fromStdString(block.toString());
  });
  return result;
}

int AudioInfoForm::getClockSourceV1(QComboBox *) const {
  const auto &clockInfo = device->get<AudioClockInfo>();
  return clockInfo.activeSourceBlock() - 1;
}

void AudioInfoForm::setClockSourceV1(QComboBox *, int selection) {
  notifyChanges();

  auto &clockInfo = device->get<AudioClockInfo>();
  clockInfo.activeSourceBlock(selection + 1);
  device->send<SetAudioClockInfoCommand>(clockInfo);
}

QStringList AudioInfoForm::clockSourceOptionsV2() const {
  const auto &clockParm = device->get<AudioClockParm>();
  QStringList result;
  clockParm.for_each([&](const AudioClockParm::SourceBlock & block) {
    result << QString::fromStdString(block.toString());
  });
  return result;
}

int AudioInfoForm::getClockSourceV2(QComboBox *) const {
  const auto &clockParm = device->get<AudioClockParm>();
  return clockParm.activeSourceBlock() - 1;
}

void AudioInfoForm::setClockSourceV2(QComboBox *, int selection) {
  notifyChanges();

  auto &clockParm = device->get<AudioClockParm>();
  clockParm.activeSourceBlock(selection + 1);
  device->send<SetAudioClockParmCommand>(clockParm);
}

QString AudioInfoForm::getUSBDeviceHostTypeV2(Word audioPortID) const {
  const auto &audioDeviceParm = device->get<AudioDeviceParm>(audioPortID);
  const auto &usbDevice = audioDeviceParm.usbDevice();
  QString result;

  switch (usbDevice.hostType()) {
    case HostType::NoHost: {
      result = tr("None");
      break;
    }
    case HostType::MacPC: {
      result = tr("Mac/PC");
      break;
    }
    case HostType::iOSDevice: {
      result = tr("iOS");
      break;
    }
    default:
      result = tr("N/A");
      break;
  }

  return result;
}

QString AudioInfoForm::getUSBDeviceHostNameV2(Word audioPortID) const {
  const auto &audioDeviceParm = device->get<AudioDeviceParm>(audioPortID);
  const auto &usbDevice = audioDeviceParm.usbDevice();
  QString result = QString::fromStdString(usbDevice.hostName());
  if (result.size() == 0) {
    result = tr("N/A");
  }
  return result;
}

QString AudioInfoForm::getUSBHostDeviceNumberV2(Word audioPortID) const {
  const auto &audioPortParm = device->get<AudioPortParm>(audioPortID);
  const auto &usbHost = audioPortParm.usbHost();
  return QString::number(usbHost.deviceNumber());
}

QString AudioInfoForm::getEthernetDeviceNumberV2(Word audioPortID) const {
  const auto &audioPortParm = device->get<AudioPortParm>(audioPortID);
  const auto &ethernet = audioPortParm.ethernet();
  return QString::number(ethernet.deviceNumber());
}

QString AudioInfoForm::getAnaloguePortV2(Word audioPortID) const {
  const auto &audioPortParm = device->get<AudioPortParm>(audioPortID);
  const auto &analogue = audioPortParm.analogue();
  return QString::number(analogue.port());
}

QString AudioInfoForm::getConnectedV2(Word audioPortID) const {
  const auto &audioDeviceParm = device->get<AudioDeviceParm>(audioPortID);
  const auto &usbHost = audioDeviceParm.usbHost();
  return ((usbHost.connected()) ? tr("Yes") : tr("No"));
}

bool AudioInfoForm::getReservedV2(Word audioPortID) const {
  const auto &audioDeviceParm = device->get<AudioDeviceParm>(audioPortID);
  const auto &usbHost = audioDeviceParm.usbHost();
  return usbHost.reserved();
}

void AudioInfoForm::setReservedV2(Word audioPortID, bool value) {
  notifyChanges();

  auto &audioDeviceParm = device->get<AudioDeviceParm>(audioPortID);
  auto &usbHost = audioDeviceParm.usbHost();
  usbHost.reserved(value);
  device->send<SetAudioDeviceParmCommand>(audioDeviceParm);
}

bool AudioInfoForm::reservedEnabledV2(Word audioPortID) const {
  auto &audioDeviceParm = device->get<AudioDeviceParm>(audioPortID);
  auto &usbHost = audioDeviceParm.usbHost();
  return ((usbHost.vendorID() != 0x0000) && (usbHost.productID() != 0x0000));
}

QString AudioInfoForm::getMaxInChannelsV2(Word audioPortID) const {
  auto &audioDeviceParm = device->get<AudioDeviceParm>(audioPortID);
  auto &usbHost = audioDeviceParm.usbHost();
  return QString::number(usbHost.maxInChannels());
}

QString AudioInfoForm::getMaxOutChannelsV2(Word audioPortID) const {
  auto &audioDeviceParm = device->get<AudioDeviceParm>(audioPortID);
  auto &usbHost = audioDeviceParm.usbHost();
  return QString::number(usbHost.maxOutChannels());
}

QString AudioInfoForm::getUSBHostVendorNameV2(Word audioPortID) const {
  auto &audioDeviceParm = device->get<AudioDeviceParm>(audioPortID);
  auto &usbHost = audioDeviceParm.usbHost();
  return QString::fromStdString(usbHost.vendorName());
}

QString AudioInfoForm::getUSBHostProductNameV2(Word audioPortID) const {
  auto &audioDeviceParm = device->get<AudioDeviceParm>(audioPortID);
  auto &usbHost = audioDeviceParm.usbHost();
  return QString::fromStdString(usbHost.productName());
}

void AudioInfoForm::saveOldValues()
{
  if (device->contains<AudioGlobalParm>()) { // v2
    oldAudioGlobalParm = device->get<AudioGlobalParm>();
    oldAudioDeviceParms.clear();
    oldAudioPortParms.clear();
    for (int i = 1; i <= oldAudioGlobalParm.numAudioPorts(); i++) {
      oldAudioDeviceParms.push_back(&device->get<AudioDeviceParm>(i));
      oldAudioPortParms.push_back(&device->get<AudioPortParm>(i));
    }
    oldAudioClockParm = device->get<AudioClockParm>();
    if (device->containsType<MixerParm>()) {
      oldMixerParm = device->get<MixerParm>(oldAudioGlobalParm.currentActiveConfig());
      oldMixerPortParm = device->get<MixerPortParm>();
    }
  }
  else { // v1
    oldAudioInfo = device->get<AudioInfo>();
    oldAudioCfgInfo = device->get<AudioCfgInfo>();

    for (int i = 1; i <= oldAudioInfo.numberOfAudioPorts(); i++) {
      const auto &portCfgInfo = device->get<AudioPortCfgInfo>(i);
      const auto &audioPortInfo = device->get<AudioPortInfo>(i);

      oldPortNames.push_back(audioPortInfo.portName());
      oldPortIn.push_back(portCfgInfo.numInputChannels());
      oldPortOut.push_back(portCfgInfo.numOutputChannels());
      oldPortIOSEnabled.push_back(audioPortInfo.isIOSAudioEnabled());
      oldPortMacPCEnabled.push_back(audioPortInfo.isPCAudioEnabled());
    }
    oldAudioClockInfo = device->get<AudioClockInfo>();
  }
}

void AudioInfoForm::restoreOldValues()
{
  if (device->contains<AudioGlobalParm>()) { // v2
    device->send<SetAudioGlobalParmCommand>(oldAudioGlobalParm);
    for (int i = 0; i < oldAudioGlobalParm.numAudioPorts(); i++) {
      device->send<SetAudioDeviceParmCommand>(*oldAudioDeviceParms.at(i));
      device->send<SetAudioPortParmCommand>(*oldAudioPortParms.at(i));
    }
    device->send<SetAudioClockParmCommand>(oldAudioClockParm);
    if (device->containsType<MixerParm>()) {
      device->send<SetMixerParmCommand>(oldMixerParm);
      device->send<SetMixerPortParmCommand>(oldMixerPortParm);
    }
  }
  else { // v1
    device->send<SetAudioCfgInfoCommand>(oldAudioCfgInfo);
    oldAudioInfo = device->get<AudioInfo>();
    for (int i = 0; i < oldAudioInfo.numberOfAudioPorts(); i++) {
      auto &portCfgInfo = device->get<AudioPortCfgInfo>(i + 1);
      auto &audioPortInfo = device->get<AudioPortInfo>(i + 1);

      audioPortInfo.portName(oldPortNames.at(i));
      portCfgInfo.numInputChannels(oldPortIn.at(i));
      portCfgInfo.numOutputChannels(oldPortIn.at(i));
      audioPortInfo.setIOSAudioEnabled(oldPortIOSEnabled.at(i));
      audioPortInfo.setPCAudioEnabled(oldPortMacPCEnabled.at(i));

      device->send<SetAudioPortCfgInfoCommand>(portCfgInfo);
      usleep(2000);
      device->send<SetAudioPortInfoCommand>(audioPortInfo);
    }
    device->send<SetAudioClockInfoCommand>(oldAudioClockInfo);
  }
}

void AudioInfoForm::notifyChanges() {
  changesHaveBeenMade = true;
  ui->frame->show();
}
