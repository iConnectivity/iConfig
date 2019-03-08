/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "MIDIControllerFilterForm.h"
#include "ui_MIDIControllerFilterForm.h"

#include "ACK.h"
#include "CCList.h"
#include "DeviceID.h"
#include "MIDIPortFilter.h"
#include "MyComboBox.h"

#include <QComboBox>

#ifndef Q_MOC_RUN
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#endif

using namespace GeneSysLib;
using namespace boost;

////////////////////////////////////////////////////////////////////////////////
/// columns defines
////////////////////////////////////////////////////////////////////////////////
namespace FilterCtlrCols {
typedef enum Enum {
  Channel1 = 0,
  Channel2,
  Channel3,
  Channel4,
  Channel5,
  Channel6,
  Channel7,
  Channel8,
  Channel9,
  Channel10,
  Channel11,
  Channel12,
  Channel13,
  Channel14,
  Channel15,
  Channel16,
  ControllerID
} Enum;
}

////////////////////////////////////////////////////////////////////////////////
/// static defines
////////////////////////////////////////////////////////////////////////////////
const QString MIDIControllerFilterForm::DefaultWindowTitle =
    tr("MIDI Controller Filters");
const float MIDIControllerFilterForm::kBatchTime = 1500;

////////////////////////////////////////////////////////////////////////////////
/// static query required
////////////////////////////////////////////////////////////////////////////////
QList<CmdEnum> MIDIControllerFilterForm::Query() {
  QList<CmdEnum> query;
  query << Command::RetMIDIInfo << Command::RetMIDIPortInfo
        << Command::RetMIDIPortFilter;
  return query;
}

////////////////////////////////////////////////////////////////////////////////
/// constructor
////////////////////////////////////////////////////////////////////////////////
MIDIControllerFilterForm::MIDIControllerFilterForm(CommPtr _comm,
                                                   DeviceInfoPtr _device,
                                                   QWidget *_parent)
    : RefreshObject(_parent),
      comm(_comm),
      device(_device),
      ui(new Ui::MIDIControllerFilterForm) {
  Q_ASSERT(comm);
  Q_ASSERT(device);
  ui->setupUi(this);

  tableListener = new TableListener(ui->tableWidget, this);
  tableListener->addIgnoreCol(FilterCtlrCols::ControllerID);
  connect(tableListener, SIGNAL(cellStateChanged(int, int, BlockState::Enum)),
          this, SLOT(cellStateChange(int, int, BlockState::Enum)));

  portSelectionForm = new MIDIPortSelectionForm(device, this);

  auto *const gridLayout = new QGridLayout(ui->portSelectionContainer);
  gridLayout->addWidget(portSelectionForm, 0, 0, 1, 1);
  gridLayout->setMargin(0);
  gridLayout->setSpacing(0);
  gridLayout->setVerticalSpacing(0);

  connect(portSelectionForm, SIGNAL(selectedPortIDsChanged(PortIDVector)), this,
          SLOT(selectedPortIDsChanged(PortIDVector)));
  portSelectionForm->setSelectionMode(QAbstractItemView::SingleSelection);

  sendTimer = new QTimer(this);
  connect(sendTimer, SIGNAL(timeout()), this, SLOT(sendUpdate()));

  connect(ui->filterTypeComboBox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(updateFilters()));

  colChBitmapMap[FilterCtlrCols::Channel1] = ChannelBitmapBit::channel1;
  colChBitmapMap[FilterCtlrCols::Channel2] = ChannelBitmapBit::channel2;
  colChBitmapMap[FilterCtlrCols::Channel3] = ChannelBitmapBit::channel3;
  colChBitmapMap[FilterCtlrCols::Channel4] = ChannelBitmapBit::channel4;
  colChBitmapMap[FilterCtlrCols::Channel5] = ChannelBitmapBit::channel5;
  colChBitmapMap[FilterCtlrCols::Channel6] = ChannelBitmapBit::channel6;
  colChBitmapMap[FilterCtlrCols::Channel7] = ChannelBitmapBit::channel7;
  colChBitmapMap[FilterCtlrCols::Channel8] = ChannelBitmapBit::channel8;
  colChBitmapMap[FilterCtlrCols::Channel9] = ChannelBitmapBit::channel9;
  colChBitmapMap[FilterCtlrCols::Channel10] = ChannelBitmapBit::channel10;
  colChBitmapMap[FilterCtlrCols::Channel11] = ChannelBitmapBit::channel11;
  colChBitmapMap[FilterCtlrCols::Channel12] = ChannelBitmapBit::channel12;
  colChBitmapMap[FilterCtlrCols::Channel13] = ChannelBitmapBit::channel13;
  colChBitmapMap[FilterCtlrCols::Channel14] = ChannelBitmapBit::channel14;
  colChBitmapMap[FilterCtlrCols::Channel15] = ChannelBitmapBit::channel15;
  colChBitmapMap[FilterCtlrCols::Channel16] = ChannelBitmapBit::channel16;

  const auto &portFilter = device->midiPortFilter(1, FilterID::InputFilter);

  QStringList vertHeaderList;
  for (auto i = 1; i <= (int) portFilter.controllerFilters().size(); ++i) {
    vertHeaderList << QString("Controller Filter %1").arg(QString::number(i));
  }
  ui->tableWidget->setRowCount(vertHeaderList.count());
  ui->tableWidget->setVerticalHeaderLabels(vertHeaderList);
  ui->tableWidget->verticalHeader()->setDefaultSectionSize(25);

  QStringList horzHeaderList;
  for (auto i = 1; i <= 16; ++i) {
    horzHeaderList << QString::number(i);
  }
  horzHeaderList << tr("Controller ID");
  ui->tableWidget->setColumnCount(horzHeaderList.count());
  ui->tableWidget->setHorizontalHeaderLabels(horzHeaderList);

  auto *const horzHeader = ui->tableWidget->horizontalHeader();
  Q_ASSERT(horzHeader);
  horzHeader->setResizeMode(QHeaderView::Fixed);
  horzHeader->setDefaultSectionSize(25);
  horzHeader->setResizeMode(horzHeaderList.count() - 1, QHeaderView::Stretch);

  tableListener->addCornerLabel(tr("Channel"));

  // add the labels
  for (auto row = 0; row < ui->tableWidget->rowCount(); ++row) {
    for (auto col = 0; col < ui->tableWidget->columnCount() - 1; ++col) {
      addEmptyLabel(row, col);
    }
  }
  tableListener->updateWidgets(
      boost::bind(&MIDIControllerFilterForm::stateForCell, this, _1, _2));

  // add combo boxes
  for (auto row = 0; row < ui->tableWidget->rowCount(); ++row) {
    addCCComboBox(row);
  }

  ui->tableWidget->setColumnWidth(FilterCtlrCols::ControllerID, 200);

  setWindowTitle(MIDIControllerFilterForm::DefaultWindowTitle);

  updateFilters();
}

