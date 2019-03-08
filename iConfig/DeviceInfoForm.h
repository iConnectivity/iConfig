/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef DEVICEINFOFORM_H
#define DEVICEINFOFORM_H

#include "Communicator.h"
#include "DeviceID.h"
#include "DeviceInfo.h"
#include "IRefreshData.h"
#include "MyLineEdit.h"
#include "refreshobject.h"

#include <QObject>
#include <QDialog>
#include <QSharedPointer>

class QTreeWidgetItem;

namespace Ui { class DeviceInfoForm; }

class DeviceInfoForm : public RefreshObject {
  Q_OBJECT

 public:
  explicit DeviceInfoForm(DeviceInfoPtr deviceReference, QWidget *parent = 0);
  virtual ~DeviceInfoForm();

  static const QString DefaultWindowTitle;

 signals:
  void informationAvailable();
  void updateMessage(QString message, int timeout);

 private slots:
  void setupSideBar();
  void buildInformation();
  void queryCompleted(Screen screen, CommandQList found);

 private:
  static QString infoStringTitle(Byte infoID);
  void ackCallback(GeneSysLib::CmdEnum command,
                   GeneSysLib::DeviceID deviceID, Word transID,
                   GeneSysLib::commandData_t commandData);
  void refreshWidget();

  //////////////////////////////////////////////////////////////////////////////
  /// add methods
  //////////////////////////////////////////////////////////////////////////////
  void addEthernetRow(QTreeWidgetItem *currentTreeItem, const Word &portID);
  void addInfoWidget(QTreeWidgetItem *treeItem, GeneSysLib::InfoIDEnum infoID);

  void addComboBox(QTreeWidgetItem *treeItem, QString title,
                   const GetComboBoxOptions &options,
                   const GetComboBoxSelectionFunctor &getFunc,
                   const SetComboBoxSelectionFunctor &setFunc);

  void addNetAddrLineEdit(QTreeWidgetItem *treeItem, QString title,
                          const GetQStringFunctor &getFunc,
                          const SetQStringFunctor &setFunc);

  void addLabel(QTreeWidgetItem *treeItem, QString title,
                const GetQStringFunctor &getFunc);

  void addNetDeviceName(QTreeWidgetItem *treeItem, Word portID);

  //////////////////////////////////////////////////////////////////////////////
  /// getter helpers
  //////////////////////////////////////////////////////////////////////////////
  QString infoString(GeneSysLib::InfoIDEnum infoID) const;
  QStringList ipModeOptions() const;
  int ipMode(Word portID, QComboBox *) const;

  QString staticIPAddress(Word portID) const;
  QString staticSubnetMask(Word portID) const;
  QString staticGateway(Word portID) const;

  QString currentIPAddress(Word portID) const;
  QString currentSubnetMask(Word portID) const;
  QString currentGateway(Word portID) const;

  QString macAddress(Word portID) const;
  QString getDeviceNetName(Word portID) const;

  //////////////////////////////////////////////////////////////////////////////
  /// setter helpers
  //////////////////////////////////////////////////////////////////////////////
  void setInfoString(GeneSysLib::InfoIDEnum infoID, QString value);

  void ipMode(Word portID, QComboBox *, int value);
  void staticIPAddress(Word portID, QString value);
  void staticSubnetMask(Word portID, QString value);
  void staticGateway(Word portID, QString value);

  DeviceInfoPtr device;

  QVector<IRefreshData *> refreshDataList;

  QSharedPointer<Ui::DeviceInfoForm> ui;

  GeneSysLib::DeviceID deviceID;
  Word transID;

  bool treeBuilt;

  std::map<GeneSysLib::CmdEnum, long> registeredHandlerIDs;
};

#endif  // DEVICEINFOFORM_H
