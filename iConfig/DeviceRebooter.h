/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef DEVICEREBOOTER_H
#define DEVICEREBOOTER_H

#include "Communicator.h"
#include "DeviceInfo.h"

enum RebootError {
  NoError = 0,
  CouldNotOpen,
  TimedOut
};

class DeviceRebooter : public QObject {
  Q_OBJECT
 public:
  explicit DeviceRebooter(GeneSysLib::CommPtr comm,
                          DeviceInfoPtr device, QObject *parent = 0);

signals:
  void rebootComplete(int outPort);
  void resetSent();
  void error(int code);

 public
slots:
  void reboot(GeneSysLib::BootModeEnum bootMode);

 private
slots:
  void startSearch();
  void timerTick();
  void cleanup();

 private:
  void ackHandler(GeneSysLib::CmdEnum command,
                  GeneSysLib::DeviceID deviceID, Word transID,
                  GeneSysLib::commandData_t commandData);
  void getDeviceHandler(GeneSysLib::CmdEnum command,
                        GeneSysLib::DeviceID deviceID, Word transID,
                        GeneSysLib::commandData_t commandData);

  long getDeviceHandlerID;

  GeneSysLib::CommPtr comm;
  DeviceInfoPtr device;
  QTimer *searchTimer;

  GeneSysLib::BootModeEnum desiredBootMode;

  std::queue<std::pair<Bytes, Word> > pendingSearchSysex;
  //bool                                m_HandleMio10; //zx,2017-06-22
  void sendNextDiscovery(); //zx,2017-06-22
};

#endif  // DEVICEREBOOTER_H