////////////////////////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////////////////////////
MIDIControllerFilterForm::~MIDIControllerFilterForm() {
  updateMutex.lock();
  bool pendingChanges = (!updateList.empty());
  updateMutex.unlock();

  if (pendingChanges) {
    sendUpdate();
  }
}

////////////////////////////////////////////////////////////////////////////////
/// this method will update the filters if the selected port ID has changed
////////////////////////////////////////////////////////////////////////////////
void MIDIControllerFilterForm::selectedPortIDsChanged(PortIDVector) {
  updateFilters();
}

////////////////////////////////////////////////////////////////////////////////
/// updates all the filters
////////////////////////////////////////////////////////////////////////////////
void MIDIControllerFilterForm::updateFilters() {
  const auto &portID = portSelectionForm->selectedPortID();
  const auto &filterData = device->midiPortFilter(portID, currentFilterID());

  for (auto row = 0; row < ui->tableWidget->rowCount(); ++row) {
    auto *const comboBox = qobject_cast<QComboBox *>(
        ui->tableWidget->cellWidget(row, FilterCtlrCols::ControllerID));
    Q_ASSERT(comboBox);

    const auto &controllerFilter = filterData.controllerFilters().at(row);

    comboBox->setCurrentIndex(controllerFilter.controllerID);
  }

  tableListener->updateWidgets(
      bind(&MIDIControllerFilterForm::stateForCell, this, _1, _2));
}

////////////////////////////////////////////////////////////////////////////////
/// this method sends all the pending updates
////////////////////////////////////////////////////////////////////////////////
void MIDIControllerFilterForm::sendUpdate() {
  updateMutex.lock();
  QHashIterator<int, FilterIDEnum> i(updateList);
  while (i.hasNext()) {
    i.next();

    const auto &filterData = device->midiPortFilter(i.key(), i.value());
    device->send<SetMIDIPortFilterCommand>(filterData);
  }

  updateList.clear();
  updateMutex.unlock();
  sendTimer->stop();
}

