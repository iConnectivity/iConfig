/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "MIDIPortSelectionForm.h"
#include "ui_MIDIPortSelectionForm.h"

#include "MyAlgorithms.h"
#include "TreeUtils.h"

#ifndef Q_MOC_RUN
#include <boost/range/adaptors.hpp>
#endif

using namespace GeneSysLib;
using namespace MyAlgorithms;
using namespace TreeUtils;
using namespace boost::adaptors;
using namespace boost;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
/// Static data
////////////////////////////////////////////////////////////////////////////////
const QString MIDIPortSelectionForm::DefaultWindowTitle = tr("MIDI Port Selection");

////////////////////////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////////////////////////
MIDIPortSelectionForm::MIDIPortSelectionForm(DeviceInfoPtr _device,
                                             QWidget *_parent)
    : QWidget(_parent), device(_device), ui(new Ui::MIDIPortSelectionForm) {
  // set the ui
  ui->setupUi(this);

  // assert that the device exists
  Q_ASSERT(device);

  // get the stored midi info
  midiInfo = device->get<MIDIInfo>();

  // connect the item clicked callback
  connect(ui->portTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this,
          SLOT(itemClicked(QTreeWidgetItem *, int)));
  // connect the item selected callback
  connect(ui->portTreeWidget, SIGNAL(itemSelectionChanged()), this,
          SLOT(selectionChanged()));

  // register the Handler type PortIDVector
  static bool registerHandlerType = false;
  if (!registerHandlerType) {
    qRegisterMetaType<PortIDVector>("PortIDVector");
  }

  createTree();
}

////////////////////////////////////////////////////////////////////////////////
/// This method sets to selection mode of the port tree widget
////////////////////////////////////////////////////////////////////////////////
void MIDIPortSelectionForm::setSelectionMode(
    QAbstractItemView::SelectionMode mode) {
  ui->portTreeWidget->setSelectionMode(mode);
}

////////////////////////////////////////////////////////////////////////////////
/// Create the tree
////////////////////////////////////////////////////////////////////////////////
void MIDIPortSelectionForm::createTree() {
  // get the selected port IDs
  auto portIDs = selectedPortIDs();

  // clear the tree widgets from previous creation
  ui->portTreeWidget->clear();
  // clearn the port item map from previous creation
  portItemMap.clear();

  // create USB device jack tree items
  for (auto i = 1; i <= midiInfo.numUSBDeviceJacks(); ++i) {
    // create the parent branch for the jack
    auto *const usbDeviceJack = addTreeWidgetItem(
        ui->portTreeWidget, tr("USB Device Jack %1").arg(QString::number(i)));

      // create children items of type USB device for the current jack
    device->for_each<MIDIPortInfo>([&](MIDIPortInfo & portInfo) {
      if ((portInfo.isOfType(PortType::USBDevice)) &&
          (portInfo.portInfo().usbDevice.jack == i)) {
        // add the tree item to the usb device jack parent
        addTreeItem(usbDeviceJack, portInfo, portIDs);
      }
    });
  }

  // create usb host jack tree items
  for (auto i = 1; i <= midiInfo.numUSBHostJacks(); ++i) {
    // create the parent branch for the jack
    auto *const usbHostJack =
        addTreeWidgetItem(ui->portTreeWidget,
                          QString("USB Host Jack %1").arg(QString::number(i)));

    // create the usb host jack filter
    const auto &usbHostJackFilter = [i](MIDIPortInfo _portInfo) {
      return ((_portInfo.isOfType(PortType::USBHost)) &&
              (_portInfo.portInfo().usbHost.jack == i));
    }
    ;

      // create children items of type usb host for the current jack
    device->for_each<MIDIPortInfo>([&](const MIDIPortInfo & portInfo) {
      if (usbHostJackFilter(portInfo)) {
        // add the tree item to the usb host jack parent
        addTreeItem(usbHostJack, portInfo, portIDs);
      }
    });
  }

  // create ethernet jack tree items
  for (auto i = 1; i <= midiInfo.numEthernetJacks(); ++i) {
    // create the parent branch for the jack
    auto *const ethernetJack =
        addTreeWidgetItem(ui->portTreeWidget,
                          QString("Ethernet Jack %1").arg(QString::number(i)));

    // create the ethernet jack filter
    const auto &ethernetJackFilter = [ = ](MIDIPortInfo _portInfo) {
      return ((_portInfo.isOfType(PortType::Ethernet)) &&
              (_portInfo.portInfo().ethernet.jack == i));
    }
    ;

      // create children items of the type ethernet for the current jack
    device->for_each<MIDIPortInfo>([&](const MIDIPortInfo & portInfo) {
      if (ethernetJackFilter(portInfo)) {
        // add the tree item to the ethernet jack parent
        addTreeItem(ethernetJack, portInfo, portIDs);
      }
    });
  }

    // create all DIN tree items
  device->for_each<MIDIPortInfo>([&](MIDIPortInfo & portInfo) {
    if (portInfo.isOfType(PortType::DIN)) {
      addTreeItem(ui->portTreeWidget, portInfo, portIDs);
    }
  });

  if ((portIDs.size() == 0) && (ui->portTreeWidget->topLevelItemCount())) {
    if (ui->portTreeWidget->topLevelItem(0)->childCount() == 0) {
      ui->portTreeWidget->topLevelItem(0)->setSelected(true);
    } else {
      ui->portTreeWidget->topLevelItem(0)->child(0)->setSelected(true);
    }
  }

  // expand all
  ui->portTreeWidget->expandAll();
}

