/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef ICRESTOREDIALOG_H
#define ICRESTOREDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include <QListView>
#include "DeviceInfo.h"

namespace Ui {
class ICRestoreDialog;
}

class ICRestoreDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ICRestoreDialog(DeviceInfoPtr device, QWidget *parent = 0);
  ~ICRestoreDialog();

  QString getFileName();
protected:
  void accept();
  void reject();
protected slots:
  void handleSelectionChanged();
private:
  Ui::ICRestoreDialog *ui;
  void loadFiles();
  void loadDescription(const QString index);
  DeviceInfoPtr currentDevice;
  QStandardItemModel* listModel;
  QString fileName;
};

#endif // ICRESTOREDIALOG_H
