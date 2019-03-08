/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "ICSaveDialog.h"
#include "ui_ICSaveDialog.h"
#include "DeviceInfo.h"
#include "MainWindow.h"
#include "SaveRestore.h"

#include <QFileDialog>
#include <QDesktopServices>
#include <QMessageBox>
//Disable Audio Patchbay related control for MDID devices, zx, 2017-04-12
#include "Device.h"
#include "DeviceID.h"
#include "DevicePID.h"
#include <QDebug>

using namespace GeneSysLib;

ICSaveDialog::ICSaveDialog(DeviceInfoPtr device, QWidget *parent) :
  currentDevice(device),
  QDialog(parent),
  ui(new Ui::ICSaveDialog)
{
  ui->setupUi(this);
  connect(ui->checkAll,SIGNAL(toggled(bool)), this, SLOT(checkToggled(bool)));
  connect(ui->checkAnalog,SIGNAL(toggled(bool)), this, SLOT(checkToggled(bool)));
//  connect(ui->checkAnalogOutput,SIGNAL(toggled(bool)), this, SLOT(checkToggled(bool)));
  connect(ui->checkAudio,SIGNAL(toggled(bool)), this, SLOT(checkToggled(bool)));
  connect(ui->checkAudioConnections,SIGNAL(toggled(bool)), this, SLOT(checkToggled(bool)));
  connect(ui->checkAudioInfo,SIGNAL(toggled(bool)), this, SLOT(checkToggled(bool)));
  connect(ui->checkAudioMixer,SIGNAL(toggled(bool)), this, SLOT(checkToggled(bool)));
  connect(ui->checkDeviceInfo,SIGNAL(toggled(bool)), this, SLOT(checkToggled(bool)));
  connect(ui->checkMidi,SIGNAL(toggled(bool)), this, SLOT(checkToggled(bool)));
  connect(ui->checkMidiRemap,SIGNAL(toggled(bool)), this, SLOT(checkToggled(bool)));
  connect(ui->checkMidiFilters,SIGNAL(toggled(bool)), this, SLOT(checkToggled(bool)));
  connect(ui->checkMidiInfo,SIGNAL(toggled(bool)), this, SLOT(checkToggled(bool)));
  connect(ui->checkMidiPortRouting,SIGNAL(toggled(bool)), this, SLOT(checkToggled(bool)));

  ui->textEditFileName->setValidator(new QRegExpValidator(QRegExp("[A-Za-z0-9_][A-Za-z0-9_.- ]+"), this));
  ui->textEditFileName->setFocus();

  //Disable Audio Patchbay related control for MDID devices, zx, 2017-04-11
  m_MIDIOnly = false;
  if(currentDevice != NULL) {
       //DevicePID::Enum pID = device->getPID();
       DevicePID::Enum pID = (DevicePID::Enum)currentDevice->getPID();
       qDebug() << pID;
       if(pID == DevicePID::MIO10 ||pID == DevicePID::MIO || pID == DevicePID::MIO2 || pID == DevicePID::MIO4 ||
          pID == DevicePID::iConnect2Plus || pID == DevicePID::iConnect4Plus) {
         ui->checkAnalog->setEnabled(false);
         ui->checkAudio->setEnabled(false);
         ui->checkAudioConnections->setEnabled(false);
         ui->checkAudioInfo->setEnabled(false);
         ui->checkAudioMixer->setEnabled(false);
         m_MIDIOnly = true;
       }
  }
}

ICSaveDialog::~ICSaveDialog()
{
  delete ui;
}

void ICSaveDialog::blockChecks(bool value) {
  ui->checkAll->blockSignals(value);
  //Disable Audio Patchbay related control for MDID devices, zx, 2017-04-11
  if(m_MIDIOnly == false)  {
    ui->checkAnalog->blockSignals(value);
//  ui->checkAnalogOutput->blockSignals(value);
    ui->checkAudio->blockSignals(value);
    ui->checkAudioConnections->blockSignals(value);
    ui->checkAudioInfo->blockSignals(value);
    ui->checkAudioMixer->blockSignals(value);
  }
  ui->checkDeviceInfo->blockSignals(value);
  ui->checkMidi->blockSignals(value);
  ui->checkMidiRemap->blockSignals(value);
  ui->checkMidiFilters->blockSignals(value);
  ui->checkMidiInfo->blockSignals(value);
  ui->checkMidiPortRouting->blockSignals(value);
}

