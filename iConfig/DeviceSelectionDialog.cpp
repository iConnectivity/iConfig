/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "DeviceSelectionDialog.h"
#include "ui_DeviceSelectionDialog.h"

#include "MIDIInfo.h"
#include "CommandList.h"
#include "Device.h"
#include "EthernetPortInfo.h"
#include "InfoList.h"
#include "MyAlgorithms.h"
#include "ResetList.h"
#include "SaveRestoreList.h"
#include "AudioControlDetail.h"

#include <QMessageBox>
#include <QMutex>
#include <QWaitCondition>

#include <utility>

#ifndef Q_MOC_RUN
#include <boost/bind.hpp>
#include <boost/range.hpp>
#include <boost/shared_ptr.hpp>
#endif

#include <QTimer>
#include <QWidget>
#include <QDialog>
#include <QString>
#include <QDebug>  //zx, 2017-06-20

using namespace GeneSysLib;
using namespace MyAlgorithms;
using namespace boost::range;
using namespace boost;
using namespace std;

/* NOTE: I am using the transID to determine which output port I sent the data
 * to. */
const int DeviceSelectionDialog::kMSecDiscoveryTimeout = 70;
const int DeviceSelectionDialog::kMSecGetInfoTimeout = 60;

QWaitCondition waitCondition;
QMutex mutexQ;

QMutex deviceMutex;

DeviceSelectionDialog::DeviceSelectionDialog(CommPtr _comm, QWidget *parent)
    : QDialog(parent),
      started(false),
      retDeviceHandlerID(-1),
      retCmdListHandlerID(-1),
      retInfoListHandlerID(-1),
      retInfoHandlerID(-1),
      ui(new Ui::DeviceSelectionDialog),
      comm(_comm) {
  ui->setupUi(this);

  discoveryTimer = QSharedPointer<QTimer>(new QTimer, &QTimer::deleteLater);
  getInfoTimer = QSharedPointer<QTimer>(new QTimer, &QTimer::deleteLater);

  connect(discoveryTimer.data(), SIGNAL(timeout()), this, SLOT(discoveryTick()),
          Qt::QueuedConnection);
  connect(getInfoTimer.data(), SIGNAL(timeout()), this, SLOT(getInfoTick()),
          Qt::QueuedConnection);
  connect(this, SIGNAL(deviceDiscovered(QString)), this,
          SLOT(addItemToList(QString)), Qt::QueuedConnection);
  connect(this, SIGNAL(enableButton(bool)),
          ui->buttonBox->button(QDialogButtonBox::Ok), SLOT(setEnabled(bool)),
          Qt::QueuedConnection);
  connect(this, SIGNAL(enableRefresh(bool)), ui->progressBar,
          SLOT(setHidden(bool)), Qt::QueuedConnection);
  connect(this, SIGNAL(enableRefresh(bool)), ui->refreshPushButton,
          SLOT(setEnabled(bool)), Qt::QueuedConnection);
  connect(this, SIGNAL(enableRefresh(bool)), ui->waitLabel,
          SLOT(setHidden(bool)));

  startSearch();

  Q_ASSERT(comm != nullptr);
}

DeviceSelectionDialog::~DeviceSelectionDialog() {
  stopDiscoveryTimer();
  stopGetInfoTimer();
}

DeviceInfoPtr DeviceSelectionDialog::getSelectedDevice() {
  DeviceInfoPtr selectedDevice;

  deviceMutex.lock();
  if ((ui->listWidget >= 0) &&
      (ui->listWidget->currentRow() < (int) devices.size())) {
    auto item = devices.begin();
    for (auto i = 0; i < ui->listWidget->currentRow(); ++i) {
      item++;
    }
    selectedDevice = *item;
  }
  deviceMutex.unlock();

  return selectedDevice;
}

void DeviceSelectionDialog::accept() {
  stopDiscoveryTimer();
  stopGetInfoTimer();
  if (ui->listWidget->count() <= 0) {
    QMessageBox msgBox;
    msgBox.setText("No device found.");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
    QDialog::reject();
  } else {
    QDialog::accept();
  }
}

void DeviceSelectionDialog::reject() {
  stopDiscoveryTimer();
  stopGetInfoTimer();
  QDialog::reject();
}

