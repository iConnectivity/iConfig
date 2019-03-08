/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "MIDIPortFiltersForm.h"
#include "ui_MIDIPortFiltersForm.h"

#include "ACK.h"
#include "MIDIPortFilter.h"
#include "MyAlgorithms.h"

#ifndef Q_MOC_RUN
#include <boost/bind.hpp>
#include <boost/range.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm/count_if.hpp>
#include <boost/shared_ptr.hpp>
#endif
#include <QMutexLocker>

using namespace GeneSysLib;
using namespace MyAlgorithms;
using namespace boost::adaptors;
using namespace boost::range;
using namespace boost;

#define kBatchTime 1500
#define kNumberOfChannels 16
#define kNumberOfMIDIChannelCommands 6

////////////////////////////////////////////////////////////////////////////////
/// Row define
////////////////////////////////////////////////////////////////////////////////
namespace PortFilterRows {
typedef enum Enum {
  PitchBendEvents = 0,
  ChannelPressureEvents,
  ProgramChangeEvents,
  ControlChangeEvents,
  PolyKeyPressureEvents,
  NoteEvents,

  ResetEvents,
  ActiveSensingEvents,
  RealtimeEvents,
  TuneRequestEvents,
  SongSelectEvents,
  SongPositionPointerEvents,
  TimeCodeEvents,
  SystemExclusiveEvents

} Enum;
}

const QString MIDIPortFiltersForm::DefaultWindowTitle = tr("MIDI Port Filters");

////////////////////////////////////////////////////////////////////////////////
/// Static query of what is required for the form
////////////////////////////////////////////////////////////////////////////////
QList<CmdEnum> MIDIPortFiltersForm::Query() {
  QList<CmdEnum> query;
  query << Command::RetMIDIInfo << Command::RetMIDIPortInfo
        << Command::RetMIDIPortFilter;
  return query;
}

