/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "DeviceInfoForm.h"
#include "ui_DeviceInfoForm.h"

#include "ACK.h"
#include "DevicePID.h"
#include "EthernetPortInfo.h"
#include "InfoList.h"
#include "MIDIInfo.h"
#include "MyAlgorithms.h"
#include "MyConverters.h"
#include "MyLineEdit.h"
#include "MyLineEdit.h"
#include "TreeUtils.h"

#include <QHostAddress>
#include <QRegExp>

#ifndef Q_MOC_RUN
#include <boost/bind.hpp>
#include <boost/range.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/shared_ptr.hpp>
#endif

using namespace GeneSysLib;
using namespace MyAlgorithms;
using namespace TreeUtils;
using namespace boost::adaptors;
using namespace boost::range;
using namespace boost;

const QString DeviceInfoForm::DefaultWindowTitle = "Device Info";

DeviceInfoForm::DeviceInfoForm(DeviceInfoPtr _device, QWidget* _parent)
    : RefreshObject(_parent),
      device(_device),
      ui(new Ui::DeviceInfoForm) {
  ui->setupUi(this);

  this->setWindowTitle(DefaultWindowTitle);
  treeBuilt = false;

  Q_ASSERT(device);

  auto addHandler = [this](CmdEnum cmd, Handler handler) {
    this->registeredHandlerIDs[cmd] = device->registerHandler(cmd, handler);
  }
  ;

  addHandler(Command::ACK,
             bind(&DeviceInfoForm::ackCallback, this, _1, _2, _3, _4));

  tie(deviceID, transID) = device->getInfo();

  connect(this, SIGNAL(informationAvailable()), this, SLOT(buildInformation()),
          Qt::QueuedConnection);
  connect(this, SIGNAL(informationAvailable()), this, SLOT(setupSideBar()),
          Qt::QueuedConnection);
  connect(device.get(), SIGNAL(queryCompleted(Screen, CommandQList)), this,
          SLOT(queryCompleted(Screen, CommandQList)), Qt::QueuedConnection);

  CommandQList query;
  query << Command::RetInfo;
  device->startQuery(InformationScreen, query);
}

DeviceInfoForm::~DeviceInfoForm() {
  for (const auto& handler : registeredHandlerIDs) {
    device->unRegisterHandler(handler.first, handler.second);
  }
}

void DeviceInfoForm::setupSideBar() {
  switch ((DevicePID::Enum) deviceID.pid()) {
  case DevicePID::iConnect4Audio:
    ui->pictureLabel
        ->setPixmap(QPixmap(":/Products/Images/block_ica4plusdevice.png"));
    break;
  case DevicePID::iConnect2Audio:
    ui->pictureLabel
        ->setPixmap(QPixmap(":/Products/Images/block_ica2plusdevice.png"));
    break;
  case DevicePID::MIO:
      ui->pictureLabel
          ->setPixmap(QPixmap(":/Products/Images/block_miodevice.png"));
      break;

  case DevicePID::MIO2:
    ui->pictureLabel->setPixmap(QPixmap(":/Products/Images/block_mio2device.png"));
    break;

  case DevicePID::MIO4:
    ui->pictureLabel->setPixmap(QPixmap(":/Products/Images/block_mio4device.png"));
    break;

  case DevicePID::MIO10:
    ui->pictureLabel->setPixmap(QPixmap(":/Products/Images/block_mio10device.png"));
    break;

  case DevicePID::PlayAudio12:
    ui->pictureLabel->setPixmap(QPixmap(":/Products/Images/block_pa12device.png"));
    break;

  case DevicePID::iConnect1:
      ui->pictureLabel
          ->setPixmap(QPixmap(":/Products/Images/block_icm1device.png"));
      break;

    case DevicePID::iConnect2Plus:
      ui->pictureLabel
          ->setPixmap(QPixmap(":/Products/Images/block_icm2plusdevice.png"));
      break;

  case DevicePID::iConnect4Plus:
    ui->pictureLabel
        ->setPixmap(QPixmap(":/Products/Images/block_icm4plusdevice.png"));
    break;

  case DevicePID::ConnectAudio24:
    ui->pictureLabel
        ->setPixmap(QPixmap(":/Products/Images/block_ca24device.png"));
    break;
  }

  if (device->containsInfo(InfoID::AccessoryName)) {
    ui->accessoryLabel->setText(QString::fromStdString(
        device->infoData(InfoID::AccessoryName).infoString()));
  } else {
    ui->accessoryLabel->setText(QString());
  }

  if (device->containsInfo(InfoID::DeviceName)) {
    ui->deviceNameLabel->setText(QString::fromStdString(
        device->infoData(InfoID::DeviceName).infoString()));
  }
}