void DeviceSelectionDialog::startSearch() {
  if (!started) {
    deviceMutex.lock();
    devices.clear();
    deviceMutex.unlock();
    ui->listWidget->clear();
    bool opened = false;
    emit enableButton(false);
    emit enableRefresh(false);
    try {
      opened = comm->openAllInputs();
      if (opened) {
        opened = comm->openAllOutputs();
      }

      if (opened) {
        auto addCallback = [this](CmdEnum command, Handler handler) {
          this->registerHandleredCallbackMap[command] =
              comm->registerHandler(command, handler);
        }
        ;

        Handler bindedDiscoveredDeviceCallback =
            boost::bind(&DeviceSelectionDialog::discoveredDeviceCallback, this,
                        _1, _2, _3, _4);
        addCallback(Command::RetDevice, bindedDiscoveredDeviceCallback);

        Handler bindedCommandListCallback = boost::bind(
            &DeviceSelectionDialog::commandListCallback, this, _1, _2, _3, _4);
        addCallback(Command::RetCommandList, bindedCommandListCallback);

        Handler bindedEthernetCallback = boost::bind(
            &DeviceSelectionDialog::midiInfoCallback, this, _1, _2, _3, _4);
        addCallback(Command::RetMIDIInfo, bindedEthernetCallback);

        Handler bindedCommonCallback = boost::bind(
            &DeviceSelectionDialog::commonCallback, this, _1, _2, _3, _4);
        addCallback(Command::RetResetList, bindedCommonCallback);
        addCallback(Command::RetSaveRestoreList, bindedCommonCallback);

        Handler bindedInfoListCallback = boost::bind(
            &DeviceSelectionDialog::infoListCallback, this, _1, _2, _3, _4);
        addCallback(Command::RetInfoList, bindedInfoListCallback);

        Handler bindedDeviceInfoCallback = boost::bind(
            &DeviceSelectionDialog::deviceInfoCallback, this, _1, _2, _3, _4);
        addCallback(Command::RetInfo, bindedDeviceInfoCallback);

        discoverAll();
        started = true;
        startDiscoveryTimer();
      }
    }
    catch (...) {
      started = false;
    }

    if (!opened) {
//bugfixing: Don't shutdown application which make user feel the software being
//crashed.
//Make the device selection dialogbox opened so that user can refresh device to
//connect again.
//--zx, 2016-06-08
/*
      QMessageBox::critical(
          this, tr("Communication Error"),
          tr("An unexpected error occured while trying to open a MIDI port."
             " Please restart the application"));
      QCoreApplication::quit();
*/
        QMessageBox::critical(
            this, tr("Communication Error"),
            tr("An unexpected error occured while trying to open a MIDI port."
               " Please refresh to connect device again"));
        stopSearch();
    }
  }
}

void DeviceSelectionDialog::stopSearch() {
  if (started) {
    comm->closeAll();
  }
  stopDiscoveryTimer();
  stopGetInfoTimer();
  for (const auto &callbackMap : registerHandleredCallbackMap) {
    try {
      comm->unRegisterHandler(callbackMap.first, callbackMap.second);
    }
    catch (...) {
    }
  }
  registerHandleredCallbackMap.clear();
  started = false;

  retDeviceHandlerID=-1;
  retCmdListHandlerID=-1;
  retInfoListHandlerID =-1;
  retInfoHandlerID =-1;

  emit enableRefresh(true);
}

void DeviceSelectionDialog::startDiscoveryTimer() {
  discoveryTimer->start(kMSecDiscoveryTimeout);
}

void DeviceSelectionDialog::stopDiscoveryTimer() { discoveryTimer->stop(); }

void DeviceSelectionDialog::startGetInfoTimer() {
  getInfoTimer->start(kMSecGetInfoTimeout);
}

void DeviceSelectionDialog::stopGetInfoTimer() { getInfoTimer->stop(); }

void DeviceSelectionDialog::discoverAll() {
  mutexQ.lock();
  int nOuputPorts = comm->getOutCount();

  //zx, 2017-06-20
  qDebug() << "Communicator Ouput count: " << nOuputPorts;

  for (Word i = 0; i < comm->getOutCount(); ++i) {
    pendingDiscoverySysex.push(
        make_pair(i, sysex(GetDeviceCommand(DeviceID(), i))));
  }
  mutexQ.unlock();
  sendNextDiscovery();
}