////////////////////////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////////////////////////
MIDIPortFiltersForm::MIDIPortFiltersForm(CommPtr _comm, DeviceInfoPtr _device,
                                         QWidget *_parent)
    : RefreshObject(_parent),
      comm(_comm),
      device(_device),
      ui(new Ui::MIDIPortFiltersForm),
      ackHandlerID(-1) {
  // setup the UI
  ui->setupUi(this);

  // assert that the passed in comm object is valid
  Q_ASSERT(comm);
  // assert that the passed in device object is valid
  Q_ASSERT(device);

  // initiate the table listener object
  tableListener = new TableListener(ui->tableWidget, this);
  // connect cell state changed to the table listener
  connect(tableListener, SIGNAL(cellStateChanged(int, int, BlockState::Enum)),
          this, SLOT(cellStateChange(int, int, BlockState::Enum)));

  // create port selection form
  portSelectionForm = new MIDIPortSelectionForm(device, this);

  // generate a grid layout for the port selection form
  auto *const gridLayout = new QGridLayout(ui->portSelectionContainer);
  gridLayout->addWidget(portSelectionForm, 0, 0, 1, 1);
  gridLayout->setMargin(0);
  gridLayout->setSpacing(0);
  gridLayout->setVerticalSpacing(0);

  // connect this form to the port selection form
  connect(portSelectionForm, SIGNAL(selectedPortIDsChanged(PortIDVector)), this,
          SLOT(selectedPortIDsChanged(PortIDVector)));

  // conenct the filter type combo box
  connect(ui->filterTypeComboBox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(updateFilters()));

  // create sender
  sendTimer = new QTimer(this);
  // connect up the timer
  connect(sendTimer, SIGNAL(timeout()), this, SLOT(sendUpdate()));

  // connect up the ack callback
  const auto &ackHandler =
      boost::bind(&MIDIPortFiltersForm::ackCallback, this, _1, _2, _3, _4);
  ackHandlerID = comm->registerHandler(Command::ACK, ackHandler);

  // setup filter types for the filter combobox
  QStringList filterTypeList;
  filterTypeList << "Input"
                 << "Output";
  ui->filterTypeComboBox->addItems(filterTypeList);

  // setup row maps
  rowChannelStatusMap[PortFilterRows::PitchBendEvents] =
      ChannelFilterStatusBit::pitchBendEvents;
  rowChannelStatusMap[PortFilterRows::ChannelPressureEvents] =
      ChannelFilterStatusBit::channelPressureEvents;
  rowChannelStatusMap[PortFilterRows::ProgramChangeEvents] =
      ChannelFilterStatusBit::programChangeEvents;
  rowChannelStatusMap[PortFilterRows::ControlChangeEvents] =
      ChannelFilterStatusBit::controlChangeEvents;
  rowChannelStatusMap[PortFilterRows::PolyKeyPressureEvents] =
      ChannelFilterStatusBit::polyKeyPressureEvents;
  rowChannelStatusMap[PortFilterRows::NoteEvents] =
      ChannelFilterStatusBit::noteEvents;

  rowFilterMap[PortFilterRows::ResetEvents] = FilterStatusBit::resetEvents;
  rowFilterMap[PortFilterRows::ActiveSensingEvents] =
      FilterStatusBit::activeSensingEvents;
  rowFilterMap[PortFilterRows::RealtimeEvents] =
      FilterStatusBit::realtimeEvents;
  rowFilterMap[PortFilterRows::TuneRequestEvents] =
      FilterStatusBit::tuneRequestEvents;
  rowFilterMap[PortFilterRows::SongSelectEvents] =
      FilterStatusBit::songSelectEvents;
  rowFilterMap[PortFilterRows::SongPositionPointerEvents] =
      FilterStatusBit::songPositionPointerEvents;
  rowFilterMap[PortFilterRows::TimeCodeEvents] =
      FilterStatusBit::timeCodeEvents;
  rowFilterMap[PortFilterRows::SystemExclusiveEvents] =
      FilterStatusBit::systemExclusiveEvents;

  // create the vertical headers for the table
  QStringList vertHeaderList;
  vertHeaderList
      << tr("Pitch Bend Events") << tr("Channel Pressure Events")
      << tr("Program Change Events") << tr("Control Change (CC) Events")
      << tr("Poly Key Pressure Events") << tr("Note On/Off Events")
      << tr("Reset Events") << tr("Active Sensing Events")
      << tr("Realtime Events") << tr("Tune Request Events")
      << tr("Song Select Events") << tr("Song Position Pointer Events")
      << tr("Time Code (MTC) Events") << tr("System Exclusive Events");
  ui->tableWidget->setRowCount(vertHeaderList.count());
  ui->tableWidget->setVerticalHeaderLabels(vertHeaderList);

  // setup the column count
  ui->tableWidget->setColumnCount(kNumberOfChannels);

  // get the vert header
  auto *const vertHeader = ui->tableWidget->verticalHeader();
  // assert that the vert header is valid

  Q_ASSERT(vertHeader);
  // set the height
  vertHeader->setDefaultSectionSize(25);

  // add a corner button label
  tableListener->addCornerLabel(tr("Channel"));

  // loop through all non channel midi command to set the span to the full width
  for (auto row = kNumberOfMIDIChannelCommands;
       row < ui->tableWidget->rowCount(); ++row) {
    ui->tableWidget->setSpan(row, 0, 1, kNumberOfChannels);
  }

  // get the pointer to the horizontal header
  auto *const horzHeader = ui->tableWidget->horizontalHeader();
  // assert that the horizontal header is valid
  Q_ASSERT(horzHeader);
  // set the resize mode for the horizontal header to stretch with the parent
  horzHeader->setResizeMode(QHeaderView::Stretch);

  // loop through all rows
  for (auto row = 0; row < ui->tableWidget->rowCount();
       row += ui->tableWidget->rowSpan(row, 0)) {
    // loop through all columns in the row
    for (auto col = 0; col < ui->tableWidget->columnCount();
         col += ui->tableWidget->columnSpan(row, col)) {

      // create an empty label
      auto *const label = new QLabel();
      // scale contents of empty label
      label->setScaledContents(true);
      // set base property to empty
      label->setProperty(kBlockState, BlockState::Empty);
      // add label to cell
      ui->tableWidget->setCellWidget(row, col, label);
    }
  }

  // set the window title
  setWindowTitle(MIDIPortFiltersForm::DefaultWindowTitle);

  // hide the some title
  ui->someIconLabel->setVisible(false);
  ui->someTextLabel->setVisible(false);

  // update all the filters
  updateFilters();
}

////////////////////////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////////////////////////
MIDIPortFiltersForm::~MIDIPortFiltersForm() {
  // get the number of pending updates atomically
  int count = 0;
  updateMutex.lock();
  count = updateList.count();
  updateMutex.unlock();

  // if there is a pending update then send it
  if (count > 0) {
    sendUpdate();
  }
  // unregister ack callback
  comm->unRegisterHandler(Command::ACK, ackHandlerID);
}

////////////////////////////////////////////////////////////////////////////////
/// Selected port ID changes notification
////////////////////////////////////////////////////////////////////////////////
void MIDIPortFiltersForm::selectedPortIDsChanged(PortIDVector portIDs) {
  // change the legend
  bool multiplePorts = (portIDs.size() > 1);

  ui->someIconLabel->setVisible(multiplePorts);
  ui->someTextLabel->setVisible(multiplePorts);

  if (multiplePorts) {
    ui->enabledTextLabel->setText(tr("All"));
    ui->disabledTextLabel->setText(tr("None"));
  } else {
    ui->enabledTextLabel->setText(tr("Enabled"));
    ui->disabledTextLabel->setText(tr("Disabled"));
  }

  // update the filters
  updateFilters();
}

////////////////////////////////////////////////////////////////////////////////
/// Update filters callback
////////////////////////////////////////////////////////////////////////////////
void MIDIPortFiltersForm::updateFilters() {
  tableListener->updateWidgets(
      bind(&MIDIPortFiltersForm::stateForCell, this, _1, _2));
}