void ICSaveDialog::checkToggled(bool value) { // handle button pre-reqs
  QCheckBox* obj = dynamic_cast<QCheckBox*>(sender());

  blockChecks(true);
  if (obj == ui->checkAll) {
    //Disable Audio Patchbay related control for MDID devices, zx, 2017-04-11
    if(m_MIDIOnly == false)  {
      ui->checkAnalog->setChecked(value);
    //ui->checkAnalogOutput->setChecked(value);
      ui->checkAudio->setChecked(value);
      ui->checkAudioConnections->setChecked(value);
      ui->checkAudioInfo->setChecked(value);
      ui->checkAudioMixer->setChecked(value);
    }
    ui->checkDeviceInfo->setChecked(value);
    ui->checkMidi->setChecked(value);
    ui->checkMidiRemap->setChecked(value);
    ui->checkMidiFilters->setChecked(value);
    ui->checkMidiInfo->setChecked(value);
    ui->checkMidiPortRouting->setChecked(value);
  }
  else if (obj == ui->checkMidi) {
    ui->checkMidiRemap->setChecked(value);
    ui->checkMidiFilters->setChecked(value);
    ui->checkMidiInfo->setChecked(value);
    ui->checkMidiPortRouting->setChecked(value);
  }
  else if (obj == ui->checkAudio) {
    //Disable Audio Patchbay related control for MDID devices, zx, 2017-04-11
    if(m_MIDIOnly == false)  {
      ui->checkAnalog->setChecked(value);
    //ui->checkAnalogOutput->setChecked(value);
      ui->checkAudioConnections->setChecked(value);
      ui->checkAudioInfo->setChecked(value);
      ui->checkAudioMixer->setChecked(value);
     }
  }
  else if (!value) {
    ui->checkAll->setChecked(false);
    if (obj == ui->checkMidiRemap ||
        obj == ui->checkMidiFilters ||
        obj == ui->checkMidiInfo ||
        obj == ui->checkMidiPortRouting) {
      ui->checkMidi->setChecked(false);
      if (obj == ui->checkMidiInfo) {
        ui->checkMidiRemap->setChecked(false);
        ui->checkMidiFilters->setChecked(false);
        ui->checkMidiPortRouting->setChecked(false);
      }
    }
    else if ((obj == ui->checkAudioConnections ||
        obj == ui->checkAudioInfo ||
        obj == ui->checkAudioMixer ||
        obj == ui->checkAnalog) &&
        m_MIDIOnly == false) { //Disable Audio Patchbay related control for MDID devices, zx, 2017-04-11
      ui->checkAudio->setChecked(false);
      if (obj == ui->checkAudioInfo) {
        ui->checkAudioMixer->setChecked(false);
        ui->checkAudioConnections->setChecked(false);
      }
    }
  }
  else {
    if ((obj == ui->checkAudioConnections ||
        obj == ui->checkAudioMixer) && m_MIDIOnly == false) { //Disable Audio Patchbay related control for MDID devices, zx, 2017-04-11
      ui->checkAudioInfo->setChecked(true);
    }
    else if (obj == ui->checkMidiRemap ||
        obj == ui->checkMidiFilters ||
        obj == ui->checkMidiPortRouting) {
      ui->checkMidiInfo->setChecked(true);
    }
  }

  blockChecks(false);
}

std::set<Command::Enum> ICSaveDialog::getPreRebootCommands() {
  std::set<Command::Enum> toReturn;
  if (ui->checkAudioInfo) {
    toReturn.insert(Command::RetAudioInfo);
    toReturn.insert(Command::RetAudioCfgInfo);
    toReturn.insert(Command::RetAudioPortInfo);
    toReturn.insert(Command::RetAudioPortCfgInfo);
    toReturn.insert(Command::RetAudioClockInfo);

    toReturn.insert(Command::RetAudioGlobalParm);
    toReturn.insert(Command::RetAudioPortParm);
    toReturn.insert(Command::RetAudioDeviceParm);
    toReturn.insert(Command::RetAudioClockParm);

    toReturn.insert(Command::RetMixerParm);
    toReturn.insert(Command::RetMixerPortParm);
  }
  return toReturn;
}

std::set<Command::Enum> ICSaveDialog::getPostRebootCommands() {
  std::set<Command::Enum> toReturn;

  if (ui->checkDeviceInfo) {
    toReturn.insert(Command::RetInfo);
    toReturn.insert(Command::RetEthernetPortInfo);
  }
  if (ui->checkAudioConnections) {
    toReturn.insert(Command::RetMixerInputParm);
    toReturn.insert(Command::RetMixerOutputParm);
    toReturn.insert(Command::RetAudioPatchbayParm);
  }
  if (ui->checkAudioMixer) {
    toReturn.insert(Command::RetMixerInputControl);
    toReturn.insert(Command::RetMixerInputControlValue);
    toReturn.insert(Command::RetMixerOutputControl);
    toReturn.insert(Command::RetMixerOutputControlValue);
  }
  if (ui->checkAnalog) {
    toReturn.insert(Command::RetAudioControlDetail);
    toReturn.insert(Command::RetAudioControlDetailValue);
  }
  if (ui->checkMidiRemap) {
    toReturn.insert(Command::RetMIDIPortRemap);
  }
  if (ui->checkMidiFilters) {
    toReturn.insert(Command::RetMIDIPortFilter);
  }
  if (ui->checkMidiInfo) {
    toReturn.insert(Command::RetMIDIInfo);
    toReturn.insert(Command::RetMIDIPortInfo);
    toReturn.insert(Command::RetMIDIPortDetail);
  }
  if (ui->checkMidiPortRouting) {
    toReturn.insert(Command::RetMIDIPortRoute);
  }

  return toReturn;
}