////////////////////////////////////////////////////////////////////////////////
/// Update all the tree elements
////////////////////////////////////////////////////////////////////////////////
void MIDIPortSelectionForm::updateTree() {
    // loop through all
  device->for_each<MIDIPortInfo>([&](const MIDIPortInfo & portInfo) {
    if (contains(portItemMap, portInfo.portID())) {
      // get the tree item
      auto *const treeItem = portItemMap.at(portInfo.portID());

      // assert that a tree item is available
      Q_ASSERT(treeItem);

      // update all the port names
      treeItem->setText(0, QString::fromStdString(portInfo.portName()));
    }
  });
}

////////////////////////////////////////////////////////////////////////////////
/// Override the item clicked method so nothing happens
////////////////////////////////////////////////////////////////////////////////
void MIDIPortSelectionForm::itemClicked(QTreeWidgetItem *, int) {}

////////////////////////////////////////////////////////////////////////////////
/// The method is called when the selection has changed
////////////////////////////////////////////////////////////////////////////////
void MIDIPortSelectionForm::selectionChanged() {
  // disable the callback so infinite loop doesn't occur
  disconnect(ui->portTreeWidget, SIGNAL(itemSelectionChanged()), this,
             SLOT(selectionChanged()));

  auto addSelection = false;
  auto first = true;

  // loop through all the selected items
  for (const auto &item : ui->portTreeWidget->selectedItems()) {
    // if the selection mode is single selection
    if (ui->portTreeWidget->selectionMode() ==
        QAbstractItemView::SingleSelection) {

      // this is single selection so unselect all but the selected
      if (!first) {
        item->setSelected(false);
      }

      // if this is a branch then remove the selection and select the first
      // child if nothing else was selected
      if (item->childCount() > 0) {
        // deselect branch
        item->setSelected(false);

        // if there isn't a child already selected
        if (!addSelection) {
          // select first childen
          item->child(0)->setSelected(true);
          // set the selection
          addSelection = true;
        }
      }
    } else if (ui->portTreeWidget->selectionMode() !=
               QAbstractItemView::
                   NoSelection) {  // selection mode is not "No Selection" or
                                   // "SingleSelection"

      // loop through all the children and select all children
      for (auto i = 0; i < item->childCount(); ++i) {
        // if the child is not selected
        if (!item->child(i)->isSelected()) {
          // select the child
          item->child(i)->setSelected(true);
        }
      }
    }

    // past this point it is no longer the first child
    first = false;
  }

  // if there are no selected ports then select the first leaf
  if (ui->portTreeWidget->selectedItems().isEmpty()) {
    // get the number of top level items
    int topLevelItemCount = ui->portTreeWidget->topLevelItemCount();

    // assert that there are some top level items
    Q_ASSERT((topLevelItemCount - 1) >= 0);

    // get the first top level item
    auto *item = ui->portTreeWidget->topLevelItem(0);

    // find the first leaf
    while (item->childCount() > 0) {
      item = item->child(0);
    }

    // select the first leaf
    item->setSelected(true);
  }

  // notify listeners that the selected port ids have changed
  emit selectedPortIDsChanged(selectedPortIDs());

  // re-enable callbacks
  connect(ui->portTreeWidget, SIGNAL(itemSelectionChanged()), this,
          SLOT(selectionChanged()));
}

////////////////////////////////////////////////////////////////////////////////
/// This method returns a list of all the selected port IDs
////////////////////////////////////////////////////////////////////////////////
PortIDVector MIDIPortSelectionForm::selectedPortIDs() const {
  vector<Word> result;

  // loop through all the leaf tree widget items
  for (const auto &item : ui->portTreeWidget->selectedItems()) {
    if (item->childCount() == 0) {
      // get the port ID variant of the leaf tree item
      const auto &portID = item->data(1, Qt::UserRole);

      // add the port ID to the result
      result.push_back(portID.toInt());
    }
  }

  // return the result
  return result;
}

////////////////////////////////////////////////////////////////////////////////
/// This method returns the first (or only) selected port ID
////////////////////////////////////////////////////////////////////////////////
Word MIDIPortSelectionForm::selectedPortID() const {
  // get the list of selected ports
  const auto &selectedPorts = selectedPortIDs();

  // assert that there is at least 1 item in the selected list
  Q_ASSERT(selectedPorts.size() >= 1);

  // return the first item in the selected port
  return selectedPorts.front();
}
