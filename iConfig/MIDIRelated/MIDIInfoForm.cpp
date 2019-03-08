/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "MIDIInfoForm.h"
#include "ui_MIDIInfoForm.h"

#include "ACK.h"
#include "CenteredContainer.h"
#include "CommandList.h"
#include "DeviceInfo.h"
#include "MIDIInfo.h"
#include "MIDIPortDetail.h"
#include "MyAlgorithms.h"
#include "MyCheckBox.h"
#include "MyComboBox.h"
#include "MyConverters.h"
#include "MyLabel.h"
#include "MyLineEdit.h"
#include "TreeUtils.h"
#include "RTPMIDIConnectionDetail.h"

#include <algorithm>

#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QLabel>
#include <QLayout>
#include <QTableWidget>

#ifndef Q_MOC_RUN
#include <boost/bind.hpp>
#include <boost/range.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/irange.hpp>
#include <boost/shared_ptr.hpp>
#endif

using namespace CenteredContainer;
using namespace GeneSysLib;
using namespace MyAlgorithms;
using namespace TreeUtils;
using namespace boost::adaptors;
using namespace boost::range;
using namespace boost;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
/// DIN table columns
////////////////////////////////////////////////////////////////////////////////
namespace DINColumns {
typedef enum ENUM {
  Jack,
  PortName,
  Input,
  Output
} ENUM;
}  // namespace DINColumns

////////////////////////////////////////////////////////////////////////////////
/// USB device columns
////////////////////////////////////////////////////////////////////////////////
namespace USBDeviceColumns {
typedef enum ENUM {
  Port,
  PortName,
  Input,
  Output
} ENUM;
}  // namespace USBDeviceColumns

////////////////////////////////////////////////////////////////////////////////
/// USB host columns
////////////////////////////////////////////////////////////////////////////////
namespace USBHostColumns {
typedef enum ENUM {
  Port,
  ConnectedDevice,
  Reserved,
  PortName,
  Input,
  Output
} ENUM;
}  // namespace USBHostColumns

////////////////////////////////////////////////////////////////////////////////
/// Ethernet columns
////////////////////////////////////////////////////////////////////////////////
namespace EthernetColumns {
typedef enum ENUM {
  Session,
  RTPMIDIPort,
  ConnIP,
  ConnPort,
  ConnName,
  Name,
  Input,
  Output
} ENUM;
}  // namespace Ethernet Columns

////////////////////////////////////////////////////////////////////////////////
/// static defines
////////////////////////////////////////////////////////////////////////////////
const QString MIDIInfoForm::DefaultWindowTitle = tr("MIDI Info");

////////////////////////////////////////////////////////////////////////////////
/// constructor
////////////////////////////////////////////////////////////////////////////////
MIDIInfoForm::MIDIInfoForm(CommPtr _comm, DeviceInfoPtr _device,
                           QWidget* _parent)
    : RefreshObject(_parent),
      ui(new Ui::MIDIInfoForm),
      comm(_comm),
      device(_device) {
  ui->setupUi(this);

  this->setWindowTitle(DefaultWindowTitle);
  treeBuilt = false;

  Q_ASSERT(comm);
  Q_ASSERT(device);

  const auto& handler = bind(&MIDIInfoForm::ackCallback, this, _1, _2, _3, _4);
  this->registeredHandlerIDs[Command::ACK] =
      comm->registerHandler(Command::ACK, handler);

  connect(this, SIGNAL(informationAvailable()), this, SLOT(buildInformation()),
          Qt::QueuedConnection);

  connect(device.get(), SIGNAL(queryCompleted(Screen, CommandQList)), this,
          SLOT(queryCompleted(Screen, CommandQList)), Qt::QueuedConnection);

  const auto& commandList = device->get<CommandList>();

  CommandQList query;
  query << Command::RetMIDIInfo;
  query << Command::RetMIDIPortInfo;

  if (commandList.contains(Command::GetMIDIPortDetail)) {
    query << Command::RetMIDIPortDetail;
  }
  if (commandList.contains(Command::GetUSBHostMIDIDeviceDetail)) {
    query << Command::RetUSBHostMIDIDeviceDetail;
  }
  if (commandList.contains(Command::GetRTPMIDIConnectionDetail)) {
    query << Command::RetRTPMIDIConnectionDetail;
  }
  if (commandList.contains(Command::GetEthernetPortInfo)) {
    query << Command::RetEthernetPortInfo;
  }

  emit updateMessage(tr("Reading MIDI Information"), kMessageTimeout);
  device->startQuery(MIDIInformationScreen, query);
  QList<int> sizes;
  sizes << 120 << 300;
  ui->splitter->setSizes(sizes);
}

