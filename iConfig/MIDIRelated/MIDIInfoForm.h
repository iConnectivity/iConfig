/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef MIDIINFORMATIONFORM_H
#define MIDIINFORMATIONFORM_H

#include "./MIDIRelated/MIDIPortSelectionForm.h"
#include "DeviceID.h"
#include "DeviceInfo.h"
#include "MIDIPortDetail.h"
#include "GetSetFunc.h"
#include "IRefreshData.h"
#include "refreshobject.h"

#include <QSharedPointer>

class MyComboBox;
class MyLabel;
class MyLineEdit;
class QTableWidget;
class QSpinBox;
class QCheckBox;

using std::map;
using GeneSysLib::MIDIInfo;
using GeneSysLib::commandData_t;
using GeneSysLib::CommPtr;

namespace Ui { class MIDIInfoForm; }

class MIDIInfoForm : public RefreshObject {
  Q_OBJECT
 public:

  explicit MIDIInfoForm(CommPtr comm, DeviceInfoPtr deviceReference,
                        QWidget *parent = 0);
  virtual ~MIDIInfoForm();

  static const QString DefaultWindowTitle;

 signals:
  void informationAvailable();
  void updateMessage(QString message, int timeout);
  void refreshAll();

 private slots:
  void buildInformation();

  // create the table
  void queryCompleted(Screen screen, CommandQList found);

  void updateNumberOfMultiPortUSB(int value);

 private:
  //////////////////////////////////////////////////////////////////////////////
  /// DIN table creation
  //////////////////////////////////////////////////////////////////////////////
  void createDINTable(const MIDIInfo &midiInfo);
  void addDINRow(QTableWidget *table, int row, Word portID);

  //////////////////////////////////////////////////////////////////////////////
  /// USB Device table creation
  //////////////////////////////////////////////////////////////////////////////
  void createUSBDeviceTables(const MIDIInfo &midiInfo);
  void createUSBDeviceTable(const MIDIInfo &midiInfo, Word jack);
  void addUSBDeviceRow(QTableWidget *table, int row, Word portID);

  //////////////////////////////////////////////////////////////////////////////
  /// USB Host table creation
  //////////////////////////////////////////////////////////////////////////////
  void createUSBHostTables(const MIDIInfo &midiInfo);
  void createUSBHostTable(const MIDIInfo &midiInfo, Word jack);
  void addUSBHostRow(QTableWidget *table, int row, Word portID);

  //////////////////////////////////////////////////////////////////////////////
  /// ethernet table creation
  //////////////////////////////////////////////////////////////////////////////
  void createEthernetTables(const MIDIInfo &midiInfo);
  void createEthernetTable(const MIDIInfo &midiInfo, Word jack);
  void addEthernetRow(QTableWidget *table, int row, Word portID);

  //////////////////////////////////////////////////////////////////////////////
  /// add methods
  //////////////////////////////////////////////////////////////////////////////
  void addPortJack(QTableWidget *tableWidget, Word portID, int row, int col);
  void addPortName(QTableWidget *tableWidget, Word portID, int row, int col);
  void addInputEnabledCheckBox(QTableWidget *tableWidget, Word portID, int row,
                               int col);
  void addOutputEnabledCheckBox(QTableWidget *tableWidget, Word portID, int row,
                                int col);

  void addCheckBox(QTableWidget *tableWidget, int row, int col,
                          const GetBoolFunctor &getFunc,
                          const SetBoolFunctor &setFunc);

  MyLineEdit *addLineEdit(QTableWidget *tableWidget, int row, int col,
                          int maxLength, const GetQStringFunctor &getFunc,
                          const SetQStringFunctor &setFunc);

  MyComboBox *addComboBox(QTableWidget *tableWidget, int row, int col,
                          const GetComboBoxOptions &options,
                          const GetComboBoxSelectionFunctor &getFunc,
                          const SetComboBoxSelectionFunctor &setFunc);

  MyLabel *addLabel(QTableWidget *tableWidget, int row, int col,
                    const GetQStringFunctor &getFunc);