////////////////////////////////////////////////////////////////////////////////
/// this method is called when the state of a cell has changed
////////////////////////////////////////////////////////////////////////////////
void MIDIControllerFilterForm::cellStateChange(int row, int col,
                                               BlockState::Enum state) {
  const auto &portID = portSelectionForm->selectedPortID();
  const auto &chBitmap = colChBitmapMap.at(col);
  auto &filterData = device->midiPortFilter(portID, currentFilterID());

  Q_ASSERT((row >= 0) && (row < (int) filterData.controllerFilters().size()));

  auto &controllerFilter = filterData.controllerFilter_at(row);
  controllerFilter.channelBitmap.set(chBitmap, (state == BlockState::Full));

  addToUpdateList(portID);
}

////////////////////////////////////////////////////////////////////////////////
/// a helper method that returns the state for the cell at location (row, col)
////////////////////////////////////////////////////////////////////////////////
BlockState::Enum MIDIControllerFilterForm::stateForCell(int row,
                                                        int col) const {
  const auto &portID = portSelectionForm->selectedPortID();
  const auto &filterData = device->midiPortFilter(portID, currentFilterID());

  Q_ASSERT((row >= 0) && (row < (int) filterData.controllerFilters().size()));
  const auto &controllerFilter = filterData.controllerFilter_at(row);

  return (controllerFilter.channelBitmap[colChBitmapMap.at(col)])
             ? BlockState::Full
             : BlockState::Empty;
}

////////////////////////////////////////////////////////////////////////////////
/// a helper method that returns the current filter ID
////////////////////////////////////////////////////////////////////////////////
GeneSysLib::FilterIDEnum MIDIControllerFilterForm::currentFilterID() const {
  return (ui->filterTypeComboBox->currentIndex() == 0) ? FilterID::InputFilter
                                                       : FilterID::OutputFilter;
}

////////////////////////////////////////////////////////////////////////////////
/// this method gets called when the UI needs to be refreshed
////////////////////////////////////////////////////////////////////////////////
void MIDIControllerFilterForm::refreshWidget() {
  portSelectionForm->updateTree();
  this->updateFilters();
}

////////////////////////////////////////////////////////////////////////////////
/// this method is used to add continuous controller combo box
////////////////////////////////////////////////////////////////////////////////
void MIDIControllerFilterForm::addCCComboBox(int row) {
  const auto &getOptions = bind(CCList);
  const auto &getFunc =
      bind(&MIDIControllerFilterForm::controllerIndexForRow, this, row, _1);
  const auto &setFunc = bind(
      &MIDIControllerFilterForm::setControllerIndexForRow, this, row, _1, _2);
  auto *const cbox = new MyComboBox(getOptions, getFunc, setFunc);
  ui->tableWidget->setCellWidget(row, FilterCtlrCols::ControllerID, cbox);
}

////////////////////////////////////////////////////////////////////////////////
/// this method is used to add an empty label to the table
////////////////////////////////////////////////////////////////////////////////
void MIDIControllerFilterForm::addEmptyLabel(int row, int col) {
  auto *const label = new QLabel();
  label->setScaledContents(true);
  label->setProperty(kBlockState, BlockState::Empty);
  ui->tableWidget->setCellWidget(row, col, label);
}

////////////////////////////////////////////////////////////////////////////////
/// add the port ID to the list of pending updates
////////////////////////////////////////////////////////////////////////////////
void MIDIControllerFilterForm::addToUpdateList(Word portID) {
  sendTimer->stop();
  updateMutex.lock();
  updateList[portID] = currentFilterID();
  updateMutex.unlock();
  sendTimer->start(kBatchTime);
}

////////////////////////////////////////////////////////////////////////////////
/// this method is used to return the current controller index for a given row
////////////////////////////////////////////////////////////////////////////////
int MIDIControllerFilterForm::controllerIndexForRow(int row,
                                                    QComboBox *) const {
  const auto &portID = portSelectionForm->selectedPortID();
  const auto &filterData  = device->midiPortFilter(portID, currentFilterID());
  const auto &controllerFilter = filterData.controllerFilter_at(row);
  return controllerFilter.controllerID;
}

////////////////////////////////////////////////////////////////////////////////
/// this method is used to set the controller index when a combo box is selected
////////////////////////////////////////////////////////////////////////////////
void MIDIControllerFilterForm::setControllerIndexForRow(int row, QComboBox *,
                                                        int selection) {
  const auto &portID = portSelectionForm->selectedPortID();
  auto &filterData = device->midiPortFilter(portID, currentFilterID());
  auto &controllerFilter = filterData.controllerFilter_at(row);
  controllerFilter.controllerID = selection;
  addToUpdateList(portID);
}