void DeviceSelectionDialog::discoveryTick() {
  mutexQ.lock();
  bool noPendingDiscovery = pendingDiscoverySysex.empty();
  mutexQ.unlock();

  if (noPendingDiscovery) {
    stopDiscoveryTimer();
    startGetInfoTimer();
    sendNextGetInfo();
  } else {
    sendNextDiscovery();
  }
}

void DeviceSelectionDialog::getInfoTick() {
  mutexQ.lock();
  bool noPendingInfo = pendingGetInfoSysex.empty();
  mutexQ.unlock();

  if (noPendingInfo) {
    stopSearch();

    for (auto dev : devices) {
      emitDevice(dev);
    }
    deviceMutex.unlock();
  } else {
    sendNextGetInfo();
  }
}

void DeviceSelectionDialog::addItemToList(QString item) {
  ui->listWidget->addItem(item);

  if (ui->listWidget->count() == 1) {
    ui->listWidget->setCurrentRow(0);
  }
}

void DeviceSelectionDialog::discoveredDeviceCallback(
    CmdEnum, DeviceID deviceID, Word transID, commandData_t commandData) {
  auto foundDevice = findDevice(deviceID);

  if (!foundDevice) {
    auto dev = DeviceInfoPtr(new DeviceInfo(comm, deviceID, transID));
    dev->addCommandData(commandData);
    deviceMutex.lock();
    devices.push_back(dev);
    deviceMutex.unlock();

// For Windows USB issue >>>>>
#ifdef _WIN32
    const auto &devInfo = commandData.get<Device>();
    if (!dev->isNewWindowsDriver()) {
        if (devInfo.mode() == BootMode::AppMode) {
          mutexQ.lock();
          for (int i = 0; i < 275; ++i) {
            pendingDiscoverySysex.push(
                make_pair(transID, sysex(GetDeviceCommand(DeviceID(), 0x00FF))));
          }
          mutexQ.unlock();
        }
    }
#endif  // WIN32
// For Windows USB issue <<<<<

    mutexQ.lock();
    pendingGetInfoSysex.push(
        make_pair(transID, sysex(GetCommandListCommand(deviceID, transID))));
    mutexQ.unlock();
  }
}

void DeviceSelectionDialog::commandListCallback(CmdEnum, DeviceID deviceID,
                                                Word transID,
                                                commandData_t commandData) {
  auto foundDevice = findDevice(deviceID);

  //printf("1: %2x, %2X\n", deviceID.pid(), transID);
  if (foundDevice) {
    foundDevice->addCommandData(commandData);

    //printf("2: %2x, %2X\n", deviceID.pid(), transID);

    auto &commandListData = commandData.get<CommandList>();
    if (commandListData.contains(Command::GetMIDIInfo)) {
      mutexQ.lock();
      pendingGetInfoSysex.push(
          make_pair(transID, sysex(GetMIDIInfoCommand(deviceID, transID))));
      mutexQ.unlock();
    }
    if (commandListData.contains(Command::GetInfoList)) {
      mutexQ.lock();
      pendingGetInfoSysex.push(
          make_pair(transID, sysex(GetInfoListCommand(deviceID, transID))));
      mutexQ.unlock();
    }
    if (commandListData.contains(Command::GetResetList)) {
      mutexQ.lock();
      pendingGetInfoSysex.push(
          make_pair(transID, sysex(GetResetListCommand(deviceID, transID))));
      mutexQ.unlock();
    }
    if (commandListData.contains(Command::GetSaveRestoreList)) {
      mutexQ.lock();
      pendingGetInfoSysex.push(make_pair(
          transID, sysex(GetSaveRestoreListCommand(deviceID, transID))));
      mutexQ.unlock();
    }
    if (commandListData.contains(Command::GetAudioControlDetail)) {

    }
  }

  sendNextGetInfo();
}

void DeviceSelectionDialog::commonCallback(CmdEnum, DeviceID deviceID, Word,
                                           commandData_t commandData) {
  auto foundDevice = findDevice(deviceID);

  if (foundDevice) {
    foundDevice->addCommandData(commandData);
  }

  sendNextGetInfo();
}

