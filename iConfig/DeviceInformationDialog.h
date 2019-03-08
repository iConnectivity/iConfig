/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef DEVICEINFORMATIONDIALOG_H
#define DEVICEINFORMATIONDIALOG_H

#include <QDialog>
#include <QSettings>
#include "DeviceInfo.h"

namespace Ui {
class DeviceInformationDialog;
}

class DeviceInformationDialog : public QDialog
{
  Q_OBJECT

public:
  explicit DeviceInformationDialog(DeviceInfoPtr device, QWidget *parent = 0);
  ~DeviceInformationDialog();
private slots:
  void checkBoxChecked(bool state);
private:
  Ui::DeviceInformationDialog *ui;

  GeneSysLib::DeviceID deviceID;
  Word transID;

};

#endif // DEVICEINFORMATIONDIALOG_H
