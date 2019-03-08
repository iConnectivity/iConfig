/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef MIDIPORTSELECTIONFORM_H
#define MIDIPORTSELECTIONFORM_H

#include "./MIDIRelated/MIDIPortSelectionForm.h"
#include "DeviceInfo.h"
#include "MyAlgorithms.h"
#include "PortIDVector.h"
#include "MIDIInfo.h"
#include "MIDIPortInfo.h"

#include <QWidget>
#include <QSharedPointer>
#include <QTreeWidgetItem>

namespace Ui { class MIDIPortSelectionForm; }

class MIDIPortSelectionForm : public QWidget {
  Q_OBJECT
 public:
  static const QString DefaultWindowTitle;

  explicit MIDIPortSelectionForm(DeviceInfoPtr device, QWidget *parent = 0);

  void setSelectionMode(QAbstractItemView::SelectionMode mode);
  PortIDVector selectedPortIDs() const;
  Word selectedPortID() const;

 signals:
  void selectedPortIDsChanged(PortIDVector);

 public slots:
  void updateTree();
  void itemClicked(QTreeWidgetItem *item, int column);

 private slots:
  void selectionChanged();

 private:
  void createTree();
  template <typename T>
  void addTreeItem(T *parent, const GeneSysLib::MIDIPortInfo &portInfo,
                   PortIDVector selectedIDs) {
    QTreeWidgetItem *treeItem = new QTreeWidgetItem(parent);
    treeItem->setText(0, QString::fromStdString(portInfo.portName()));
    treeItem->setData(1, Qt::UserRole, QVariant::fromValue(portInfo.portID()));
    treeItem->setSelected(MyAlgorithms::contains(
        selectedIDs, static_cast<Word>(portInfo.portID())));
    portItemMap[portInfo.portID()] = treeItem;
  }

  DeviceInfoPtr device;

  GeneSysLib::MIDIInfo midiInfo;

  QSharedPointer<Ui::MIDIPortSelectionForm> ui;

  std::map<Word, QTreeWidgetItem *> portItemMap;
};

#endif  // MIDIPORTSELECTIONFORM_H
