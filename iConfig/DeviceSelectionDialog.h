/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef DEVICESELECTIONDIALOG_H
#define DEVICESELECTIONDIALOG_H

#include "Communicator.h"
#include "DeviceID.h"
#include "DeviceInfo.h"

#include <QDialog>
#include <QString>
#include <QTimer>
#include <QSharedPointer>

namespace Ui { class DeviceSelectionDialog; }

class DeviceSelectionDialog : public QDialog {
  static const int kMSecDiscoveryTimeout;
  static const int kMSecGetInfoTimeout;

  Q_OBJECT

 public:
  typedef boost::tuple<GeneSysLib::DeviceID, Word, GeneSysLib::commandData_t>
      DeviceTuple;
  typedef boost::tuple<GeneSysLib::DeviceID, Word> DeviceKey;
  typedef std::map<GeneSysLib::CmdEnum, GeneSysLib::commandData_t>
      CommandDataMap;

  explicit DeviceSelectionDialog(GeneSysLib::CommPtr comm,
                                 QWidget *parent = 0);
  virtual ~DeviceSelectionDialog();

  DeviceInfoPtr getSelectedDevice();

  //Re-factory device selection UI functions, zx, 2017-04-06
  DeviceInfoPtr GetPreviousOpenDevice();
  void SetPreviousCachedDeviceID(GeneSysLib::DeviceID deviceID);
  GeneSysLib::DeviceID m_CachedPreviousDviceID;


 signals:
  void deviceDiscovered(QString item);

  void enableButton(bool);
  void enableRefresh(bool);

 public slots:
  virtual void accept();
  virtual void reject();
  void connectToOne();

 private slots:
  void startSearch();
  void stopSearch();

  void startDiscoveryTimer();
  void stopDiscoveryTimer();
  void startGetInfoTimer();
  void stopGetInfoTimer();

  void discoverAll();

  void discoveryTick();
  void getInfoTick();

  void addItemToList(QString item);

  void on_refreshPushButton_clicked();
private:
  void discoveredDeviceCallback(GeneSysLib::CmdEnum command,
                                GeneSysLib::DeviceID deviceID, Word transID,
                                GeneSysLib::commandData_t commandData);
  void commandListCallback(GeneSysLib::CmdEnum command,
                           GeneSysLib::DeviceID deviceID, Word transID,
                           GeneSysLib::commandData_t commandData);
  void commonCallback(GeneSysLib::CmdEnum command,
                      GeneSysLib::DeviceID deviceID, Word transID,
                      GeneSysLib::commandData_t commandData);
  void midiInfoCallback(GeneSysLib::CmdEnum command,
                        GeneSysLib::DeviceID deviceID, Word transID,
                        GeneSysLib::commandData_t commandData);
  void infoListCallback(GeneSysLib::CmdEnum command,
                        GeneSysLib::DeviceID deviceID, Word transID,
                        GeneSysLib::commandData_t commandData);
  void deviceInfoCallback(GeneSysLib::CmdEnum command,
                          GeneSysLib::DeviceID deviceID, Word transID,
                          GeneSysLib::commandData_t commandData);

  void emitSerialNumber(GeneSysLib::DeviceID deviceID);
  void emitRecoveryDevice(Word pid);

  void emitDevice(DeviceInfoPtr &device);

  std::list<DeviceInfoPtr> devices;
  std::queue<std::pair<unsigned int, Bytes> > pendingDiscoverySysex;
  std::queue<std::pair<unsigned int, Bytes> > pendingGetInfoSysex;

  void sendNextDiscovery();
  void sendNextGetInfo();

  DeviceInfoPtr findDevice(GeneSysLib::DeviceID deviceID);

  bool started;

  std::map<GeneSysLib::CmdEnum, long> registerHandleredCallbackMap;
  long retDeviceHandlerID;
  long retCmdListHandlerID;
  long retInfoListHandlerID;
  long retInfoHandlerID;

  QSharedPointer<Ui::DeviceSelectionDialog> ui;
  QSharedPointer<QTimer> discoveryTimer;
  QSharedPointer<QTimer> getInfoTimer;
  GeneSysLib::CommPtr comm;
};

#endif  // DEVICESELECTIONDIALOG_H
