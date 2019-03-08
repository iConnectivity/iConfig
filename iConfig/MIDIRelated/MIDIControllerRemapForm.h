/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef MIDICONTROLLERREMAPFORM_H
#define MIDICONTROLLERREMAPFORM_H

#include "./MIDIRelated/MIDIPortSelectionForm.h"
#include "DeviceID.h"
#include "refreshobject.h"
#include "tablelistener.h"

#include <QSignalMapper>
#include <QSharedPointer>

using std::map;

namespace Ui { class MIDIControllerRemapForm; }

class MIDIControllerRemapForm : public RefreshObject {
  Q_OBJECT
 public:
  static const QString DefaultWindowTitle;
  static const float kBatchTime;
  static QList<GeneSysLib::CmdEnum> Query();

  explicit MIDIControllerRemapForm(DeviceInfoPtr device, QWidget *parent = 0);
  virtual ~MIDIControllerRemapForm();

 private slots:
  void selectedPortIDsChanged(PortIDVector portIDs);

  void updateCCRemap();
  void sendUpdate();
  void cellStateChange(int row, int col, BlockState::Enum state);

  void srcComboBoxChanged(int row);
  void dstComboBoxChanged(int row);

  void refreshWidget();

 private:
  void ackCallback(GeneSysLib::CmdEnum, GeneSysLib::DeviceID, Word,
                   GeneSysLib::commandData_t);
  long ackHandlerID;

  BlockState::Enum stateForCell(int row, int col) const;
  GeneSysLib::RemapTypeEnum currentRemapID() const;
  void addToUpdateList(Word portID);

  QTimer *sendTimer;
  QMutex updateMutex;
  QHash<GeneSysLib::RemapTypeEnum, Word> updateList;

  QSharedPointer<Ui::MIDIControllerRemapForm> ui;
  MIDIPortSelectionForm *portSelectionForm;

  DeviceInfoPtr device;

  TableListener *tableListener;

  QSignalMapper *srcSignalMapper;
  QSignalMapper *dstSignalMapper;

  map<int, GeneSysLib::MIDIPortRemap::ChannelBitmapBits::Enum>
      colChBitmapMap;
};

#endif  // MIDICONTROLLERREMAPFORM_H
