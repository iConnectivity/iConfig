/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "DeviceInformationDialog.h"
#include "ui_DeviceInformationDialog.h"

DeviceInformationDialog::DeviceInformationDialog(DeviceInfoPtr device, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DeviceInformationDialog)
{
  tie(deviceID, transID) = device->getInfo();

  QSettings settings(QCoreApplication::organizationName(),
                     QCoreApplication::applicationName());
  settings.beginGroup("informationScreen");
  bool show = settings.value("show" + QString::number(deviceID.pid()), true).toBool();
  ui->setupUi(this);
  if (!show) {
    ui->checkBox->setChecked(true);
  }
  else {
    ui->checkBox->setChecked(false);
  }
  settings.setValue("show" + QString::number(deviceID.pid()), QVariant::fromValue(!(ui->checkBox->checkState() == Qt::Checked)));
  settings.endGroup();

  ui->textBrowser->setSource(QUrl("qrc:/html/html/ica4.html"));
  ui->textBrowser->setOpenExternalLinks(true);
}

DeviceInformationDialog::~DeviceInformationDialog()
{
  delete ui;
}

void DeviceInformationDialog::checkBoxChecked(bool state)
{
  QSettings settings(QCoreApplication::organizationName(),
                     QCoreApplication::applicationName());
  settings.beginGroup("informationScreen");
  settings.setValue("show" + QString::number(deviceID.pid()), QVariant::fromValue(!state));
  settings.endGroup();
}
