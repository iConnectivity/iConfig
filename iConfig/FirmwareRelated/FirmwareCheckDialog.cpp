/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "FirmwareCheckDialog.h"
#include "ui_FirmwareCheckDialog.h"

#include <QSettings>
#include <QDateTime>

FirmwareCheckDialog::FirmwareCheckDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FirmwareCheckDialog)
{
  ui->setupUi(this);
  QSettings settings(QCoreApplication::organizationName(),
                     QCoreApplication::applicationName());
  settings.beginGroup("firmwareCheck");
  int freq = settings.value("frequency", 0).toInt();

  switch(freq) {
  default:
  case 0:
    ui->radioEveryLoad->setChecked(true);
    ui->radioEveryLoad->setFocus();
    break;
  case 1:
    ui->radioEveryWeek->setChecked(true);
    ui->radioEveryWeek->setFocus();
    break;
  case 2:
    ui->radioEveryMonth->setChecked(true);
    ui->radioEveryMonth->setFocus();
    break;
  case 3:
    ui->radioNever->setChecked(true);
    ui->radioNever->setFocus();
    break;
  }
  settings.endGroup();

  connect(ui->radioEveryLoad, SIGNAL(toggled(bool)),this, SLOT(checkChanged(bool)));
  connect(ui->radioEveryWeek, SIGNAL(toggled(bool)),this, SLOT(checkChanged(bool)));
  connect(ui->radioEveryMonth, SIGNAL(toggled(bool)),this, SLOT(checkChanged(bool)));
  connect(ui->radioNever, SIGNAL(toggled(bool)),this, SLOT(checkChanged(bool)));
}

void FirmwareCheckDialog::checkChanged(bool checked) {
  QRadioButton* obj = dynamic_cast<QRadioButton*>(sender());

  QSettings settings(QCoreApplication::organizationName(),
                     QCoreApplication::applicationName());
  settings.beginGroup("firmwareCheck");
  if (checked) {
    if (obj == ui->radioEveryLoad) {
      settings.setValue("frequency", 0);
    }
    else if (obj == ui->radioEveryWeek) {
      settings.setValue("frequency", 1);
    }
    else if (obj == ui->radioEveryMonth) {
      settings.setValue("frequency", 2);
    }
    else if (obj == ui->radioNever) {
      settings.setValue("frequency", 3);
    }
    settings.setValue("lastCheck", QDateTime::currentDateTime().toTime_t());
  }
  settings.endGroup();

}

FirmwareCheckDialog::~FirmwareCheckDialog()
{
  delete ui;
}