void DeviceInfoForm::buildInformation() {
  treeBuilt = true;

  refreshDataList.clear();

  auto* deviceInformationTreeItem = new QTreeWidgetItem(ui->treeWidget);
  deviceInformationTreeItem->setText(0, tr("Device Information"));

  // add device specific information to the tree
  const auto& infoToInfoID = [](Info info) { return info.infoID(); }
  ;
  device->for_each<Info>([&](Info & info) {
    addInfoWidget(deviceInformationTreeItem, infoToInfoID(info));
  });

  // ethernet information
  if (device->contains<MIDIInfo>()) {
    auto& midiInfo = device->get<MIDIInfo>();

    if (midiInfo.numEthernetJacks() > 0) {
      auto* ethernetInformationTreeItem = new QTreeWidgetItem(ui->treeWidget);
      ethernetInformationTreeItem->setText(0, tr("Ethernet Information"));

      QTreeWidgetItem* currentTreeItem = ethernetInformationTreeItem;

      device->for_each<EthernetPortInfo>([&](EthernetPortInfo & ethInfo) {
        const auto& portID = ethInfo.portJackID();

        // if there are more than 1 ethernet jack then add a seperator between
        // them
        if (midiInfo.numEthernetJacks() > 1) {
          currentTreeItem = new QTreeWidgetItem(ethernetInformationTreeItem);
          currentTreeItem->setText(0, tr("Ethernet Jack"));
          currentTreeItem->setText(
              1, QString::number(ethInfo.portJackID()));
        }

        addEthernetRow(currentTreeItem, portID);
      });
    }
  }

  ui->treeWidget->expandAll();
  ui->treeWidget->resizeColumnToContents(0);
}

void DeviceInfoForm::queryCompleted(Screen sc, CommandQList) {
  if (sc == InformationScreen) {
    if (!treeBuilt) {
      emit informationAvailable();
    }
  }
}

void DeviceInfoForm::ackCallback(CmdEnum, DeviceID, Word,
                                 commandData_t commandData) {
  const auto& ackData = commandData.get<ACK>();

  if ((ackData.commandID() == Command::SetInfo) ||
      (ackData.commandID() == Command::SetEthernetPortInfo)) {
    emit updateMessage(
        (ackData.errorCode() == 0x00)
            ? tr("Information successfully written.")
            : tr("Error writting information. Please try again."),
        kMessageTimeout);
    emit requestRefresh();
  }
}

void DeviceInfoForm::refreshWidget() {
  for (auto* ptr : refreshDataList) {
    ptr->refreshData();
  }
}

////////////////////////////////////////////////////////////////////////////////
/// add methods
////////////////////////////////////////////////////////////////////////////////
void DeviceInfoForm::addEthernetRow(QTreeWidgetItem* currentTreeItem,
                                    const Word& portID) {
  // IP mode
  addComboBox(currentTreeItem, tr("IP Mode"),
              boost::bind(&DeviceInfoForm::ipModeOptions, this),
              boost::bind(&DeviceInfoForm::ipMode, this, portID, _1),
              boost::bind(&DeviceInfoForm::ipMode, this, portID, _1, _2));

  // static IP address
  addNetAddrLineEdit(
      currentTreeItem, tr("Static IP Address"),
      boost::bind(&DeviceInfoForm::staticIPAddress, this, portID),
      boost::bind(&DeviceInfoForm::staticIPAddress, this, portID, _1));

  // static subnet mask
  addNetAddrLineEdit(
      currentTreeItem, tr("Static Subnet Mask"),
      bind(&DeviceInfoForm::staticSubnetMask, this, portID),
      bind(&DeviceInfoForm::staticSubnetMask, this, portID, _1));

  // static gateway
  addNetAddrLineEdit(currentTreeItem, tr("Static Gateway"),
                     bind(&DeviceInfoForm::staticGateway, this, portID),
                     bind(&DeviceInfoForm::staticGateway, this, portID, _1));

  // current IP
  addLabel(currentTreeItem, tr("Current IP Address"),
           bind(&DeviceInfoForm::currentIPAddress, this, portID));

  // current Subnet mask
  addLabel(currentTreeItem, tr("Current Subnet Mask"),
           bind(&DeviceInfoForm::currentSubnetMask, this, portID));

  // current gateway
  addLabel(currentTreeItem, tr("Current Gateway"),
           bind(&DeviceInfoForm::currentGateway, this, portID));

  // mac address
  addLabel(currentTreeItem, tr("MAC Address"),
           bind(&DeviceInfoForm::macAddress, this, portID));

  // device name
  addLabel(currentTreeItem, tr("Device Name"),
           bind(&DeviceInfoForm::getDeviceNetName, this, portID));
}