void DeviceSelectionDialog::midiInfoCallback(CmdEnum, DeviceID deviceID,
                                             Word transID,
                                             commandData_t commandData) {
  auto foundDevice = findDevice(deviceID);

  if (foundDevice) {
    foundDevice->addCommandData(commandData);
  }

  auto &midiInfo = commandData.get<MIDIInfo>();
  if (midiInfo.numEthernetJacks() > 0) {
    mutexQ.lock();
    for (int jackID = 1; jackID <= midiInfo.numEthernetJacks(); ++jackID) {
      pendingGetInfoSysex.push(make_pair(
          transID,
          sysex(GetEthernetPortInfoCommand(deviceID, transID, jackID))));
    }
    mutexQ.unlock();
  }

  sendNextGetInfo();
}

void DeviceSelectionDialog::infoListCallback(CmdEnum, DeviceID deviceID,
                                             Word transID,
                                             commandData_t commandData) {
  auto foundDevice = findDevice(deviceID);

  if (foundDevice) {
    foundDevice->addCommandData(commandData);
  }

  const auto &infoListData = commandData.get<InfoList>();
  const auto &containsDeviceName = infoListData.contains(InfoID::DeviceName);
  const auto &containsAccessoryName =
      infoListData.contains(InfoID::AccessoryName);

  if (containsDeviceName) {
    mutexQ.lock();
    pendingGetInfoSysex.push(make_pair(
        transID, sysex(GetInfoCommand(deviceID, transID, InfoID::DeviceName))));
    mutexQ.unlock();
  }

  if (containsAccessoryName) {
    mutexQ.lock();
    pendingGetInfoSysex.push(make_pair(
        transID,
        sysex(GetInfoCommand(deviceID, transID, InfoID::AccessoryName))));
    mutexQ.unlock();
  }

  sendNextGetInfo();
}

void DeviceSelectionDialog::deviceInfoCallback(CmdEnum, DeviceID deviceID, Word,
                                               commandData_t commandData) {
  auto foundDevice = findDevice(deviceID);

  if (foundDevice) {
    foundDevice->addCommandData(commandData);
  }
  sendNextGetInfo();
}

void DeviceSelectionDialog::on_refreshPushButton_clicked() {
  stopSearch();
  startSearch();
}

void DeviceSelectionDialog::emitSerialNumber(DeviceID deviceID) {
  uint32_t snum = deviceID.serialNumber()[0];
  snum = (snum << 7) | deviceID.serialNumber()[1];
  snum = (snum << 7) | deviceID.serialNumber()[2];
  snum = (snum << 7) | deviceID.serialNumber()[3];
  snum = (snum << 7) | deviceID.serialNumber()[4];

  emit deviceDiscovered(QString("%1 (%2)")
                            .arg(QString::number(deviceID.pid()),
                                 QString("%1").arg(snum, 8, 16, QChar('0'))));
  emit enableButton(true);
}

void DeviceSelectionDialog::connectToOne() {
  //printf("connecting to one!\n");
  if (ui->listWidget->count() == 1) {
    QDialog::accept();
  }
}

void DeviceSelectionDialog::emitRecoveryDevice(Word pid) {
  QString deviceType = QString::number(pid);

  switch ((DevicePID::Enum) pid) {
    case DevicePID::MIO:
      deviceType = tr("mio");
      break;

    case DevicePID::MIO2:
      deviceType = tr("mio2");
      break;

  case DevicePID::MIO4:
    deviceType = tr("mio4");
    break;

  case DevicePID::MIO10:
    deviceType = tr("mio10");
    break;

  case DevicePID::PlayAudio12:
    deviceType = tr("PlayAUDIO12");
    break;

  case DevicePID::ConnectAudio24:
    deviceType = tr("ConnectAUDIO2/4");
    break;

    case DevicePID::iConnect1:
      deviceType = tr("iConnectMIDI1");
      break;

    case DevicePID::iConnect2Plus:
      deviceType = tr("iConnectMIDI2+");
      break;

    case DevicePID::iConnect4Plus:
      deviceType = tr("iConnectMIDI4+");
      break;
    case DevicePID::iConnect4Audio:
      deviceType = tr("iConnectAUDIO4+");
      break;
  case DevicePID::iConnect2Audio:
    deviceType = tr("iConnectAUDIO2+");
    break;  }

  emit deviceDiscovered(QString("%1 [Bootloader Mode]").arg(deviceType));
  emit enableButton(true);
}

