/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "MIDIPortRoutingForm.h"
#include "ui_MIDIPortRoutingForm.h"

#include "ACK.h"
#include "MIDIPortRoute.h"
#include "MyAlgorithms.h"

#ifndef Q_MOC_RUN
#include <boost/bind.hpp>
#include <boost/range.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/range/algorithm/count_if.hpp>
#include <boost/shared_ptr.hpp>
#endif

using namespace GeneSysLib;
using namespace MyAlgorithms;
using namespace boost::adaptors;
using namespace boost::range;
using namespace boost;

////////////////////////////////////////////////////////////////////////////////
/// Static information associated with this form
////////////////////////////////////////////////////////////////////////////////
const QString MIDIPortRoutingForm::DefaultWindowTitle = tr("MIDI Port Routing");
const char *MIDIPortRoutingForm::kPortIDProperty = "PortIDProperty";
const int MIDIPortRoutingForm::kSendTime = 1500;

////////////////////////////////////////////////////////////////////////////////
/// The required query for this form
////////////////////////////////////////////////////////////////////////////////
QList<CmdEnum> MIDIPortRoutingForm::Query() {
  QList<CmdEnum> query;
  query << Command::RetMIDIInfo << Command::RetMIDIPortInfo
        << Command::RetMIDIPortRoute;
  return query;
}

////////////////////////////////////////////////////////////////////////////////
/// The constructor
////////////////////////////////////////////////////////////////////////////////
MIDIPortRoutingForm::MIDIPortRoutingForm(DeviceInfoPtr _device,
                                         QWidget *_parent)
    : RefreshObject(_parent),
      device(_device),
      ui(new Ui::MIDIPortRoutingForm),
      ackHandlerID(-1) {
  // setup the UI
  ui->setupUi(this);

  // set the window title
  setWindowTitle(MIDIPortRoutingForm::DefaultWindowTitle);

  // check that the device objects exist
  Q_ASSERT(device);

  // config that the midi information exists
  if (device->contains<MIDIInfo>()) {
    // Get the midi info
    midiInfo = device->get<MIDIInfo>();

    // create a port selection from passing in the device and this as the parent
    portSelectionForm = new MIDIPortSelectionForm(device, this);

    // create a GridLayout for the port selection and the table forms
    auto *const gridLayout = new QGridLayout(ui->portSelectionContainer);
    gridLayout->addWidget(portSelectionForm, 0, 0, 1, 1);
    gridLayout->setMargin(0);
    gridLayout->setSpacing(0);
    gridLayout->setVerticalSpacing(0);

    // create a table listener object
    tableListener = new TableListener(ui->tableWidget, this);

    // create the table
    createTable();

    // update the table
    updateRouting();

    // connect listeners to the table cell state changes
    connect(tableListener, SIGNAL(cellStateChanged(int, int, BlockState::Enum)),
            this, SLOT(cellChanged(int, int, BlockState::Enum)));

    // connect listers to the port ID change from the port selection form
    connect(portSelectionForm, SIGNAL(selectedPortIDsChanged(PortIDVector)),
            this, SLOT(selectedPortIDsChanged(PortIDVector)));

    // add ack handler callback
    auto ackHandler =
        bind(&MIDIPortRoutingForm::ackCallback, this, _1, _2, _3, _4);
    ackHandlerID = device->registerHandler(Command::ACK, ackHandler);

    // hide the both legend
    ui->someIconLabel->setVisible(false);
    ui->someTextLabel->setVisible(false);

    // create timer registerHandler a listener to its events
    sendTimer = new QTimer(this);
    connect(sendTimer, SIGNAL(timeout()), this, SLOT(sendUpdate()));
  }
}

////////////////////////////////////////////////////////////////////////////////
/// The destructor
////////////////////////////////////////////////////////////////////////////////
MIDIPortRoutingForm::~MIDIPortRoutingForm() {
  // get the update count
  int count = 0;
  updateMutex.lock();
  count = updateSet.size();
  updateMutex.unlock();

  // if there are things to update then do that before destructing
  if (count > 0) {
    sendUpdate();
  }

  // unregister the acknowledgement callback
  device->unRegisterHandler(Command::ACK, ackHandlerID);
}

////////////////////////////////////////////////////////////////////////////////
/// This method gets called if the selected source ports change
////////////////////////////////////////////////////////////////////////////////
void MIDIPortRoutingForm::selectedPortIDsChanged(PortIDVector portIDs) {
  bool multipleSelected = (portIDs.size() > 1);
  ui->someIconLabel->setVisible(multipleSelected);
  ui->someTextLabel->setVisible(multipleSelected);

  if (multipleSelected) {
    ui->enabledTextLabel->setText(tr("All"));
    ui->disabledTextLabel->setText(tr("None"));
  } else {
    ui->enabledTextLabel->setText(tr("Enabled"));
    ui->disabledTextLabel->setText(tr("Disabled"));
  }

  updateRouting();
}