void DeviceInfoForm::addInfoWidget(QTreeWidgetItem* treeItem,
                                   InfoIDEnum infoID) {
  auto& infoListData = device->get<InfoList>();
  Q_ASSERT(infoListData.contains(infoID));

  const auto& maxLength = infoListData.record_at(infoID).maxLength();
  if (maxLength > 0) {
    auto* infoLineEdit = addLineEditItem(
        ui->treeWidget, treeItem, infoStringTitle(infoID), maxLength,
        bind(&DeviceInfoForm::infoString, this, infoID),
        bind(&DeviceInfoForm::setInfoString, this, infoID, _1));
    if (infoID == InfoID::DeviceName) {
      connect(infoLineEdit, SIGNAL(textChanged(QString)), ui->deviceNameLabel,
              SLOT(setText(QString)));
    }
    refreshDataList.push_back(infoLineEdit);
  } else {
    const auto& getFunc = bind(&DeviceInfoForm::infoString, this, infoID);
    addLabelItem(ui->treeWidget, treeItem, infoStringTitle(infoID), getFunc);
  }
}

void DeviceInfoForm::addComboBox(QTreeWidgetItem* treeItem, QString title,
                                 const GetComboBoxOptions& options,
                                 const GetComboBoxSelectionFunctor& getFunc,
                                 const SetComboBoxSelectionFunctor& setFunc) {
  auto* cbox = addComboBoxItem(treeItem, title, options, getFunc, setFunc);
  refreshDataList.push_back(cbox);
}

void DeviceInfoForm::addNetAddrLineEdit(QTreeWidgetItem* treeItem,
                                        QString title,
                                        const GetQStringFunctor& getFunc,
                                        const SetQStringFunctor& setFunc) {
  auto* lineEdit =
      addLineEditItem(ui->treeWidget, treeItem, title, 16, getFunc, setFunc);
  lineEdit->setValidator(new QRegExpValidator(NetAddrTools::ipRegEx()));
  refreshDataList.push_back(lineEdit);
}

void DeviceInfoForm::addLabel(QTreeWidgetItem* treeItem, QString title,
                              const GetQStringFunctor& getFunc) {
  auto* label = addLabelItem(ui->treeWidget, treeItem, title, getFunc);
  refreshDataList.push_back(label);
}

////////////////////////////////////////////////////////////////////////////////
/// getter helpers
////////////////////////////////////////////////////////////////////////////////

QString DeviceInfoForm::infoString(InfoIDEnum infoID) const {
  const auto& info = device->infoData(infoID);
  const auto& infoStr = info.infoString();
  return QString::fromStdString(infoStr);
}

QStringList DeviceInfoForm::ipModeOptions() const {
  QStringList options;
  options << tr("Static IP") << tr("Dynamic IP");
  return options;
}

int DeviceInfoForm::ipMode(Word portID, QComboBox*) const {
  const auto& ethInfo = device->get<EthernetPortInfo>(portID);
  return static_cast<int>(ethInfo.ipMode());
}

QString DeviceInfoForm::staticIPAddress(Word portID) const {
  const auto& ethInfo = device->get<EthernetPortInfo>(portID);
  return NetAddrTools::fromNetAddr(ethInfo.staticIPAddress());
}

QString DeviceInfoForm::staticSubnetMask(Word portID) const {
  const auto& ethInfo = device->get<EthernetPortInfo>(portID);
  return NetAddrTools::fromNetAddr(ethInfo.staticSubnetMask());
}

