/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef MIDICHANNELREMAPFORM_H
#define MIDICHANNELREMAPFORM_H

#include "./MIDIRelated/MIDIPortSelectionForm.h"
#include "DeviceID.h"
#include "DeviceInfoForm.h"
#include "refreshobject.h"
#include "tablelistener.h"
#include "RemapID.h"

#include <QTimer>
#include <QMutex>
#include <QSignalMapper>
#include <QSharedPointer>

namespace GeneSysLib { struct MIDIInfo; struct MIDIPortInfo; }
namespace Ui { class MIDIChannelRemapForm; }

class MIDIChannelRemapForm : public RefreshObject {
  Q_OBJECT
 public:
  explicit MIDIChannelRemapForm(GeneSysLib::CommPtr comm, DeviceInfoPtr device,
                                QWidget *parent = 0);
  virtual ~MIDIChannelRemapForm();

  static const QString DefaultWindowTitle;
  static const float kBatchTime;
  static QList<GeneSysLib::CmdEnum> Query();

 private slots:
  void selectedPortIDsChanged(PortIDVector portIDs);

  void updateChannelRemap();
  void sendUpdate();
  void cellStateChange(int row, int col, BlockState::Enum state);

  void lineEditChanged(int col);
  void refreshWidget();

 private:
  BlockState::Enum stateForCell(int row, int col) const;
  bool rowToRemapStatus(
      const GeneSysLib::MIDIPortRemap::RemapStatus &remapStatus, int row) const;
  void setRemapStatusBit(GeneSysLib::MIDIPortRemap::RemapStatus &remapStatus,
                         int row, bool value) const;
  GeneSysLib::RemapTypeEnum currentRemapID() const;
  void addToUpdateList(Word portID);

  GeneSysLib::CommPtr comm;
  DeviceInfoPtr device;
  MIDIPortSelectionForm *portSelectionForm;
  QHash<GeneSysLib::RemapTypeEnum, Word> updateList;
  QMutex updateMutex;
  QSignalMapper *lineEditSignalMapper;
  QTimer *sendTimer;
  TableListener *tableListener;
  QSharedPointer<Ui::MIDIChannelRemapForm> ui;
};

#endif  // MIDICHANNELREMAPFORM_H
