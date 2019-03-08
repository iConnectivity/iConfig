/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef MIDIPORTROUTINGFORM_H
#define MIDIPORTROUTINGFORM_H

#include "./MIDIRelated/MIDIPortSelectionForm.h"
#include "CommandData.h"
#include "Communicator.h"
#include "DeviceID.h"
#include "DeviceInfo.h"
#include "refreshobject.h"
#include "tablelistener.h"

#include <set>

#include <QMutex>
#include <QSharedPointer>

namespace Ui { class MIDIPortRoutingForm; }

class MIDIPortRoutingForm : public RefreshObject {
  Q_OBJECT

 public:
  static const QString DefaultWindowTitle;
  static QList<GeneSysLib::CmdEnum> Query();

  explicit MIDIPortRoutingForm(DeviceInfoPtr device, QWidget *parent = 0);
  virtual ~MIDIPortRoutingForm();

 private slots:
  void selectedPortIDsChanged(PortIDVector portIDs);

  void cellChanged(int row, int col, BlockState::Enum state);
  void updateRouting();

  void sendUpdate();
  void refreshWidget();

 private:
  void createTable();

  int maxPorts() const;
  int rowForPort(GeneSysLib::MIDIPortInfo portInfo) const;
  int spanForPort(GeneSysLib::MIDIPortInfo portInfo) const;
  int colForPort(GeneSysLib::MIDIPortInfo portInfo) const;

  void addLabel(int row, int col, GeneSysLib::MIDIPortInfo portInfo);
  void ackCallback(GeneSysLib::CmdEnum command,
                   GeneSysLib::DeviceID deviceID, Word transID,
                   GeneSysLib::commandData_t commandData);
  bool portRouted(Word srcPortID, Word destPortID) const;
  BlockState::Enum cellState(int row, int col) const;

  static const char *kPortIDProperty;
  static const int kSendTime;

  DeviceInfoPtr device;
  GeneSysLib::MIDIInfo midiInfo;
  MIDIPortSelectionForm *portSelectionForm;
  QMutex updateMutex;
  QTimer *sendTimer;
  TableListener *tableListener;

  QSharedPointer<Ui::MIDIPortRoutingForm> ui;

  long ackHandlerID;
  std::map<Word, QLabel *> labelPortIDMap;
  std::set<Word> updateSet;
};

#endif  // MIDIPORTROUTINGFORM_H