  //////////////////////////////////////////////////////////////////////////////
  /// refresh widget
  //////////////////////////////////////////////////////////////////////////////
  void refreshWidget();
  void ackCallback(GeneSysLib::CmdEnum command, GeneSysLib::DeviceID deviceID,
                   Word transID, commandData_t commandData);

  QTreeWidgetItem *addGeneralMIDIInformation(QTreeWidget *parent,
                                             const MIDIInfo &midiInfo);

  int selectedUSBHostForHostDetails(
      const GeneSysLib::MIDIPortDetailTypes::USBHost &usbHostDetails,
      int choiceCount);

  bool isReservedPort(
      const GeneSysLib::MIDIPortDetailTypes::USBHost &usbHostDetails);
  bool isInReservedList(
      const GeneSysLib::MIDIPortDetailTypes::USBHost &usbHostDetails);

  QStringList generateMIDIHostOptions(
      const GeneSysLib::MIDIPortDetailTypes::USBHost &usbHostDetails);

  //////////////////////////////////////////////////////////////////////////////
  // getter helpers
  //////////////////////////////////////////////////////////////////////////////
  Word dinStartPortID() const;
  Word dinEndPortID(const MIDIInfo &midiInfo) const;
  Word usbDeviceStartPortID(const MIDIInfo &midiInfo, Word jack) const;
  Word usbDeviceEndPortID(const MIDIInfo &midiInfo, Word jack) const;
  Word usbHostStartPortID(const MIDIInfo &midiInfo, Word jack) const;
  Word usbHostEndPortID(const MIDIInfo &midiInfo, Word jack) const;
  Word ethernetStartPortID(const MIDIInfo &midiInfo) const;
  Word ethernetEndPortID(const MIDIInfo &midiInfo) const;

  QString getPortName(Word portID) const;
  int getMaxPortNameLength(Word portID) const;
  QString getJackNumber(Word portID) const;
  bool getPortInputEnabled(Word portID) const;
  bool getPortOutputEnabled(Word portID) const;

  QString getUSBDeviceHostName(Word portID) const;
  QString getUSBDeviceHostType(Word portID) const;
  QString getUSBDevicePort(Word portID) const;

  QString getUSBHostPort(Word portID) const;

  QString getUSBHostConnectedDevice(Word portID) const;
  QStringList getUSBHostOptions(Word portID);
  int getSelectedUSBHostForPort(Word portID, QComboBox *comboBox);
  QString getVendorNameForPort(Word portID) const;
  QString getProductNameForPort(Word portID) const;
  QString getHostedPortForPort(Word portID) const;
  QString getSessionNameForPort(Word portID) const;
  QString getActiveConnectionForPort(Word portID) const;
  QString getRTPIPPortForPort(Word portID) const;
  QString getRTPConnIPForPort(Word portID) const;
  QString getRTPMIDIPortForPort(Word portID) const;
  QString getConnSessionNameForPort(Word portID) const;

  //////////////////////////////////////////////////////////////////////////////
  // setter helpers
  //////////////////////////////////////////////////////////////////////////////
  void setPortName(Word portID, QString value);
  void setPortInputEnabled(Word portID, bool value);
  void setPortOutputEnabled(Word portID, bool value);
  void setSelectedUSBHostForPort(Word portID, QComboBox *comboBox, int value);

  //////////////////////////////////////////////////////////////////////////////
  // update helper
  //////////////////////////////////////////////////////////////////////////////
  void updateMIDIPortInfo(Word portID) const;
  void updateMIDIPortDetails(Word portID) const;

  //////////////////////////////////////////////////////////////////////////////
  /// member variables
  //////////////////////////////////////////////////////////////////////////////
  QSharedPointer<Ui::MIDIInfoForm> ui;
  CommPtr comm;
  DeviceInfoPtr device;
  bool treeBuilt;
  map<GeneSysLib::CmdEnum, long> registeredHandlerIDs;
  std::vector<MyComboBox *> reservedComboBoxes;
  std::vector<IRefreshData *> refreshList;
};

#endif  // MIDIINFORMATIONFORM_H
