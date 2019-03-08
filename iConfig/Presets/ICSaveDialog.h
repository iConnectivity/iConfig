/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef ICSAVEDIALOG_H
#define ICSAVEDIALOG_H

#include <QDialog>
#include "DeviceInfo.h"

using namespace GeneSysLib;

namespace Ui {
class ICSaveDialog;
}

class ICSaveDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ICSaveDialog(DeviceInfoPtr device, QWidget *parent = 0);
  ~ICSaveDialog();

  void accept();
  void reject();
public slots:
  void checkToggled(bool value);
  void buttonExportAsMidi_triggered();
protected:
  void blockChecks(bool value);
private:
  Ui::ICSaveDialog *ui;

  DeviceInfoPtr currentDevice;
  std::set<Command::Enum> getPreRebootCommands();
  std::set<Command::Enum> getPostRebootCommands();

  //Disable Audio Patchbay related control for MDID devices, zx, 2017-04-10
  bool m_MIDIOnly;
};

#endif // ICSAVEDIALOG_H
