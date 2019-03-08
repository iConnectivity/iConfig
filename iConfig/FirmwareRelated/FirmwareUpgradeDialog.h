/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef FIRMWAREUPGRADEDIALOG_H
#define FIRMWAREUPGRADEDIALOG_H

#include "Communicator.h"
#include "DeviceID.h"
#include "DeviceInfo.h"
#include "DeviceRebooter.h"

#include <QList>
#include <QMap>
#include <QDialog>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QMessageBox>
#include <QByteArray>
#include <QSharedPointer>

namespace FirmwareMode {
enum Enum {
  UpgradeMode,
  RecoveryMode,
  CheckMode
};
}  // namespace FirmwareMode

namespace Ui { class FirmwareUpgradeDialog; }

class FirmwareUpgradeDialog : public QDialog {
  Q_OBJECT
 public:
  explicit FirmwareUpgradeDialog(GeneSysLib::CommPtr comm,
                                 DeviceInfoPtr device,
                                 FirmwareMode::Enum mode =
                                     FirmwareMode::UpgradeMode,
                                 QWidget *parent = 0);

/*  FirmwareUpgradeDialog(GeneSysLib::CommPtr comm,
                                 DeviceInfoPtr device,
                                 FirmwareMode::Enum mode,
                                 QWidget *parent,
                                 QString url);
*/
 //Support local firmware file updated, zx, 2017-05-08
   FirmwareUpgradeDialog(GeneSysLib::CommPtr comm,
                                 DeviceInfoPtr device,
                                 QString fwFileName,
                                 QWidget *parent = 0);

    void ShowFWUpdateDialogForLocalFile();
 signals:
  void firmwareUpToDate(QString version);
  void firmwareUpdateRequired(QString version, QString url);
  void checkComplete();
  void sendNext();
  void redirectDownloadURL();//Bugfixing for new download website, zx 2017-03-03
  void firmwareUpdateTimeOut(); //Fixed timeout signal issue, zx, 2017-06-19

 private slots:
  void finishedRequest(QNetworkReply *reply);
  void downloadFirmware(QString version, QString url);

  void rebootComplete();
  void messagePump();

  void reQueryDownloadSite();//Bugfixing for new download website, zx 2017-03-03

  void fileUpdateTimeOut();//zx,2017-06-21
 private:
  void parseXML(QString xml);
  void parseMIDI(QByteArray midi);
  void checkFirmware();
  void handleACK(GeneSysLib::CmdEnum command, GeneSysLib::DeviceID deviceID,
                 Word transID, GeneSysLib::commandData_t commandData);

  QSharedPointer<Ui::FirmwareUpgradeDialog> ui;
  GeneSysLib::CommPtr comm;
  DeviceInfoPtr device;
  FirmwareMode::Enum firmwareMode;

  QSharedPointer<QNetworkAccessManager> nam;

  std::vector<std::pair<Bytes, Word> > searchList;

  QList<QMap<QString, QString> > firmwareList;
  QList<QByteArray> sysexMessages;

  QMessageBox *noUpgradeNeeded;

  bool ackRegistered;

  QSharedPointer<DeviceRebooter> deviceRebooter;

  QString m_downloadURL;

  QString m_LocalFirmwareFile;
  bool    m_bLocalFile;
protected:
  //bugfixing, zx, 2017-03-22
    void timerEvent(QTimerEvent *event);
    int  m_TimerCount;
};

#endif  // FIRMWAREUPGRADEDIALOG_H