QString DeviceInfoForm::staticGateway(Word portID) const {
  const auto& ethInfo = device->get<EthernetPortInfo>(portID);
  return NetAddrTools::fromNetAddr(ethInfo.staticGateway());
}

QString DeviceInfoForm::currentIPAddress(Word portID) const {
  const auto& ethInfo = device->get<EthernetPortInfo>(portID);
  return NetAddrTools::fromNetAddr(ethInfo.currentIPAddress());
}

QString DeviceInfoForm::currentSubnetMask(Word portID) const {
  const auto& ethInfo = device->get<EthernetPortInfo>(portID);
  return NetAddrTools::fromNetAddr(ethInfo.currentSubnetMask());
}

QString DeviceInfoForm::currentGateway(Word portID) const {
  const auto& ethInfo = device->get<EthernetPortInfo>(portID);
  return NetAddrTools::fromNetAddr(ethInfo.currentGateway());
}

QString DeviceInfoForm::macAddress(Word portID) const {
  const auto& ethInfo = device->get<EthernetPortInfo>(portID);
  return QString::fromStdString(ethInfo.macAddress());
}

QString DeviceInfoForm::getDeviceNetName(Word portID) const {
  const auto& ethInfo = device->get<EthernetPortInfo>(portID);
  return QString::fromStdString(ethInfo.deviceName());
}

////////////////////////////////////////////////////////////////////////////////
/// setter helpers
////////////////////////////////////////////////////////////////////////////////

void DeviceInfoForm::setInfoString(InfoIDEnum infoID, QString value) {
  auto& info = device->infoData(infoID);
  info.infoString(value.toStdString());
  device->send<SetInfoCommand>(info);
}

void DeviceInfoForm::ipMode(Word portID, QComboBox*, int value) {
  auto& ethInfo = device->get<EthernetPortInfo>(portID);
  ethInfo.ipMode(static_cast<IPModeEnum>(value));
  device->send<SetEthernetPortInfoCommand>(ethInfo);
}

void DeviceInfoForm::staticIPAddress(Word portID, QString value) {
  auto& ethInfo = device->get<EthernetPortInfo>(portID);
  auto addr = NetAddrTools::toNetAddr(value);
  if (!((addr[0] == 0) && (addr[1] == 0) && (addr[2] == 0) && (addr[3] == 0))) {
    ethInfo.staticIPAddress(addr);
    device->send<SetEthernetPortInfoCommand>(ethInfo);
  }
}

void DeviceInfoForm::staticSubnetMask(Word portID, QString value) {
  auto& ethInfo = device->get<EthernetPortInfo>(portID);
  auto addr = NetAddrTools::toNetAddr(value);
  if (!((addr[0] == 0) && (addr[1] == 0) && (addr[2] == 0) && (addr[3] == 0))) {
    ethInfo.staticSubnetMask(addr);
    device->send<SetEthernetPortInfoCommand>(ethInfo);
  }
}

void DeviceInfoForm::staticGateway(Word portID, QString value) {
  auto& ethInfo = device->get<EthernetPortInfo>(portID);
  auto addr = NetAddrTools::toNetAddr(value);
  if (!((addr[0] == 0) && (addr[1] == 0) && (addr[2] == 0) && (addr[3] == 0))) {
    ethInfo.staticGateway(addr);
    device->send<SetEthernetPortInfoCommand>(ethInfo);
  }
}

QString DeviceInfoForm::infoStringTitle(Byte infoID) {
  InfoIDEnum infoIDEnum = (InfoIDEnum) infoID;
  QString result = tr("Unknown Information");

  switch (infoIDEnum) {
    case InfoID::AccessoryName:
      result = tr("Accessory Name");
      break;

    case InfoID::ManufacturerName:
      result = tr("Manufacturer Name");
      break;

    case InfoID::ModelNumber:
      result = tr("Model Number");
      break;

    case InfoID::SerialNumber:
      result = tr("Serial Number");
      break;

    case InfoID::FirmwareVersion:
      result = tr("Firmware Version");
      break;

    case InfoID::HardwareVersion:
      result = tr("Hardware Version");
      break;

    case InfoID::MACAddress1:
      result = tr("MAC address 1");
      break;

    case InfoID::MACAddress2:
      result = tr("MAC address 2");
      break;

    case InfoID::DeviceName:
      result = tr("Device Name");
      break;

    case InfoID::Unknown:
      // do nothing
      break;
  }

  return result;
}