////////////////////////////////////////////////////////////////////////////////
/// destructor
////////////////////////////////////////////////////////////////////////////////
MIDIInfoForm::~MIDIInfoForm() {
  for (auto& handlerIDPair : registeredHandlerIDs) {
    comm->unRegisterHandler(handlerIDPair.first, handlerIDPair.second);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// builds the information
////////////////////////////////////////////////////////////////////////////////
void MIDIInfoForm::buildInformation() {
  treeBuilt = true;

  // MIDI information
  const auto& midiInfo = device->get<MIDIInfo>();

  if (midiInfo.versionNumber() == 0x01) {
    addGeneralMIDIInformation(ui->treeWidget, midiInfo);

    // create the trees
    createDINTable(midiInfo);
    createUSBDeviceTables(midiInfo);
    createUSBHostTables(midiInfo);
    createEthernetTables(midiInfo);
    auto* const spacer =
        new QSpacerItem(kDefaultHeaderSize, 0, QSizePolicy::MinimumExpanding,
                        QSizePolicy::Expanding);
    ui->verticalTablesLayout->addItem(spacer);
  }

  ui->treeWidget->expandAll();
  ui->treeWidget->resizeColumnToContents(0);
}

////////////////////////////////////////////////////////////////////////////////
/// create DIN table
////////////////////////////////////////////////////////////////////////////////
void MIDIInfoForm::createDINTable(const MIDIInfo& midiInfo) {
  if (midiInfo.numDINPairs() > 0) {
    ui->verticalTablesLayout->addWidget(new QLabel(tr("DIN Jacks")));

    QStringList labels;
    labels << tr("Jack") << tr("Name") << tr("Input") << tr("Output");

    auto* const tableWidget =
        createInfoTableWidget(labels, midiInfo.numDINPairs());
    ui->verticalTablesLayout->addWidget(tableWidget);

    QHeaderView* horzHdr = tableWidget->horizontalHeader();
    horzHdr->setResizeMode(QHeaderView::Fixed);
    horzHdr->setDefaultSectionSize(kDefaultColumnSize);
    horzHdr->setResizeMode(DINColumns::PortName, QHeaderView::Stretch);

    const auto& b = dinStartPortID();
    const auto& e = dinEndPortID(midiInfo);
    for (Word portID = b; portID < e; ++portID) {
      addDINRow(tableWidget, portID - b, portID);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
/// add DIN row to DIN table
////////////////////////////////////////////////////////////////////////////////
void MIDIInfoForm::addDINRow(QTableWidget* table, int row, Word portID) {
  // set the row height
  table->setRowHeight(row, kDefaultRowSize);

  // jack number
  addLabel(table, row, DINColumns::Jack,
           bind(&MIDIInfoForm::getJackNumber, this, portID));
  // port name
  addPortName(table, portID, row, DINColumns::PortName);
  // input enabled
  addInputEnabledCheckBox(table, portID, row, DINColumns::Input);
  // output enabled checkbox
  addOutputEnabledCheckBox(table, portID, row, DINColumns::Output);
}

////////////////////////////////////////////////////////////////////////////////
/// creates all USB device tables
////////////////////////////////////////////////////////////////////////////////
void MIDIInfoForm::createUSBDeviceTables(const MIDIInfo& midiInfo) {
  for (Word jack = 1; jack <= midiInfo.numUSBDeviceJacks(); ++jack) {
    createUSBDeviceTable(midiInfo, jack);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// creates a single usb device table
////////////////////////////////////////////////////////////////////////////////
void MIDIInfoForm::createUSBDeviceTable(const MIDIInfo& midiInfo, Word jack) {
  Q_ASSERT(jack >= 1 && jack <= midiInfo.numUSBDeviceJacks());
  Q_ASSERT(midiInfo.numUSBMIDIPortPerDeviceJack() > 0);

  const auto& tableName = [ = ] {
    return tr("USB Device Jack %1 (Type:%2, Name:%3)")
        .arg(QString::number(jack)).arg(getUSBDeviceHostType(
            jack * midiInfo.numUSBMIDIPortPerDeviceJack()))
        .arg(getUSBDeviceHostName(jack *
                                  midiInfo.numUSBMIDIPortPerDeviceJack()));
  }
  ;
  ui->verticalTablesLayout->addWidget(new MyLabel(tableName));

  QStringList labels;
  labels << tr("Port") << tr("Name") << tr("Input") << tr("Output");
  auto* const tableWidget =
      createInfoTableWidget(labels, midiInfo.numUSBMIDIPortPerDeviceJack());
  ui->verticalTablesLayout->addWidget(tableWidget);

  QHeaderView* horzHdr = tableWidget->horizontalHeader();
  horzHdr->setDefaultSectionSize(kDefaultColumnSize);
  horzHdr->setResizeMode(QHeaderView::Fixed);
  horzHdr->setResizeMode(USBDeviceColumns::PortName, QHeaderView::Stretch);

  const auto& b = usbDeviceStartPortID(midiInfo, jack);
  const auto& e = usbDeviceEndPortID(midiInfo, jack);
  int row = 0;
  for (Word portID = b; portID < e; ++portID, ++row) {
    addUSBDeviceRow(tableWidget, row, portID);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// adds a usb device row to the usb device table
////////////////////////////////////////////////////////////////////////////////
void MIDIInfoForm::addUSBDeviceRow(QTableWidget* table, int row, Word portID) {
  table->setRowHeight(row, kDefaultRowSize);

  // port jack
  addPortJack(table, portID, row, USBDeviceColumns::Port);
  // port name
  addPortName(table, portID, row, USBDeviceColumns::PortName);
  // input enabled
  addInputEnabledCheckBox(table, portID, row, USBDeviceColumns::Input);
  // output enabled
  addOutputEnabledCheckBox(table, portID, row, USBDeviceColumns::Output);
}

////////////////////////////////////////////////////////////////////////////////
/// creates all usb host tables
////////////////////////////////////////////////////////////////////////////////
void MIDIInfoForm::createUSBHostTables(const MIDIInfo& midiInfo) {
  for (Word jack = 1; jack <= midiInfo.numUSBHostJacks(); ++jack) {
    createUSBHostTable(midiInfo, jack);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// creates single usb host table
////////////////////////////////////////////////////////////////////////////////
void MIDIInfoForm::createUSBHostTable(const MIDIInfo& midiInfo, Word jack) {
  Q_ASSERT(jack >= 1 && jack <= midiInfo.numUSBHostJacks());

  const auto& tableName = [ = ] {
    return tr("USB Host Jack %1").arg(QString::number(jack));
  }
  ;

  ui->verticalTablesLayout->addWidget(new MyLabel(tableName));

  QStringList labels;
  labels << tr("Port") << tr("Connected\nDevice") << tr("Reserved")
         << tr("Name") << tr("Input") << tr("Output");
  auto rowCount =
      (midiInfo.numUSBHostJacks() * midiInfo.numUSBMIDIPortPerHostJack());
  auto* tableWidget = createInfoTableWidget(labels, rowCount);
  ui->verticalTablesLayout->addWidget(tableWidget);

  QHeaderView* horzHdr = tableWidget->horizontalHeader();
  horzHdr->setResizeMode(QHeaderView::Fixed);
  horzHdr->setDefaultSectionSize(kDefaultColumnSize);
  horzHdr->setResizeMode(USBHostColumns::PortName, QHeaderView::Stretch);
  horzHdr->setResizeMode(USBHostColumns::ConnectedDevice, QHeaderView::Stretch);
  horzHdr->setResizeMode(USBHostColumns::Reserved, QHeaderView::Stretch);

  const auto& b = usbHostStartPortID(midiInfo, jack);
  const auto& e = usbHostEndPortID(midiInfo, jack);
  int row = 0;
  for (Word portID = b; portID < e; ++portID, ++row) {
    addUSBHostRow(tableWidget, row, portID);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// adds a row to passed in usb host table
////////////////////////////////////////////////////////////////////////////////
void MIDIInfoForm::addUSBHostRow(QTableWidget* table, int row, Word portID) {
  table->setRowHeight(row, kDefaultRowSize);
  // port name
  addPortName(table, portID, row, USBHostColumns::PortName);
  // port specific
  addLabel(table, row, USBHostColumns::Port,
           bind(&MIDIInfoForm::getUSBHostPort, this, portID));
  // input enabled
  addInputEnabledCheckBox(table, portID, row, USBHostColumns::Input);
  // output enabled
  addOutputEnabledCheckBox(table, portID, row, USBHostColumns::Output);
  // connected device
  addLabel(table, row, USBHostColumns::ConnectedDevice,
           bind(&MIDIInfoForm::getUSBHostConnectedDevice, this, portID));
  auto* const reservedComboBox = addComboBox(
      table, row, USBHostColumns::Reserved,
      bind(&MIDIInfoForm::getUSBHostOptions, this, portID),
      bind(&MIDIInfoForm::getSelectedUSBHostForPort, this, portID, _1),
      bind(&MIDIInfoForm::setSelectedUSBHostForPort, this, portID, _1, _2));
  reservedComboBoxes.push_back(reservedComboBox);
}

////////////////////////////////////////////////////////////////////////////////
/// creates all the ethernet tables
////////////////////////////////////////////////////////////////////////////////
void MIDIInfoForm::createEthernetTables(const MIDIInfo& midiInfo) {
  for (Word jack = 1; jack <= midiInfo.numEthernetJacks(); ++jack) {
    createEthernetTable(midiInfo, jack);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// creates a single ethernet information table
////////////////////////////////////////////////////////////////////////////////
void MIDIInfoForm::createEthernetTable(const MIDIInfo& midiInfo, Word jack) {
  Q_ASSERT(jack >= 1 && jack <= midiInfo.numEthernetJacks());
  const auto& title = [ = ] {
    return tr("Ethernet Jack %1").arg(QString::number(jack));
  }
  ;
  ui->verticalTablesLayout->addWidget(new MyLabel(title));

  QStringList labels;
  labels << tr("Session") << tr("RTP MIDI\nPort") << tr("Connected IP")
         << tr("Connected\nPort") << tr("Connected\nName") << tr("Name")
         << tr("Input") << tr("Output");
  auto rowCount = (midiInfo.numEthernetJacks() *
                   midiInfo.numRTPMIDISessionsPerEthernetJack());
  auto* tableWidget = createInfoTableWidget(labels, rowCount);
  ui->verticalTablesLayout->addWidget(tableWidget);

  auto* horzHdr = tableWidget->horizontalHeader();
  horzHdr->setResizeMode(QHeaderView::ResizeToContents);
  horzHdr->setDefaultSectionSize(kDefaultColumnSize);
  horzHdr->setResizeMode(EthernetColumns::Name, QHeaderView::Stretch);
  horzHdr->setResizeMode(EthernetColumns::Session, QHeaderView::Stretch);
  horzHdr->setResizeMode(EthernetColumns::ConnName, QHeaderView::Stretch);
  horzHdr->setResizeMode(EthernetColumns::Input, QHeaderView::Fixed);
  horzHdr->setResizeMode(EthernetColumns::Output, QHeaderView::Fixed);

  int row = 0;
  const auto& b = ethernetStartPortID(midiInfo);
  const auto& e = ethernetEndPortID(midiInfo);
  for (Word portID = b; portID < e; ++portID, ++row) {
    addEthernetRow(tableWidget, row, portID);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// adds a row to the passed in ethernet table
////////////////////////////////////////////////////////////////////////////////
void MIDIInfoForm::addEthernetRow(QTableWidget* table, int row, Word portID) {
  table->setRowHeight(row, kDefaultRowSize);

  // port name
  addPortName(table, portID, row, EthernetColumns::Name);
  // session
  addLabel(table, row, EthernetColumns::Session,
           bind(&MIDIInfoForm::getSessionNameForPort, this, portID));
  // input enabled
  addInputEnabledCheckBox(table, portID, row, EthernetColumns::Input);
  // output enabled
  addOutputEnabledCheckBox(table, portID, row, EthernetColumns::Output);
  // RTP MIDI port
  addLabel(table, row, EthernetColumns::RTPMIDIPort,
           bind(&MIDIInfoForm::getRTPIPPortForPort, this, portID));

  auto& rtpConnData = device->get<RTPMIDIConnectionDetail>(portID);

  if (rtpConnData.versionNumber() == 0x01) {
    // IP
    addLabel(table, row, EthernetColumns::ConnIP,
             bind(&MIDIInfoForm::getRTPConnIPForPort, this, portID));

    // port
    addLabel(table, row, EthernetColumns::ConnPort,
             bind(&MIDIInfoForm::getRTPMIDIPortForPort, this, portID));

    // connected session name
    addLabel(table, row, EthernetColumns::ConnName,
             bind(&MIDIInfoForm::getConnSessionNameForPort, this, portID));
  }
}

////////////////////////////////////////////////////////////////////////////////
/// adds a port jack label for passed in port ID to table at (row, col)
////////////////////////////////////////////////////////////////////////////////
void MIDIInfoForm::addPortJack(QTableWidget* tableWidget, Word portID, int row,
                               int col) {
  Q_ASSERT(tableWidget);
  const auto& getFunc = bind(&MIDIInfoForm::getUSBDevicePort, this, portID);
  addLabel(tableWidget, row, col, getFunc);
}

////////////////////////////////////////////////////////////////////////////////
/// adds a port name for a passed in port ID to table at (row, col)
////////////////////////////////////////////////////////////////////////////////
void MIDIInfoForm::addPortName(QTableWidget* tableWidget, Word portID, int row,
                               int col) {
  Q_ASSERT(tableWidget);
  addLineEdit(tableWidget, row, col, getMaxPortNameLength(portID),
              bind(&MIDIInfoForm::getPortName, this, portID),
              bind(&MIDIInfoForm::setPortName, this, portID, _1));
}

////////////////////////////////////////////////////////////////////////////////
/// adds input enabled checkbox for a passed in port ID to a table at (row, col)
////////////////////////////////////////////////////////////////////////////////
void MIDIInfoForm::addInputEnabledCheckBox(QTableWidget* tableWidget,
                                           Word portID, int row, int col) {
  const auto& getFunc = bind(&MIDIInfoForm::getPortInputEnabled, this, portID);
  const auto& setFunc =
      bind(&MIDIInfoForm::setPortInputEnabled, this, portID, _1);
  addCheckBox(tableWidget, row, col, getFunc, setFunc);
}

////////////////////////////////////////////////////////////////////////////////
/// adds output enabled checkbox for a passed in port ID to a table at (row,
/// col)
////////////////////////////////////////////////////////////////////////////////
void MIDIInfoForm::addOutputEnabledCheckBox(QTableWidget* tableWidget,
                                            Word portID, int row, int col) {
  const auto& getFunc = bind(&MIDIInfoForm::getPortOutputEnabled, this, portID);
  const auto& setFunc =
      bind(&MIDIInfoForm::setPortOutputEnabled, this, portID, _1);
  addCheckBox(tableWidget, row, col, getFunc, setFunc);
}

////////////////////////////////////////////////////////////////////////////////
/// adds an enabled checkbox with passed in getter and setter functions to
/// passed in table widget at (row, col)
////////////////////////////////////////////////////////////////////////////////
void MIDIInfoForm::addCheckBox(QTableWidget* tableWidget, int row, int col,
                               const GetBoolFunctor& getFunc,
                               const SetBoolFunctor& setFunc) {
  Q_ASSERT(tableWidget);

  auto* outputEnabledCheckBox = new MyCheckBox(getFunc, setFunc);
  auto* centeredWidget = centeredContainer(outputEnabledCheckBox);

  tableWidget->setCellWidget(row, col, centeredWidget);
  refreshList.push_back(outputEnabledCheckBox);
}

////////////////////////////////////////////////////////////////////////////////
/// adds a line edit with passed in getter and setter functions to passed in
/// table widget at (row, col)
////////////////////////////////////////////////////////////////////////////////
MyLineEdit* MIDIInfoForm::addLineEdit(QTableWidget* tableWidget, int row,
                                      int col, int maxLength,
                                      const GetQStringFunctor& getFunc,
                                      const SetQStringFunctor& setFunc) {
  auto* lineEdit = new MyLineEdit(getFunc, setFunc);
  lineEdit->setMaxLength(maxLength);
  tableWidget->setCellWidget(row, col, lineEdit);
  refreshList.push_back(lineEdit);
  return lineEdit;
}

////////////////////////////////////////////////////////////////////////////////
/// adds combo box with options, getter and setter to passed in table widget at
/// (row, col)
////////////////////////////////////////////////////////////////////////////////
MyComboBox* MIDIInfoForm::addComboBox(
    QTableWidget* tableWidget, int row, int col,
    const GetComboBoxOptions& options,
    const GetComboBoxSelectionFunctor& getFunc,
    const SetComboBoxSelectionFunctor& setFunc) {
  auto* comboBox = new MyComboBox(options, getFunc, setFunc);
  tableWidget->setCellWidget(row, col, comboBox);
  refreshList.push_back(comboBox);
  return comboBox;
}

////////////////////////////////////////////////////////////////////////////////
/// addes label with passed in getter function to passed in table widget at
/// (row, col)
////////////////////////////////////////////////////////////////////////////////
MyLabel* MIDIInfoForm::addLabel(QTableWidget* tableWidget, int row, int col,
                                const GetQStringFunctor& getFunc) {
  auto* label = new MyLabel(getFunc);
  label->setAlignment(Qt::AlignCenter);
  tableWidget->setCellWidget(row, col, label);
  refreshList.push_back(label);
  return label;
}

////////////////////////////////////////////////////////////////////////////////
/// a callback that gets called when the command query is completed
////////////////////////////////////////////////////////////////////////////////
void MIDIInfoForm::queryCompleted(Screen sc, CommandQList) {
  if (sc == MIDIInformationScreen)
    if (!treeBuilt) emit informationAvailable();
}

////////////////////////////////////////////////////////////////////////////////
/// this is a helper function to set the maximum number of multi port usb midi
/// ports on a single usb port
////////////////////////////////////////////////////////////////////////////////
void MIDIInfoForm::updateNumberOfMultiPortUSB(int value) {
  if (device->contains<MIDIInfo>()) {
    auto& midiInfo = device->get<MIDIInfo>();
    midiInfo.maxPortsOnMultiPortUSBDevice(value);

    device->send<SetMIDIInfoCommand>(midiInfo);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// this method gets called when the UI needs to refresh
////////////////////////////////////////////////////////////////////////////////
void MIDIInfoForm::refreshWidget() {
  for (auto* refresh : refreshList) {
    refresh->refreshData();
  }
}

////////////////////////////////////////////////////////////////////////////////
/// this callback is called when the system receives an ACK (used to update a
/// user message)
////////////////////////////////////////////////////////////////////////////////
void MIDIInfoForm::ackCallback(CmdEnum, DeviceID, Word,
                               commandData_t _commandData) {
  const auto& ackData = _commandData.get<ACK>();
  if (ackData.commandID() == Command::SetInfo) {  // update set info
    emit updateMessage(
        (ackData.errorCode() == 0x00)
            ? tr("Information successfully written.")
            : tr("Error writting information. Please try again."),
        kMessageTimeout);
    emit requestRefresh();
  } else if (ackData.commandID() == Command::SetMIDIInfo) {  // update MIDI info
    emit updateMessage(
        (ackData.errorCode() == 0x00)
            ? tr("Information successfully written.")
            : tr("Error writting information. Please try again."),
        kMessageTimeout);
    emit requestRefresh();
  } else if (ackData.commandID() ==
             Command::SetMIDIPortInfo) {  // update port info
    emit updateMessage(
        (ackData.errorCode() == 0x00)
            ? tr("Information successfully written.")
            : tr("Error writting information. Please try again."),
        kMessageTimeout);
    emit requestRefresh();
  }
}

////////////////////////////////////////////////////////////////////////////////
/// a helper function used at add the general MIDI information to the info tree
////////////////////////////////////////////////////////////////////////////////
QTreeWidgetItem* MIDIInfoForm::addGeneralMIDIInformation(
    QTreeWidget* parent, const MIDIInfo& midiInfo) {
  auto* midiInfoTreeItem = new QTreeWidgetItem(parent);
  midiInfoTreeItem->setText(0, tr("MIDI Information"));

  addTextItem(midiInfoTreeItem, tr("Number of MIDI Ports"),
              QString::number(midiInfo.numMIDIPorts()));

  // din related information
  if (midiInfo.numDINPairs()) {
    addTextItem(midiInfoTreeItem, tr("Number of DIN Pairs"),
                QString::number(midiInfo.numDINPairs()));
  }

  // usb device jack information
  if (midiInfo.numUSBDeviceJacks()) {
    addTextItem(midiInfoTreeItem, tr("Number of Device Jacks"),
                QString::number(midiInfo.numUSBDeviceJacks()));

    addTextItem(midiInfoTreeItem, tr("Number of USB MIDI Ports/Device Jack"),
                QString::number(midiInfo.numUSBMIDIPortPerDeviceJack()));
  }

  // usb host jack information
  if (midiInfo.numUSBHostJacks()) {
    addTextItem(midiInfoTreeItem, tr("Number of USB Host Jacks"),
                QString::number(midiInfo.numUSBHostJacks()));

    addTextItem(midiInfoTreeItem, tr("Number of USB MIDI Ports/Host Jack"),
                QString::number(midiInfo.numUSBMIDIPortPerHostJack()));

    const auto& getFunc = [ = ] {
      const auto& _midiInfo = device->get<MIDIInfo>();
      return _midiInfo.maxPortsOnMultiPortUSBDevice();
    }
    ;
    const auto& setFunc = [ = ](int value) {
      auto& _midiInfo = device->get<MIDIInfo>();
      _midiInfo.maxPortsOnMultiPortUSBDevice(value);
      device->send<SetMIDIInfoCommand>(_midiInfo);
    }
    ;

    auto* sbMaxMultiPort =
        addSpinBoxItem(ui->treeWidget, midiInfoTreeItem,
                       tr("Maximum number of ports to use on multi-port USB devices"), 1,
                       midiInfo.numUSBMIDIPortPerHostJack(), getFunc, setFunc);
    refreshList.push_back(sbMaxMultiPort);
  }

  // ethernet related information
  if (midiInfo.numEthernetJacks()) {
    addTextItem(midiInfoTreeItem, tr("Number of Ethernet Jacks"),
                QString::number(midiInfo.numEthernetJacks()));

    addTextItem(midiInfoTreeItem,
                tr("Number of RTP MIDI Sessions/Ethernet Jack"),
                QString::number(midiInfo.numRTPMIDISessionsPerEthernetJack()));

    addTextItem(midiInfoTreeItem,
                tr("Number of RTP MIDI Connections/RTP MIDI Session"),
                QString::number(midiInfo.numRTPMIDIConnectionsPerSession()));
  }

  // routing between ports on multiport USB devices
  if (midiInfo.numUSBHostJacks()) {
    const auto& getFunc = [ = ] {
      auto& _midiInfo = device->get<MIDIInfo>();
      return _midiInfo.isSet(GlobalMIDIFlags::RoutingBetweenMultiPort);
    }
    ;

    const auto& setFunc = [ = ](bool value) {
      auto& _midiInfo = device->get<MIDIInfo>();
      _midiInfo.setFlag(GlobalMIDIFlags::RoutingBetweenMultiPort, value);
      device->send<SetMIDIInfoCommand>(_midiInfo);
    }
    ;

    auto* refreshItem = TreeUtils::addCheckBox(
        ui->treeWidget, midiInfoTreeItem,
        tr("Enable routing between ports on multi-port USB devices"), getFunc,
        setFunc);
    refreshList.push_back(refreshItem);
  }

  // running status on DINs creation code
  if (midiInfo.numDINPairs()) {
    const auto& getFunc = [ = ] {
      auto& _midiInfo = device->get<MIDIInfo>();
      return _midiInfo.isSet(GlobalMIDIFlags::RunningStatusOnDINs);
    }
    ;
    const auto& setFunc = [ = ](bool value) {
      auto& _midiInfo = device->get<MIDIInfo>();
      _midiInfo.setFlag(GlobalMIDIFlags::RunningStatusOnDINs, value);
      device->send<SetMIDIInfoCommand>(_midiInfo);
    }
    ;
    auto* activeCheckBox = TreeUtils::addCheckBox(
        ui->treeWidget, midiInfoTreeItem, tr("Enable Running Status on DIN ports"),
        getFunc, setFunc);
    refreshList.push_back(activeCheckBox);
  }

  return midiInfoTreeItem;
}

////////////////////////////////////////////////////////////////////////////////
/// a helper method used to determine if the usb host port is reserved
////////////////////////////////////////////////////////////////////////////////
bool MIDIInfoForm::isReservedPort(
    const MIDIPortDetailTypes::USBHost& usbHostDetails) {
  return ((usbHostDetails.flags() & 0x01) == 0x01);
}

////////////////////////////////////////////////////////////////////////////////
/// a helper method used to determine if usb host port is in the list of
/// reserved ports
////////////////////////////////////////////////////////////////////////////////
bool MIDIInfoForm::isInReservedList(
    const MIDIPortDetailTypes::USBHost& usbHostDetails) {
  const auto& f = find_if(device->usbHostMIDIDeviceDetails,
                          [ = ](USBHostMIDIDeviceDetail usbHost) {
    return (
        ((usbHost.numMIDIIn() > 0) || (usbHost.numMIDIOut() > 0)) &&
        (usbHostDetails.hostedUSBProductID() == usbHost.hostedUSBProductID()) &&
        (usbHostDetails.hostedUSBVendorID() == usbHost.hostedUSBVendorID()));
  });
  const auto& e = boost::end(device->usbHostMIDIDeviceDetails);

  return f != e;
}

////////////////////////////////////////////////////////////////////////////////
/// a helper method used to general a list of all reservable usb host ports
////////////////////////////////////////////////////////////////////////////////
QStringList MIDIInfoForm::generateMIDIHostOptions(
    const MIDIPortDetailTypes::USBHost& usbHostDetails) {
  QStringList choices;
  choices << tr("None");

  for (const auto& choice : device->usbHostMIDIDeviceDetails) {
    const auto& numPorts = std::max(choice.numMIDIIn(), choice.numMIDIOut());
    if (numPorts > 1) {
      for (int portID = 1; portID <= numPorts; ++portID) {
        choices << QString("%1 %2 (Port %3)")
                       .arg(QString::fromStdString(choice.vendorName()))
                       .arg(QString::fromStdString(choice.productName()))
                       .arg(QString::number(portID));
      }
    } else {
      choices
          << QString("%1 %2").arg(QString::fromStdString(choice.vendorName()))
                 .arg(QString::fromStdString(choice.productName()));
    }
  }

  if ((isReservedPort(usbHostDetails)) && (!isInReservedList(usbHostDetails))) {
    choices << QString("%1 %2 (Port %3) [Not attached]")
                   .arg(QString::fromStdString(usbHostDetails.vendorName()))
                   .arg(QString::fromStdString(usbHostDetails.productName()))
                   .arg(QString::number(usbHostDetails.hostedDeviceMIDIPort()));
  }

  return choices;
}

////////////////////////////////////////////////////////////////////////////////
/// a helper method used to return the currently reserved device for a specific
/// host port
////////////////////////////////////////////////////////////////////////////////
int MIDIInfoForm::selectedUSBHostForHostDetails(
    const MIDIPortDetailTypes::USBHost& usbHostDetails, int choiceCount) {
  int selected = 0;

  if (isReservedPort(usbHostDetails)) {
    if (!isInReservedList(usbHostDetails)) {
      selected = choiceCount - 1;
    } else {
      for (const auto& choice : device->usbHostMIDIDeviceDetails) {
        if ((choice.hostedUSBVendorID() ==
             usbHostDetails.hostedUSBVendorID()) &&
            (choice.hostedUSBProductID() ==
             usbHostDetails.hostedUSBProductID())) {
          selected += usbHostDetails.hostedDeviceMIDIPort();
          break;
        } else {
          selected += std::max(choice.numMIDIIn(), choice.numMIDIOut());
        }
      }
    }
  }

  return selected;
}

////////////////////////////////////////////////////////////////////////////////
/// the start of the DIN port IDs
////////////////////////////////////////////////////////////////////////////////
Word MIDIInfoForm::dinStartPortID() const { return 1; }

////////////////////////////////////////////////////////////////////////////////
/// the end of the DIN port IDs
////////////////////////////////////////////////////////////////////////////////
Word MIDIInfoForm::dinEndPortID(const MIDIInfo& midiInfo) const {
  return dinStartPortID() + midiInfo.numDINPairs();
}

////////////////////////////////////////////////////////////////////////////////
/// the start of the USB device port IDs
////////////////////////////////////////////////////////////////////////////////
Word MIDIInfoForm::usbDeviceStartPortID(const MIDIInfo& midiInfo,
                                        Word jack) const {
  return midiInfo.numDINPairs() + 1 +
         ((jack - 1) * midiInfo.numUSBMIDIPortPerDeviceJack());
}

////////////////////////////////////////////////////////////////////////////////
/// the end of the USB device port IDs
////////////////////////////////////////////////////////////////////////////////
Word MIDIInfoForm::usbDeviceEndPortID(const MIDIInfo& midiInfo,
                                      Word jack) const {
  return usbDeviceStartPortID(midiInfo, jack) +
         midiInfo.numUSBMIDIPortPerDeviceJack();
}

////////////////////////////////////////////////////////////////////////////////
/// the start of the USB host port IDs
////////////////////////////////////////////////////////////////////////////////
Word MIDIInfoForm::usbHostStartPortID(const MIDIInfo& midiInfo,
                                      Word jack) const {
  return midiInfo.numDINPairs() +
         midiInfo.numUSBDeviceJacks() * midiInfo.numUSBMIDIPortPerDeviceJack() +
         1 + ((jack - 1) * midiInfo.numUSBMIDIPortPerHostJack());
}

////////////////////////////////////////////////////////////////////////////////
/// the end of the USB host port IDs
////////////////////////////////////////////////////////////////////////////////
Word MIDIInfoForm::usbHostEndPortID(const MIDIInfo& midiInfo, Word jack) const {
  return usbHostStartPortID(midiInfo, jack) +
         midiInfo.numUSBMIDIPortPerHostJack();
}

////////////////////////////////////////////////////////////////////////////////
/// the start of the ethernet port IDs
////////////////////////////////////////////////////////////////////////////////
Word MIDIInfoForm::ethernetStartPortID(const MIDIInfo& midiInfo) const {
  return midiInfo.numDINPairs() +
         midiInfo.numUSBDeviceJacks() * midiInfo.numUSBMIDIPortPerDeviceJack() +
         midiInfo.numUSBHostJacks() * midiInfo.numUSBMIDIPortPerHostJack() + 1;
}

////////////////////////////////////////////////////////////////////////////////
/// the end of the ethernet port IDs
////////////////////////////////////////////////////////////////////////////////
Word MIDIInfoForm::ethernetEndPortID(const MIDIInfo& midiInfo) const {
  return ethernetStartPortID(midiInfo) +
         midiInfo.numEthernetJacks() *
             midiInfo.numRTPMIDIConnectionsPerSession() *
             midiInfo.numRTPMIDISessionsPerEthernetJack();
}

////////////////////////////////////////////////////////////////////////////////
/// a getter method used to get the port name given a port ID
////////////////////////////////////////////////////////////////////////////////
QString MIDIInfoForm::getPortName(Word portID) const {
  const auto& mpi = device->get<MIDIPortInfo>(portID);
  return QString::fromStdString(mpi.portName());
}

////////////////////////////////////////////////////////////////////////////////
/// a getter method used to get the maximum length of a port name given a port
/// ID
////////////////////////////////////////////////////////////////////////////////
int MIDIInfoForm::getMaxPortNameLength(Word portID) const {
  const auto& mpi = device->get<MIDIPortInfo>(portID);
  return mpi.maxPortName();
}

////////////////////////////////////////////////////////////////////////////////
/// a getter method used to get a jack number given a port ID
////////////////////////////////////////////////////////////////////////////////
QString MIDIInfoForm::getJackNumber(Word portID) const {
  const auto& mpi = device->get<MIDIPortInfo>(portID);
  return QString::number(mpi.portInfo().common.jack);
}

////////////////////////////////////////////////////////////////////////////////
/// a getter method used to get the input enabled status given a port ID
////////////////////////////////////////////////////////////////////////////////
bool MIDIInfoForm::getPortInputEnabled(Word portID) const {
  return device->get<MIDIPortInfo>(portID).isInputEnabled();
}

////////////////////////////////////////////////////////////////////////////////
/// a getter method used to get the output enabled status given a port ID
////////////////////////////////////////////////////////////////////////////////
bool MIDIInfoForm::getPortOutputEnabled(Word portID) const {
  return device->get<MIDIPortInfo>(portID).isOutputEnabled();
}

////////////////////////////////////////////////////////////////////////////////
/// a getter method used to get the usb device host name given a port ID
////////////////////////////////////////////////////////////////////////////////
QString MIDIInfoForm::getUSBDeviceHostName(Word portID) const {
  const auto& midiDetail = device->get<MIDIPortDetail>(portID);
  const auto& usbDetails = midiDetail.getUSBDevice();
  QString hostName = QString::fromStdString(usbDetails.hostName());
  if (hostName.size() <= 0) {
    hostName = tr("None");
  }
  return hostName;
}

////////////////////////////////////////////////////////////////////////////////
/// a getter method used to get the usb device host type given a port ID
////////////////////////////////////////////////////////////////////////////////
QString MIDIInfoForm::getUSBDeviceHostType(Word portID) const {
  const auto& midiDetail = device->get<MIDIPortDetail>(portID);
  const auto& usbDetails = midiDetail.getUSBDevice();
  QString hostTypeString = tr("None");

  switch (usbDetails.hostType()) {
    case HostType::NoHost:
      hostTypeString = tr("None");
      break;

    case HostType::iOSDevice:
      hostTypeString = tr("iOS Device Host");
      break;

    case HostType::MacPC:
      hostTypeString = tr("Mac/PC Host");
      break;

    default:
      hostTypeString = tr("Unknown");
  }
  return hostTypeString;
}

////////////////////////////////////////////////////////////////////////////////
/// a getter method used to get the usb device port given a port ID
////////////////////////////////////////////////////////////////////////////////
QString MIDIInfoForm::getUSBDevicePort(Word portID) const {
  const auto& mpi = device->get<MIDIPortInfo>(portID);
  return QString::number(mpi.portInfo().usbDevice.devicePort);
}

////////////////////////////////////////////////////////////////////////////////
/// a getter method used to get the usb host port given a port ID
////////////////////////////////////////////////////////////////////////////////
QString MIDIInfoForm::getUSBHostPort(Word portID) const {
  const auto& mpi = device->get<MIDIPortInfo>(portID);
  return QString::number(mpi.portInfo().usbHost.hostPort);
}

////////////////////////////////////////////////////////////////////////////////
/// a getter method used to get the usb host connected device given a port ID
////////////////////////////////////////////////////////////////////////////////
QString MIDIInfoForm::getUSBHostConnectedDevice(Word portID) const {
  const auto& vendorName = getVendorNameForPort(portID);
  const auto& productName = getProductNameForPort(portID);
  const auto& hostedPort = getHostedPortForPort(portID);
  QString result = tr("None");
  if ((vendorName.size() > 0) || (productName.size() > 0)) {
    result = tr("%1 %2 (%3)").arg(vendorName).arg(productName).arg(hostedPort);
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
/// a getter method used to get the usb host options given a port ID
////////////////////////////////////////////////////////////////////////////////
QStringList MIDIInfoForm::getUSBHostOptions(Word portID) {
  auto& midiDetail = device->get<MIDIPortDetail>(portID);
  auto& usbHostDetails = midiDetail.getUSBHost();
  return generateMIDIHostOptions(usbHostDetails);
}

////////////////////////////////////////////////////////////////////////////////
/// a getter method used to get the selected usb host given a port ID
////////////////////////////////////////////////////////////////////////////////
int MIDIInfoForm::getSelectedUSBHostForPort(Word portID, QComboBox* comboBox) {
  auto& midiDetail = device->get<MIDIPortDetail>(portID);
  auto& usbHostDetails = midiDetail.getUSBHost();
  return selectedUSBHostForHostDetails(usbHostDetails, comboBox->count());
}

////////////////////////////////////////////////////////////////////////////////
/// a getter method used to get the vendor name given a port ID
////////////////////////////////////////////////////////////////////////////////
QString MIDIInfoForm::getVendorNameForPort(Word portID) const {
  auto& midiDetail = device->get<MIDIPortDetail>(portID);
  const auto& usbHostDetails = midiDetail.getUSBHost();
  return (
      (usbHostDetails.vendorName().size() > 0)
          ? QString::fromStdString(usbHostDetails.vendorName())
          : ((usbHostDetails.hostedUSBVendorID() != 0)
                 ? (QString("ID: %1").arg(usbHostDetails.hostedUSBVendorID(), 1,
                                          16).toUpper())
                 : (tr(""))));
}

////////////////////////////////////////////////////////////////////////////////
/// a getter method used to get the product name given a port ID
////////////////////////////////////////////////////////////////////////////////
QString MIDIInfoForm::getProductNameForPort(Word portID) const {
  const auto& midiDetail = device->get<MIDIPortDetail>(portID);
  const auto& usbHostDetails = midiDetail.getUSBHost();
  return (
      (usbHostDetails.productName().size() > 0)
          ? QString::fromStdString(usbHostDetails.productName())
          : ((usbHostDetails.hostedUSBProductID() != 0)
                 ? (QString("ID: %1").arg(usbHostDetails.hostedUSBProductID(),
                                          1, 16).toUpper())
                 : (tr(""))));
}

////////////////////////////////////////////////////////////////////////////////
/// a getter method used to get the hosted port given a port ID
////////////////////////////////////////////////////////////////////////////////
QString MIDIInfoForm::getHostedPortForPort(Word portID) const {
  const auto& midiDetail = device->get<MIDIPortDetail>(portID);
  const auto& usbHostDetails = midiDetail.getUSBHost();
  return tr("Port %1")
      .arg(QString::number(usbHostDetails.hostedDeviceMIDIPort()));
}

////////////////////////////////////////////////////////////////////////////////
/// a getter method used to get the session name for a given a port ID
////////////////////////////////////////////////////////////////////////////////
QString MIDIInfoForm::getSessionNameForPort(Word portID) const {
  const auto& midiDetail = device->get<MIDIPortDetail>(portID);
  const auto& ethernetDetails = midiDetail.getEthernet();
  auto sessionName = QString::fromStdString(ethernetDetails.sessionName());
  if (sessionName.size() == 0) {
    sessionName = tr("None");
  }
  return sessionName;
}

////////////////////////////////////////////////////////////////////////////////
/// a getter method used to get the active connection for a given a port ID
////////////////////////////////////////////////////////////////////////////////
QString MIDIInfoForm::getActiveConnectionForPort(Word portID) const {
  const auto& midiDetail = device->get<MIDIPortDetail>(portID);
  const auto& ethernetDetails = midiDetail.getEthernet();
  return QString::number(ethernetDetails.numActiveRTPConnections());
}

////////////////////////////////////////////////////////////////////////////////
/// a getter method used to get the RTP IP port for a given a port ID
////////////////////////////////////////////////////////////////////////////////
QString MIDIInfoForm::getRTPIPPortForPort(Word portID) const {
  const auto& midiDetail = device->get<MIDIPortDetail>(portID);
  const auto& ethernetDetails = midiDetail.getEthernet();
  return QString::number(ethernetDetails.rtpMIDIPortNumber());
}

////////////////////////////////////////////////////////////////////////////////
/// a getter method used to get the RTP MIDI connected IP port for a given a
/// port ID
////////////////////////////////////////////////////////////////////////////////
QString MIDIInfoForm::getRTPConnIPForPort(Word portID) const {
  const auto& rtpConnData = device->get<RTPMIDIConnectionDetail>(portID);
  return NetAddrTools::fromNetAddr(rtpConnData.connectedIPAddress());
}

////////////////////////////////////////////////////////////////////////////////
/// a getter method used to get the RTP MIDI port for a given a port ID
////////////////////////////////////////////////////////////////////////////////
QString MIDIInfoForm::getRTPMIDIPortForPort(Word portID) const {
  const auto& rtpConnData = device->get<RTPMIDIConnectionDetail>(portID);
  return QString::number(rtpConnData.rtpMIDIPortNumber());
}

////////////////////////////////////////////////////////////////////////////////
/// a getter method used to get the connection session name given a port ID
////////////////////////////////////////////////////////////////////////////////
QString MIDIInfoForm::getConnSessionNameForPort(Word portID) const {
  const auto& rtpConnData = device->get<RTPMIDIConnectionDetail>(portID);
  auto result = QString::fromStdString(rtpConnData.sessionName());

  if (result.size() == 0) {
    result = tr("None");
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
/// a setter method used to set the port name for a given port ID
////////////////////////////////////////////////////////////////////////////////
void MIDIInfoForm::setPortName(Word portID, QString value) {
  device->get<MIDIPortInfo>(portID).portName(value.toStdString());

  updateMIDIPortInfo(portID);
}

////////////////////////////////////////////////////////////////////////////////
/// a setter method used to set the input enabled bit for a given port ID
////////////////////////////////////////////////////////////////////////////////
void MIDIInfoForm::setPortInputEnabled(Word portID, bool value) {
  device->get<MIDIPortInfo>(portID).setInputEnabled(value);

  updateMIDIPortInfo(portID);
}

////////////////////////////////////////////////////////////////////////////////
/// a setter method used to set the output enabled bit for a given port ID
////////////////////////////////////////////////////////////////////////////////
void MIDIInfoForm::setPortOutputEnabled(Word portID, bool value) {
  device->get<MIDIPortInfo>(portID).setOutputEnabled(value);

  updateMIDIPortInfo(portID);
}

////////////////////////////////////////////////////////////////////////////////
/// a setter method used to set the usb host for a given port ID
////////////////////////////////////////////////////////////////////////////////
void MIDIInfoForm::setSelectedUSBHostForPort(Word portID, QComboBox* comboBox,
                                             int index) {
  auto& midiDetails = device->get<MIDIPortDetail>(portID);
  auto& usbHostDetails = midiDetails.getUSBHost();

  // index 0 is not reserved
  if (index == 0) {
    usbHostDetails.notReserved();
    updateMIDIPortDetails(portID);
  } else {
    // remove duplicate selections
    const auto& isDiffSection = [ = ](MyComboBox * other) {
      return ((other != comboBox) && (other->currentIndex() == index));
    }
    ;
    for_each(reservedComboBoxes | filtered(isDiffSection),
             bind(&MyComboBox::setCurrentIndex, _1, 0));

    // find the host device and the host port ID
    int hostPortID = 0;
    auto choice = device->usbHostMIDIDeviceDetails.begin();
    for (; choice != device->usbHostMIDIDeviceDetails.end(); ++choice) {
      hostPortID =
          std::min(static_cast<Byte>(index),
                   std::max(choice->numMIDIIn(), choice->numMIDIOut()));
      index -= hostPortID;
      if (index <= 0) {
        break;
      }
    }

    // if we have a match
    if (choice != device->usbHostMIDIDeviceDetails.end()) {

      // reserve selected port
      usbHostDetails.reserved(*choice, hostPortID);

      // update the port
      updateMIDIPortDetails(portID);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
/// a helper method used to update the port info for a given port ID (to the iCM
/// device)
////////////////////////////////////////////////////////////////////////////////
void MIDIInfoForm::updateMIDIPortInfo(Word portID) const {
  const auto& midiPortInfo = device->get<MIDIPortInfo>(portID);
  device->send<SetMIDIPortInfoCommand>(midiPortInfo);
}

////////////////////////////////////////////////////////////////////////////////
/// a helper method used to update the midi port details for a given port ID (to
/// the iCM device)
////////////////////////////////////////////////////////////////////////////////
void MIDIInfoForm::updateMIDIPortDetails(Word portID) const {
  const auto& midiDetails = device->get<MIDIPortDetail>(portID);
  device->send<SetMIDIPortDetailCommand>(midiDetails);
}
