/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "ICRestoreDialog.h"
#include "ui_ICRestoreDialog.h"
#include "DeviceInfo.h"
#include "MainWindow.h"

#include <QFileDialog>
#include <QDesktopServices>
#include <QMessageBox>
#include <QDebug>


ICRestoreDialog::ICRestoreDialog(DeviceInfoPtr device, QWidget *parent) :
  currentDevice(device),
  QDialog(parent),
  ui(new Ui::ICRestoreDialog)
{
  qDebug() << "ICRestoreDialog is open";
  ui->setupUi(this);
  listModel = new QStandardItemModel();
  fileName = "";
  loadFiles();
  setModal(true);
}

ICRestoreDialog::~ICRestoreDialog()
{
  delete ui;
}

void ICRestoreDialog::loadFiles() {
  QDir::root().mkpath(QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/presets");
  QDir presetsDir = QDir(QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/presets");

  qDebug() << "Preset Device:" << MainWindow::extensionForPID(currentDevice->getPID());

  presetsDir.setNameFilters(QStringList()<<("*" + MainWindow::extensionForPID(currentDevice->getPID())));

  QStringList fileList = presetsDir.entryList();

  foreach (QString file, fileList) {
    ui->listFiles->addItem(file);
  }

  connect(ui->listFiles,SIGNAL(itemSelectionChanged()),
          this, SLOT(handleSelectionChanged()));
}

void ICRestoreDialog::handleSelectionChanged() {
  if(ui->listFiles->selectedItems().isEmpty()) {
    ui->textEditDescription->setText("");
  }
  else {
    loadDescription(ui->listFiles->selectedItems().first()->text());
  }
}

void ICRestoreDialog::loadDescription(const QString index) {
  fileName = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/presets/" + index;

  QFile file(fileName);
  if (!file.open(QFile::ReadOnly)) {
    QMessageBox::warning(this, tr("Read Failed"),
                         tr("Cannot read file %1:\n%2.").arg(fileName)
                         .arg(file.errorString()));
    return;
  }
  else {
    QByteArray qData = file.readAll();
    file.close();

    bool valid = (qData.size() > 22);

    if (valid) {
      int descSize = qData.at(5);
      QString description = QString(qData.mid(6,descSize));

      ui->textEditDescription->setText(description);
    }
  }
}

void ICRestoreDialog::accept() {
  qDebug() << "Open Preset Filename:" << fileName;
  if (fileName == "") {
    QMessageBox::warning(this, "Open Preset", "You need to select a preset to open");
    return;
  }
  QDialog::accept();
}

void ICRestoreDialog::reject() {
  QDialog::reject();
}

QString ICRestoreDialog::getFileName() {
  return fileName;
}
