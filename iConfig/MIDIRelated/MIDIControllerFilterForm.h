/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef MIDICONTROLLERFILTERFORM_H
#define MIDICONTROLLERFILTERFORM_H

#include "./MIDIRelated/MIDIPortSelectionForm.h"
#include "BlockState.h"
#include "DeviceID.h"
#include "refreshobject.h"
#include "tablelistener.h"

#include <QMutex>
#include <QComboBox>
#include <QSharedPointer>
#include <QTimer>

namespace Ui { class MIDIControllerFilterForm; }

class MIDIControllerFilterForm : public RefreshObject {
  Q_OBJECT public : static QList<GeneSysLib::CmdEnum> Query();
  static const QString DefaultWindowTitle;
  static const float kBatchTime;

  explicit MIDIControllerFilterForm(GeneSysLib::CommPtr comm,
                                    DeviceInfoPtr device, QWidget *parent = 0);
  virtual ~MIDIControllerFilterForm();

 private
slots:
  void selectedPortIDsChanged(PortIDVector portIDs);
  void updateFilters();
  void sendUpdate();
  void cellStateChange(int row, int col, BlockState::Enum state);
  void refreshWidget();

 private:
  void addCCComboBox(int row);
  void addEmptyLabel(int row, int col);
  BlockState::Enum stateForCell(int row, int col) const;
  GeneSysLib::FilterIDEnum currentFilterID() const;
  void addToUpdateList(Word portID);

  int controllerIndexForRow(int row, QComboBox *) const;
  void setControllerIndexForRow(int row, QComboBox *, int selection);

  GeneSysLib::CommPtr comm;
  DeviceInfoPtr device;
  MIDIPortSelectionForm *portSelectionForm;
  QHash<int, GeneSysLib::FilterIDEnum> updateList;
  QMutex updateMutex;
  QTimer *sendTimer;
  TableListener *tableListener;

  QSharedPointer<Ui::MIDIControllerFilterForm> ui;

  std::map<int, GeneSysLib::ChannelBitmapBitEnum> colChBitmapMap;
};

#endif  // MIDICONTROLLERFILTERFORM_H