////////////////////////////////////////////////////////////////////////////////
/// This method gets called to change the state of the cell at location (row,
/// col)
////////////////////////////////////////////////////////////////////////////////
void MIDIPortRoutingForm::cellChanged(int row, int col,
                                      BlockState::Enum state) {
  // get the label associated with the click cell
  const auto *const label = tableListener->labelAt(row, col);
  // assert that a label exists
  if (label) {
    // get the associated destination port ID of the label
    const auto &destPortID = label->property(kPortIDProperty).toInt();

    // assert that the associated destination port ID is in bounds
    Q_ASSERT((destPortID > 0) && (destPortID <= midiInfo.numMIDIPorts()));

    // get the list of selected source ports
    const auto &selectedSrcPorts = portSelectionForm->selectedPortIDs();

    // loop through all the source ports
    for (const auto &srcPortID : selectedSrcPorts) {
      // get route for the current source port
      auto &portRoute = device->get<MIDIPortRoute>(srcPortID);
      // set the route from source to destination to true only if the passed in
      // state is full
      portRoute.setRoutedTo(destPortID, (state == BlockState::Full));
    }

    // stop the timer (if it is running)
    sendTimer->stop();
    // request a mutex lock
    updateMutex.lock();

    // copy the port ID of the selected source ports to the update list
    std::copy(selectedSrcPorts.begin(), selectedSrcPorts.end(),
              std::inserter(updateSet, updateSet.end()));

    // release the mutex lock
    updateMutex.unlock();

    // start the timer for the next update command
    sendTimer->start(kSendTime);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// This method gets called to refresh all the routes
////////////////////////////////////////////////////////////////////////////////
void MIDIPortRoutingForm::updateRouting() {
  tableListener->updateWidgets(
      boost::bind(&MIDIPortRoutingForm::cellState, this, _1, _2));

  ui->tableWidget->viewport()->update();
}

////////////////////////////////////////////////////////////////////////////////
/// This method will stop the countdown and send all updated routing information
/// to device
////////////////////////////////////////////////////////////////////////////////
void MIDIPortRoutingForm::sendUpdate() {
  // stop the timer so this method does not get call indefinately
  sendTimer->stop();

  // request a lock
  updateMutex.lock();

    // loop through all the updated port routes
  for (const auto &portRoute : updateSet | transformed([&](Word portID) {
    return device->get<MIDIPortRoute>(portID);
  })) {
    // send the updated port route to the device
    device->send<SetMIDIPortRouteCommand>(portRoute);
  }

  // clear the update set
  updateSet.clear();

  // release the lock
  updateMutex.unlock();
}

////////////////////////////////////////////////////////////////////////////////
/// This method will create the table UI element
////////////////////////////////////////////////////////////////////////////////
void MIDIPortRoutingForm::createTable() {
  // clear the table in case this method gets called multiple times
  ui->tableWidget->clear();

  // clear the stored label links in case this method gets called multiple times
  labelPortIDMap.clear();

  // create a vertical header
  QStringList vertHeader;

  // sets the number of columns to the maximum number of jacks per port type
  ui->tableWidget->setColumnCount(maxPorts());

  // set the default column width
  for (auto i = 0; i < maxPorts(); ++i) {
    ui->tableWidget->setColumnWidth(i, 35);
  }

  // create a header for each of the USB device jacks
  for (auto i = 1; i <= midiInfo.numUSBDeviceJacks(); ++i) {
    vertHeader << QString("USB Device Jack %1").arg(QString::number(i));
  }
  // create a header for each of the USB host jacks
  for (auto i = 1; i <= midiInfo.numUSBHostJacks(); ++i) {
    vertHeader << QString("USB Host Jack %1").arg(QString::number(i));
  }
  // create a header for each of the ethernet jacks
  for (auto i = 1; i <= midiInfo.numEthernetJacks(); ++i) {
    vertHeader << QString("Ethernet Jack %1").arg(QString::number(i));
  }
  // create a header for each of the din jack pairs
  for (auto i = 1; i <= midiInfo.numDINPairs(); ++i) {
    vertHeader << QString("DIN Jack %1").arg(QString::number(i));
  }

  // set the row count and vertical header label
  ui->tableWidget->setRowCount(vertHeader.count());

  // assign the vertical header created above to the table widget
  ui->tableWidget->setVerticalHeaderLabels(vertHeader);

  // make sure that the vertical header is visable
  ui->tableWidget->verticalHeader()->show();

    // loop through all stored port info objects
  device->for_each<MIDIPortInfo>([&](const MIDIPortInfo & portInfo) {
    // get the row of the current port info object
    const auto &row = rowForPort(portInfo);
    // get the column for the current port info object
    const auto &col = colForPort(portInfo);

    // set the span for the current port info object
    const auto &span = spanForPort(portInfo);
    if (span > 1) {
      ui->tableWidget->setSpan(row, col, 1, span);
    }
    // add a label at location (row, col) for the current port info object
    addLabel(row, col, portInfo);
  });

  // set the horizontal headers to be empty
  QStringList horizontalHeaders;
  std::fill_n(std::back_inserter(horizontalHeaders), maxPorts(), tr(""));
  ui->tableWidget->setHorizontalHeaderLabels(horizontalHeaders);

  // set the resize mode for the horizontal headers to stretch with the dialog
  auto *const horzHdr = ui->tableWidget->horizontalHeader();
  Q_ASSERT(horzHdr);
  horzHdr->setResizeMode(QHeaderView::Stretch);
}

////////////////////////////////////////////////////////////////////////////////
/// A helper function used to get the maximum number of ports
////////////////////////////////////////////////////////////////////////////////
int MIDIPortRoutingForm::maxPorts() const {
  // returns the max number of ports for the device jack, the host jack and the
  // ethernet jack
  return std::max(std::max(midiInfo.numUSBMIDIPortPerDeviceJack(),
                           midiInfo.numUSBMIDIPortPerHostJack()),
                  midiInfo.numRTPMIDISessionsPerEthernetJack());
}

////////////////////////////////////////////////////////////////////////////////
/// A helper function used to determine the number of rows for a port info
/// object
////////////////////////////////////////////////////////////////////////////////
int MIDIPortRoutingForm::rowForPort(MIDIPortInfo portInfo) const {
  int row = 0;
  bool matched = false;
  /* the order of rows is:
   *  - USB Device     (*)optional
   *  - USB Host       (*)optional
   *  - Ethernet       (*)optional
   *  - DINS(Same Row) (*)optional
   */

  // count the number of USB device ports
  if (portInfo.isOfType(PortType::USBDevice)) {
    row = portInfo.portInfo().usbDevice.jack - 1;
    matched = true;
  } else {
    row += midiInfo.numUSBDeviceJacks();
  }

  // count the number of USB host ports
  if (portInfo.isOfType(PortType::USBHost)) {
    row += portInfo.portInfo().common.jack - 1;
    matched = true;
  } else if (!matched) {
    row += midiInfo.numUSBHostJacks();
  }

  // count the number of ethernet ports
  if (portInfo.isOfType(PortType::Ethernet)) {
    row += portInfo.portInfo().common.jack - 1;
    matched = true;
  } else if (!matched) {
    row += midiInfo.numEthernetJacks();
  }

  // count the number of DIN ports
  if (portInfo.isOfType(PortType::DIN)) {
    row += portInfo.portInfo().din.jack - 1;
  }

  return row;
}

////////////////////////////////////////////////////////////////////////////////
/// A helper method that returns the span for a given port info object
////////////////////////////////////////////////////////////////////////////////
int MIDIPortRoutingForm::spanForPort(MIDIPortInfo portInfo) const {
  // the default span is 1
  int span = 1;

  // the span is the maximum number of port divided by the number of ports per
  // jack
  if (portInfo.portType() == PortType::USBDevice) {
    span = maxPorts() / midiInfo.numUSBMIDIPortPerDeviceJack();
  } else if (portInfo.portType() == PortType::USBHost) {
    span = maxPorts() / midiInfo.numUSBMIDIPortPerHostJack();
  } else if (portInfo.portType() == PortType::Ethernet) {
    span = maxPorts() / midiInfo.numRTPMIDISessionsPerEthernetJack();
  } else if (portInfo.portType() == PortType::DIN) {
    span = maxPorts();
  }

  // return the span for the passed in port info object
  return span;
}

////////////////////////////////////////////////////////////////////////////////
/// A helper method that returns the number of columns for the given port type
////////////////////////////////////////////////////////////////////////////////
int MIDIPortRoutingForm::colForPort(MIDIPortInfo portInfo) const {
  int result = spanForPort(portInfo);

  // determine the number of columns for each port type
  if (portInfo.isOfType(PortType::DIN)) {
    result = 0;  // DINs only have 1 col
  } else if (portInfo.isOfType(PortType::USBDevice)) {
    result *= (portInfo.portInfo().usbDevice.devicePort - 1);
  } else if (portInfo.isOfType(PortType::USBHost)) {
    result *= (portInfo.portInfo().usbHost.hostPort - 1);
  } else if (portInfo.isOfType(PortType::Ethernet)) {
    result *= (portInfo.portInfo().ethernet.session - 1);
  }
  return result;
}

////////////////////////////////////////////////////////////////////////////////
/// This method will add a label at location (row, col) with the passed in port
/// info.
////////////////////////////////////////////////////////////////////////////////
void MIDIPortRoutingForm::addLabel(int row, int col, MIDIPortInfo portInfo) {
  // get the destination port ID
  const auto &dstPortID = portInfo.portID();

  // get port name of the port info for tool tip
  const auto &toolTip = QString::fromStdString(portInfo.portName());

  // create variable for the text
  QString text;

  // add a title to the lable
  if (portInfo.isOfType(PortType::DIN)) {
    // add the port name
    text = QString::fromStdString(portInfo.portName());
  } else {
    // the number of the col the user sees
    text = QString::number(col / spanForPort(portInfo) + 1);
  }

  // create the text label
  auto *const textLabel = new QLabel(text);

  // align the text label to center
  textLabel->setAlignment(Qt::AlignCenter);

  // add the tool tip
  textLabel->setToolTip(toolTip);

  // create the grid layout
  auto *const gridLayout = new QGridLayout();

  // set the grid layout margin to 0
  gridLayout->setMargin(0);

  // add the text label to the layout
  gridLayout->addWidget(textLabel);

  // create the pixmap label
  auto *const pixmapLabel = new QLabel();

  // scale the content to fit the pixmap layout
  pixmapLabel->setScaledContents(true);

  // set the property of the label to the port ID so a reverse lookup can be
  // done later
  pixmapLabel->setProperty(kPortIDProperty, QVariant::fromValue(dstPortID));

  // set the layout of the pixmap layout to the grid layout defined above
  pixmapLabel->setLayout(gridLayout);

  // add the label to the table
  this->ui->tableWidget->setCellWidget(row, col, pixmapLabel);

  // add label to map so it can be looked up later
  this->labelPortIDMap[dstPortID] = pixmapLabel;
}

////////////////////////////////////////////////////////////////////////////////
/// The method that gets called when the system receives an ACK command.
////////////////////////////////////////////////////////////////////////////////
void MIDIPortRoutingForm::ackCallback(CmdEnum, DeviceID, Word,
                                      commandData_t commandData) {
  // cast the command data to the ACK data type
  const auto &ackData = commandData.get<ACK>();

  // only refresh the view if the command was a SetMIDIPortRoute command
  if (ackData.commandID() == Command::SetMIDIPortRoute) {
    // request a refresh
    requestRefresh();
  }
}

////////////////////////////////////////////////////////////////////////////////
/// This is a helper method used to determine if a route exists from the passed
/// in source port the the passed in destination port
/// Returns true is a route exists from the source to the destination.
/// Returns false otherwise.
////////////////////////////////////////////////////////////////////////////////
bool MIDIPortRoutingForm::portRouted(Word srcPortID, Word destPortID) const {
  // get the from port in question
  const auto &portRoute = device->get<MIDIPortRoute>(srcPortID);

  // determine if a route exisites from the source to the destination
  return portRoute.isRoutedTo(destPortID);
}

////////////////////////////////////////////////////////////////////////////////
/// A helper method used to determine the state for the cell at location (row,
/// col)
////////////////////////////////////////////////////////////////////////////////
BlockState::Enum MIDIPortRoutingForm::cellState(int row, int col) const {
  // get a list of all the selected source ports
  const auto &selectedSrcPorts = portSelectionForm->selectedPortIDs();

  // get the label at (row, col)
  auto *const cellLabel = tableListener->labelAt(row, col);

  // assert that there is a cell label at location (row, col)
  if (!cellLabel) {
    return BlockState::Empty;
  }

  // get the destination port ID of the label at cell(row, col)
  const auto &destPortID = cellLabel->property(kPortIDProperty).toInt();

  // assert that the stored destPortID is in range
  Q_ASSERT((destPortID > 0) && (destPortID <= midiInfo.numMIDIPorts()));

  // create a convinence variable for the bind function
  const auto &isRouted =
      bind(&MIDIPortRoutingForm::portRouted, this, _1, destPortID);

  // create a return variable to hold the state of the selected destination port
  BlockState::Enum state;

  // get the number of source ports that are routed to the destination
  const auto &routeCount = count_if(selectedSrcPorts, isRouted);

  // determine the state of the selected destination port
  if (routeCount == 0) {
    // if none of selected source ports are routed to the destination then the
    // state is empty
    state = BlockState::Empty;
  } else if (routeCount == static_cast<long>(selectedSrcPorts.size())) {
    // if all selected source ports are routed to the destination then the state
    // is full
    state = BlockState::Full;
  } else {
    // otherwise the state is half
    state = BlockState::Half;
  }

  // return the state of the selected destination port
  return state;
}

////////////////////////////////////////////////////////////////////////////////
/// A method that get called when form should refresh
////////////////////////////////////////////////////////////////////////////////
void MIDIPortRoutingForm::refreshWidget() {
  // update the port selection form to reflect changes to port names
  portSelectionForm->updateTree();

  // update the routing view of the selected source port
  this->updateRouting();
}