////////////////////////////////////////////////////////////////////////////////
/// Sends the update to the device
////////////////////////////////////////////////////////////////////////////////
void MIDIPortFiltersForm::sendUpdate() {
  // stop the timer
  sendTimer->stop();

  // lock the mutex (it will be unlocked when it goes out of scope
  QMutexLocker locker(&updateMutex);

  // get the iterator for the hash table
  QHashIterator<FilterIDEnum, Word> i(updateList);

  // iterate through the hash
  while (i.hasNext()) {
    // move to the next hash value
    i.next();

    // get the port ID
    const auto &portID = i.value();
    // get the filter ID
    const auto &filterID = i.key();

    // get the filterID for the portID and filterID
    const auto &filterData = device->midiPortFilter(portID, filterID);

    // send the set sysex method to the device
    device->send<SetMIDIPortFilterCommand>(filterData);
  }

  // clear the update list
  updateList.clear();
}

////////////////////////////////////////////////////////////////////////////////
/// Called when there is a change to the cell state
////////////////////////////////////////////////////////////////////////////////
void MIDIPortFiltersForm::cellStateChange(int row, int col,
                                          BlockState::Enum state) {
  // determine if it is a channel status
  const auto &isChannelStatus = contains(rowChannelStatusMap, row);
  // determine if it is a filter status
  const auto &isFilterStatus = contains(rowFilterMap, row);

  // determine if the value should be set
  const auto &value = (state == BlockState::Full);

  // stop the timer
  sendTimer->stop();

  // lock the update list
  QMutexLocker locker(&updateMutex);

  // get the selected ports list
  const auto &selectedPorts = portSelectionForm->selectedPortIDs();

  // loop through the selected ports
  for (const auto &portID : selectedPorts) {
    // get the filter data for current selected port
    auto &filterData = device->midiPortFilter(portID, currentFilterID());

    // if it is a channel status
    if (isChannelStatus) {
      // get column filter status
      auto &colChFilterStatus = filterData.channelFilterStatus_at(col);
      colChFilterStatus.set(rowChannelStatusMap.at(row), value);
    } else if (isFilterStatus) {  // if it is a filter status
      filterData.filterStatus().set(rowFilterMap.at(row), value);
    }
    // add current portID and current filter ID to update list
    updateList[currentFilterID()] = portID;
  }
  sendTimer->start(kBatchTime);
}

////////////////////////////////////////////////////////////////////////////////
/// Helper method to determine if a cell is set
////////////////////////////////////////////////////////////////////////////////
bool MIDIPortFiltersForm::isCellSet(int row, int col, int portID) const {
  bool result = false;

  // determine if this is a channel status map
  const auto &isChannelStatusMap = contains(rowChannelStatusMap, row);
  // determine if this is a filter map
  const auto &isFilterMap = contains(rowFilterMap, row);

  // get the filter data for the current port ID
  const auto &filterData = device->midiPortFilter(portID, currentFilterID());

  // if this is the channel status map
  if (isChannelStatusMap) {
    // determine if channel statis is set
    result =
        filterData.channelFilterStatus_at(col)[rowChannelStatusMap.at(row)];
  } else if (isFilterMap) {
    // determine if filter status is set
    result = filterData.filterStatus()[rowFilterMap.at(row)];
  }
  return result;
}

////////////////////////////////////////////////////////////////////////////////
/// Helper method to determine the state of a cell
////////////////////////////////////////////////////////////////////////////////
BlockState::Enum MIDIPortFiltersForm::stateForCell(int row, int col) const {
  const auto &selectedPorts = portSelectionForm->selectedPortIDs();
  const auto &isSet = bind(&MIDIPortFiltersForm::isCellSet, this, row, col, _1);
  auto result = BlockState::Empty;

  // count the number of ports that are set
  size_t count = count_if(selectedPorts, isSet);

  if (count == 0) {  // if the number of ports that are set is zero then the
                     // state is empty
    result = BlockState::Empty;
  } else if (count <
             selectedPorts.size()) {  // if the number of ports that are set is
                                      // not the total number of selected ports
                                      // than the state is half
    result = BlockState::Half;
  } else {  // the number of ports sets equals the total number of ports, so the
            // state is full
    result = BlockState::Full;
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
/// The ACK callback
////////////////////////////////////////////////////////////////////////////////
void MIDIPortFiltersForm::ackCallback(CmdEnum, DeviceID, Word,
                                      commandData_t commandData) {
  const auto &ackData = commandData.get<ACK>();
  if (ackData.commandID() == Command::SetMIDIPortFilter) {
    emit requestRefresh();
  }
}

////////////////////////////////////////////////////////////////////////////////
/// This method gets called when the form needs to refresh
////////////////////////////////////////////////////////////////////////////////
void MIDIPortFiltersForm::refreshWidget() {
  portSelectionForm->updateTree();
  this->updateFilters();
}

////////////////////////////////////////////////////////////////////////////////
/// This method gets the current filter ID
////////////////////////////////////////////////////////////////////////////////
FilterIDEnum MIDIPortFiltersForm::currentFilterID() const {
  return (ui->filterTypeComboBox->currentIndex() == 0) ? FilterID::InputFilter
                                                       : FilterID::OutputFilter;
}
