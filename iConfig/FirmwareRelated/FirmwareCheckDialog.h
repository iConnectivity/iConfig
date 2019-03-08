/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef FIRMWARECHECKDIALOG_H
#define FIRMWARECHECKDIALOG_H

#include <QDialog>

namespace Ui {
class FirmwareCheckDialog;
}

class FirmwareCheckDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FirmwareCheckDialog(QWidget *parent = 0);
  ~FirmwareCheckDialog();
public slots:
  void checkChanged(bool checked);

private:
  Ui::FirmwareCheckDialog *ui;
};

#endif // FIRMWARECHECKDIALOG_H