void DeviceSelectionDialog::sendNextDiscovery() {
  mutexQ.lock();
  bool pendingMessage = (!pendingDiscoverySysex.empty());
  mutexQ.unlock();

  if (pendingMessage) {
    mutexQ.lock();
    auto nextPair = pendingDiscoverySysex.front();
    pendingDiscoverySysex.pop();
    mutexQ.unlock();
    comm->setCurrentOutput(nextPair.first);
    comm->sendSysex(nextPair.second);
  }
}

void DeviceSelectionDialog::sendNextGetInfo() {
  mutexQ.lock();
  bool pendingMessage = (!pendingGetInfoSysex.empty());
  mutexQ.unlock();

  if (pendingMessage) {
    mutexQ.lock();
    auto nextPair = pendingGetInfoSysex.front();
    pendingGetInfoSysex.pop();
    mutexQ.unlock();

    comm->setCurrentOutput(nextPair.first);
    comm->sendSysex(nextPair.second);
  }
}

DeviceInfoPtr DeviceSelectionDialog::findDevice(DeviceID deviceID) {
  DeviceInfoPtr result;
  deviceMutex.lock();

  auto foundDevice = find_if(devices.begin(), devices.end(),
                             [ = ](const DeviceInfoPtr & device)->bool {
    return deviceID == device->getDeviceID();
  });

  if (foundDevice != devices.end()) {
    result = (*foundDevice);
  }

  deviceMutex.unlock();

  return result;
}

void DeviceSelectionDialog::emitDevice(DeviceInfoPtr &device) {
  BootModeEnum bootMode = BootMode::AppMode;
  DeviceID deviceID;
  Word transID;

  tie(deviceID, transID) = device->getInfo();

  auto &devInfo = device->get<Device>();
  bootMode = (BootModeEnum)(devInfo.mode());

  uint32_t snum = deviceID.serialNumber()[0];
  snum = (snum << 7) | deviceID.serialNumber()[1];
  snum = (snum << 7) | deviceID.serialNumber()[2];
  snum = (snum << 7) | deviceID.serialNumber()[3];
  snum = (snum << 7) | deviceID.serialNumber()[4];

  if (device->containsInfo(InfoID::DeviceName)) {
    const auto &infoData = device->infoData(InfoID::DeviceName);
    emit deviceDiscovered(
        QString("%1").arg(QString::fromStdString(infoData.infoString())));

    cout << "Device discovered: " << infoData.infoString() << endl;

    emit enableButton(true);

    QTimer::singleShot(200, this, SLOT(connectToOne()));
    //printf("timer began\n");
  } else if (device->containsInfo(InfoID::AccessoryName)) {
    const auto &infoData = device->infoData(InfoID::AccessoryName);
    emit deviceDiscovered(
        QString("%1 (%2)").arg(QString::fromStdString(infoData.infoString()),
                               QString("%1").arg(snum, 8, 16, QChar('0'))));
  } else {
    if (bootMode == BootMode::AppMode) {
      emitSerialNumber(deviceID);
    } else if (bootMode == BootMode::BootLoaderMode) {
      emitRecoveryDevice(deviceID.pid());
    }
  }
  emit enableButton(true);
}

//Re-factory device selection UI functions, zx, 2017-04-06
DeviceInfoPtr DeviceSelectionDialog::GetPreviousOpenDevice() {
  DeviceInfoPtr selectedDevice;

  if (0 < (int) devices.size()) {
    auto item = devices.begin();
    for (auto i = 0; i < (int) devices.size(); ++i) {
      if((*item)->getDeviceID() == m_CachedPreviousDviceID) {
        return (*item);
      }
      item++;
    }
  }

  return selectedDevice;
}

void DeviceSelectionDialog::SetPreviousCachedDeviceID(GeneSysLib::DeviceID deviceID) {
  m_CachedPreviousDviceID = deviceID;

}

