/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "MIDIChannelRemapForm.h"
#include "ui_MIDIChannelRemapForm.h"

#include "ACK.h"
#include "MyAlgorithms.h"
#include "MIDIInfo.h"
#include "MIDIPortInfo.h"

#include <QLineEdit>

#ifndef Q_MOC_RUN
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#endif

using namespace boost;
using namespace MyAlgorithms;
using namespace GeneSysLib;

namespace RemapStatusRow {
typedef enum Enum {
  PitchBendEvents = 1,
  ChannelPressureEvents,
  ProgramChangeEvents,
  ControlChangeEvents,
  PolyKeyPressureEvents,
  NoteEvents
} Enum;
}

const QString MIDIChannelRemapForm::DefaultWindowTitle = tr("MIDI Channel Remap");
const float MIDIChannelRemapForm::kBatchTime = 1500;

QList<CmdEnum> MIDIChannelRemapForm::Query() {
  QList<CmdEnum> query;
  query << Command::RetMIDIInfo << Command::RetMIDIPortInfo
        << Command::RetMIDIPortRemap;
  return query;
}

MIDIChannelRemapForm::MIDIChannelRemapForm(CommPtr _comm, DeviceInfoPtr _device,
                                           QWidget *_parent)
    : RefreshObject(_parent), ui(new Ui::MIDIChannelRemapForm) {
  comm = _comm;
  device = _device;
  Q_ASSERT(comm);
  Q_ASSERT(device);
  ui->setupUi(this);

  tableListener = new TableListener(ui->tableWidget, this);
  tableListener->addIgnoreRow(0);
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

  connect(ui->remapTypeComboBox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(updateChannelRemap()));

  sendTimer = new QTimer(this);
  connect(sendTimer, SIGNAL(timeout()), this, SLOT(sendUpdate()));

  setWindowTitle(MIDIChannelRemapForm::DefaultWindowTitle);

  // Setup Table
  QStringList vertHeaderList;
  vertHeaderList << tr("Remap to Channel") << tr("Pitch Bend")
                 << tr("Channel Pressure") << tr("Program Change")
                 << tr("Control Change") << tr("Poly Key Pressure")
                 << tr("Note On/Off");
  ui->tableWidget->setRowCount(vertHeaderList.count());
  ui->tableWidget->setVerticalHeaderLabels(vertHeaderList);
  ui->tableWidget->verticalHeader()->setDefaultSectionSize(25);

  QStringList horzHeaderList;
  for (auto i = 1; i <= 16; ++i) {
    horzHeaderList << QString::number(i);
  }
  ui->tableWidget->setColumnCount(horzHeaderList.count());
  ui->tableWidget->setHorizontalHeaderLabels(horzHeaderList);
  auto *const horzHeader = ui->tableWidget->horizontalHeader();
  Q_ASSERT(horzHeader);
  horzHeader->setResizeMode(QHeaderView::Stretch);

  tableListener->addCornerLabel("Channel");

  // Add the labels
  for (auto row = 1; row < ui->tableWidget->rowCount(); ++row) {
    for (auto col = 0; col < ui->tableWidget->columnCount(); ++col) {
      auto *const label = new QLabel();
      label->setScaledContents(true);
      label->setProperty(kBlockState, BlockState::Empty);
      ui->tableWidget->setCellWidget(row, col, label);
    }
  }

  lineEditSignalMapper = new QSignalMapper(this);

  // Add line edit
  for (auto col = 0; col < ui->tableWidget->columnCount(); ++col) {
    auto *const lineEdit = new QLineEdit();
    lineEdit->setValidator(new QIntValidator(1, 16));
    lineEdit->setText(QString::number(16));

    lineEditSignalMapper->setMapping(lineEdit, col);
    connect(lineEdit, SIGNAL(editingFinished()), lineEditSignalMapper,
            SLOT(map()));

    ui->tableWidget->setCellWidget(0, col, lineEdit);
  }
  connect(lineEditSignalMapper, SIGNAL(mapped(int)), this,
          SLOT(lineEditChanged(int)));

  updateChannelRemap();
}

MIDIChannelRemapForm::~MIDIChannelRemapForm() {
  int count = 0;
  updateMutex.lock();
  count = updateList.count();
  updateMutex.unlock();

  if (count > 0) {
    sendUpdate();
  }
}

void MIDIChannelRemapForm::selectedPortIDsChanged(PortIDVector) {
  updateChannelRemap();
}

void MIDIChannelRemapForm::updateChannelRemap() {
  const auto &portID = portSelectionForm->selectedPortID();
  auto &remapMap = device->midiPortRemap(portID, currentRemapID());

  for (auto col = 0; col < ui->tableWidget->columnCount(); ++col) {
    auto *const lineEdit =
        qobject_cast<QLineEdit *>(ui->tableWidget->cellWidget(0, col));
    Q_ASSERT(lineEdit);

    const auto &remapStatus = remapMap.remapStatus_at(col);
    lineEdit->setText(QString::number(remapStatus.channelNumber + 1));
  }

  tableListener->updateWidgets(
      bind(&MIDIChannelRemapForm::stateForCell, this, _1, _2));
}

