/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef MIDIPORTFILTERSFORM_H
#define MIDIPORTFILTERSFORM_H

#include "./MIDIRelated/MIDIPortSelectionForm.h"
#include "refreshobject.h"
#include "tablelistener.h"

#include <QSharedPointer>

namespace Ui { class MIDIPortFiltersForm; }

class MIDIPortFiltersForm : public RefreshObject {
  Q_OBJECT
 public:
  static const QString DefaultWindowTitle;
  static QList<GeneSysLib::CmdEnum> Query();

  explicit MIDIPortFiltersForm(GeneSysLib::CommPtr comm, DeviceInfoPtr device,
                               QWidget *parent = 0);
  virtual ~MIDIPortFiltersForm();

 private slots:
  void selectedPortIDsChanged(PortIDVector portIDs);
  void updateFilters();
  void sendUpdate();
  void cellStateChange(int row, int col, BlockState::Enum state);

  void refreshWidget();

 private:
  void ackCallback(GeneSysLib::CmdEnum command, GeneSysLib::DeviceID deviceID,
                   Word transID, GeneSysLib::commandData_t commandData);
  bool isCellSet(int row, int col, int portID) const;
  BlockState::Enum stateForCell(int row, int col) const;
  GeneSysLib::FilterIDEnum currentFilterID() const;

  GeneSysLib::CommPtr comm;

  DeviceInfoPtr device;

  MIDIPortSelectionForm *portSelectionForm;

  QHash<GeneSysLib::FilterIDEnum, Word> updateList;

  QMutex updateMutex;

  QTimer *sendTimer;

  TableListener *tableListener;

  QSharedPointer<Ui::MIDIPortFiltersForm> ui;

  long ackHandlerID;

  std::map<int, GeneSysLib::ChannelFilterStatusBitEnum> rowChannelStatusMap;
  std::map<int, GeneSysLib::FilterStatusBitEnum> rowFilterMap;
};

#endif  // MIDIPORTFILTERSFORM_H