Bytes midiHeader() {
  static const unsigned char arr[] = {
    'M','T','h','d',
    0x00,0x00,0x00,0x06,
    0x00,0x00,
    0x00,0x01,
    0x00,0x60,
    'M','T','r','k'
  };
  Bytes midi(arr, arr + sizeof(arr) / sizeof(arr[0]) );

  return midi;
}

void ICSaveDialog::buttonExportAsMidi_triggered() {
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), "untitled.mid", tr("Midi File (*.mid)"));
  QString fileName2;

  Bytes midiH = midiHeader();

  std::set<Command::Enum> preRebootCommands = getPreRebootCommands();
  std::set<Command::Enum> postRebootCommands = getPostRebootCommands();

  if (preRebootCommands.empty() && postRebootCommands.empty()) {
    QMessageBox::warning(this, tr("Save As"), tr("You need to select data to save!"));
    return;
  }

  if (!fileName.isEmpty()) {
    Bytes serialized;
    if (!preRebootCommands.empty()) {
      serialized = currentDevice->serialize2midi(preRebootCommands, true);
      fileName2 = fileName;

      fileName.replace(".mid","_part1_reboot.mid");
      fileName2.replace(".mid","_part2.mid");
    }
    else {
      serialized = currentDevice->serialize2midi(postRebootCommands, false);
    }

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
      QMessageBox::warning(this, tr("Save As"),
                           tr("Cannot write file %1:\n%2.").arg(fileName)
                           .arg(file.errorString()));
      return;
    }

    QFile file2(fileName2);
    if (file2.exists()) {
      file2.remove();
    }
    char* data = (char*)serialized.data();
    char* midiHeader = (char*) midiH.data();
    int sizeOfData = serialized.size();
    char* sizeOfDataP = (char*)&sizeOfData;
    file.write(midiHeader, midiH.size());
    file.write(&sizeOfDataP[3], 1);
    file.write(&sizeOfDataP[2], 1);
    file.write(&sizeOfDataP[1], 1);
    file.write(&sizeOfDataP[0], 1);
    file.write(data, serialized.size());
    file.close();

    if (!preRebootCommands.empty()) {
      QFile file(fileName2);

      if (!file.open(QFile::WriteOnly)) {
        QMessageBox::warning(this, tr("Save As"),
                             tr("An unexpected error occured writing %1:\n%2.").arg(fileName + ".aux")
                             .arg(file.errorString()));
        return;
      }
      Bytes serialized = currentDevice->serialize2midi(postRebootCommands, false);
      char* data = (char*)serialized.data();
      int sizeOfData = serialized.size();
      char* sizeOfDataP = (char*)&sizeOfData;
      file.write(midiHeader, midiH.size());
      file.write(&sizeOfDataP[3], 1);
      file.write(&sizeOfDataP[2], 1);
      file.write(&sizeOfDataP[1], 1);
      file.write(&sizeOfDataP[0], 1);
      file.write(data, serialized.size());
      file.close();
    }
  }
  else {
    QMessageBox::warning(this, tr("Save As"),
                         tr("Need a file name!"));
    return;
  }

  QDialog::accept();
}

void ICSaveDialog::accept() {
  QDir::root().mkpath(QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/presets");

  QString fileName = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/presets/" +
                     ui->textEditFileName->text() + MainWindow::extensionForPID(currentDevice->getPID());

  std::set<Command::Enum> preRebootCommands = getPreRebootCommands();
  std::set<Command::Enum> postRebootCommands = getPostRebootCommands();

  if (preRebootCommands.empty() && postRebootCommands.empty()) {
    QMessageBox::warning(this, tr("Save As"), tr("You need to select data to save!"));
    return;
  }

  if (!fileName.isEmpty()) {
    QFile file(fileName);

    if (!file.open(QFile::WriteOnly)) {
      QMessageBox::warning(this, tr("Save As"),
                           tr("Cannot write file %1:\n%2.").arg(fileName)
                           .arg(file.errorString()));
      return;
    }

    QFile file2(fileName + ".aux");
    if (file2.exists()) {
      file2.remove();
    }

    Bytes serialized;
    if (!preRebootCommands.empty()) {
      serialized = currentDevice->serialize2(preRebootCommands, ui->textEditDescription->text().toAscii());
    }
    else {
      serialized = currentDevice->serialize2(postRebootCommands, ui->textEditDescription->text().toAscii());
    }
    char* data = (char*)serialized.data();
    file.write(data, serialized.size());
    file.close();

    if (!preRebootCommands.empty()) {
      QFile file(fileName + ".aux");

      if (!file.open(QFile::WriteOnly)) {
        QMessageBox::warning(this, tr("Save As"),
                             tr("An unexpected error occured writing %1:\n%2.").arg(fileName + ".aux")
                             .arg(file.errorString()));
        return;
      }
      Bytes serialized = currentDevice->serialize2(postRebootCommands);
      char* data = (char*)serialized.data();
      file.write(data, serialized.size());
      file.close();
    }
  }
  else {
    QMessageBox::warning(this, tr("Save As"),
                         tr("Need a file name!"));
    return;
  }

  QDialog::accept();
}

void ICSaveDialog::reject()
{
  printf("canceled preset save\n");
  QDialog::reject();
}