void MIDIChannelRemapForm::sendUpdate() {
  updateMutex.lock();
  QHashIterator<RemapTypeEnum, Word> i(updateList);
  while (i.hasNext()) {
    i.next();
    const auto &remapID = i.key();
    const auto &portID = i.value();
    const auto &remapData = device->midiPortRemap(portID, remapID);

    device->send<SetMIDIPortRemapCommand>(remapData);
  }
  updateList.clear();
  updateMutex.unlock();

  sendTimer->stop();
}

void MIDIChannelRemapForm::cellStateChange(int row, int col,
                                           BlockState::Enum state) {
  const auto &portID = portSelectionForm->selectedPortID();
  auto &remapMap = device->midiPortRemap(portID, currentRemapID());
  auto &remapStatus = remapMap.remapStatus_at(col);

  setRemapStatusBit(remapStatus, row, (state == BlockState::Full));

  addToUpdateList(portID);
}

void MIDIChannelRemapForm::lineEditChanged(int col) {
  auto *const lineEdit =
      qobject_cast<QLineEdit *>(ui->tableWidget->cellWidget(0, col));
  Q_ASSERT(lineEdit);

  const auto &portID = portSelectionForm->selectedPortID();
  auto &remapMap = device->midiPortRemap(portID, currentRemapID());
  auto &remapStatus = remapMap.remapStatus_at(col);

  remapStatus.channelNumber = (uint16_t)(lineEdit->text().toInt() - 1) & 0x0F;

  addToUpdateList(portID);
}

BlockState::Enum MIDIChannelRemapForm::stateForCell(int row, int col) const {
  const auto &remapID =
      (ui->remapTypeComboBox->currentIndex() == 0) ? RemapID::InputRemap
                                                   : RemapID::OutputRemap;
  const auto &portID = portSelectionForm->selectedPortID();
  const auto &remapMap = device->midiPortRemap(portID, remapID);
  const auto &remapStatus = remapMap.remapStatus_at(col);

  return ((rowToRemapStatus(remapStatus, row)) ? (BlockState::Full)
                                               : (BlockState::Empty));
}

bool MIDIChannelRemapForm::rowToRemapStatus(
    const MIDIPortRemap::RemapStatus &remapStatus, int row) const {
  bool statusSet = false;
  switch (row) {
    case RemapStatusRow::PitchBendEvents:
      statusSet = remapStatus.pitchBendEvents;
      break;
    case RemapStatusRow::ChannelPressureEvents:
      statusSet = remapStatus.channelPressureEvents;
      break;
    case RemapStatusRow::ProgramChangeEvents:
      statusSet = remapStatus.programChangeEvents;
      break;
    case RemapStatusRow::ControlChangeEvents:
      statusSet = remapStatus.controlChangeEvents;
      break;
    case RemapStatusRow::PolyKeyPressureEvents:
      statusSet = remapStatus.polyKeyPressureEvents;
      break;
    case RemapStatusRow::NoteEvents:
      statusSet = remapStatus.noteEvents;
      break;
    default:
      break;
  }
  return statusSet;
}

void MIDIChannelRemapForm::setRemapStatusBit(
    MIDIPortRemap::RemapStatus &remapStatus, int row, bool value) const {
  switch (row) {
    case RemapStatusRow::PitchBendEvents:
      remapStatus.pitchBendEvents = value;
      break;
    case RemapStatusRow::ChannelPressureEvents:
      remapStatus.channelPressureEvents = value;
      break;
    case RemapStatusRow::ProgramChangeEvents:
      remapStatus.programChangeEvents = value;
      break;
    case RemapStatusRow::ControlChangeEvents:
      remapStatus.controlChangeEvents = value;
      break;
    case RemapStatusRow::PolyKeyPressureEvents:
      remapStatus.polyKeyPressureEvents = value;
      break;
    case RemapStatusRow::NoteEvents:
      remapStatus.noteEvents = value;
      break;
    default:
      break;
  }
}

RemapTypeEnum MIDIChannelRemapForm::currentRemapID() const {
  return (ui->remapTypeComboBox->currentIndex() == 0) ? RemapID::InputRemap
                                                      : RemapID::OutputRemap;
}

void MIDIChannelRemapForm::refreshWidget() {
  portSelectionForm->updateTree();
  this->updateChannelRemap();
}

void MIDIChannelRemapForm::addToUpdateList(Word portID) {
  sendTimer->stop();
  updateMutex.lock();
  updateList[currentRemapID()] = portID;
  updateMutex.unlock();
  sendTimer->start(kBatchTime);
}
