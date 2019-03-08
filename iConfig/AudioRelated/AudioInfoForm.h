/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef AUDIOINFOFORM_H
#define AUDIOINFOFORM_H

#include "AudioInfo.h"
#include "AudioCfgInfo.h"
#include "AudioClockInfo.h"
#include "AudioPortCfgInfo.h"
#include "AudioPortInfo.h"
#include "AudioGlobalParm.h"
#include "AudioPortParm.h"
#include "AudioDeviceParm.h"
#include "AudioControlParm.h"
#include "AudioControlDetail.h"
#include "AudioClockParm.h"
#include "AudioPatchbayParm.h"
#include "AudioPortPatchbay.h"
#include "MixerParm.h"
#include "MixerPortParm.h"
#include "Communicator.h"
#include "DeviceInfo.h"
#include "GetSetFunc.h"
#include "refreshobject.h"
#include "CommandDataKey.h"
#include "MixerRelated/MixerInterface.h"

#include <QSharedPointer>
#include <QTreeWidgetItem>

class MyLabel;
class MyLineEdit;
class QTableWidget;
class MyComboBox;
struct IRefreshData;

namespace GeneSysLib {
struct AudioCfgInfo;
struct AudioInfo;
struct AudioPortCfgInfo;

struct AudioGlobalParm;
struct AudioPortParm;

struct MixerParm;
}

using namespace GeneSysLib;

namespace Ui { class AudioInfoForm; }

class AudioInfoForm : public RefreshObject {
  Q_OBJECT
 public:
  explicit AudioInfoForm(DeviceInfoPtr deviceReference, QWidget* parent = 0);
  virtual ~AudioInfoForm();

  static const QString DefaultWindowTitle;

public slots:
  bool promptForClose();

signals:
  void v1InfoAvailable();
  void v2InfoAvailable();

  void updateMessage(QString message, int timeout);

 private slots:
  void buildV1Info();
  void buildV2Info();
  void queryCompleted(Screen screen, CommandQList found);
  void on_pushButton_clicked();
  void on_pushButton_2_clicked();

  void refreshWidget();
  void saveOldValues();
  void restoreOldValues();


 private:
  //////////////////////////////////////////////////////////////////////////////
  /// USB Device Tables
  //////////////////////////////////////////////////////////////////////////////
  void createUSBDeviceTableV1(const GeneSysLib::AudioInfo& audioInfo);
  void createPortTablesV2();
  void createUSBDeviceTableV2();
  void createUSBHostTablesV2();
  void createUSBHostTableV2(Byte hostJack);
  void createEthernetTablesV2();
  void createEthernetTableV2(Byte hostJack);
  void createAnalogueTableV2();

  void addUSBDeviceRowV1(QTableWidget* table, int row, Word portID);
  void addUSBDeviceRowV2(QTableWidget* table, int row,
                         const GeneSysLib::AudioPortParm& audioPortParm);
  void addUSBHostRowV2(QTableWidget* table, int row,
                       const GeneSysLib::AudioPortParm& audioPortParm);
  void addEthernetRowV2(QTableWidget* table, int row,
                        const GeneSysLib::AudioPortParm& audioPortParm);
  void addAnalogueRowV2(QTableWidget *table, int row,
                        const GeneSysLib::AudioPortParm &audioPortParm);

  MyLabel* addLabel(QTableWidget* tableWidget, int row, int col,
                    const GetQStringFunctor& getFunc);
  MyLabel* addLabel(QTableWidget* tableWidget, int row, int col,
                    const QString& value);
  MyLineEdit* addLineEdit(QTableWidget* tableWidget, int row, int col,
                          int maxLength, const GetQStringFunctor& getFunc,
                          const SetQStringFunctor& setFunc);
  void addCheckBox(QTableWidget* tableWidget, int row, int col,
                   const GetBoolFunctor& getFunc,
                   const SetBoolFunctor& setFunc);
  void addCheckBox(QTableWidget* tableWidget, int row, int col,
                   const GetBoolFunctor& getFunc, const SetBoolFunctor& setFunc,
                   const GetBoolFunctor& enableFunc);
  void addComboBox(QTableWidget* tableWidget, int row, int col,
                   const GetComboBoxOptions& options,
                   const GetComboBoxSelectionFunctor& getFunc,
                   const SetComboBoxSelectionFunctor& setFunc);

//  void closeEvent(QCloseEvent* event);

  QTreeWidgetItem* addAudioInfoSectionV1(GeneSysLib::AudioInfo& audioInfo);
  QTreeWidgetItem* addAudioInfoSectionV2(
      GeneSysLib::AudioGlobalParm& audioGlobalParm);
  QTreeWidgetItem* addMixerInfoSectionV2(
      GeneSysLib::MixerParm& mixerParm);

  void addAudioFrameComboBoxV1(QTreeWidgetItem* treeItem);
  void addAudioFrameComboBoxV2(QTreeWidgetItem* treeItem);

  void addSyncFactorComboBoxV1(QTreeWidgetItem* treeItem);
  void addSyncFactorComboBoxV2(QTreeWidgetItem* treeItem);

  void addAudioConfigSectionV1(QTreeWidgetItem* treeItem);
  void addAudioConfigSectionV2(QTreeWidgetItem* treeItem);

  void addMixerConfigSectionV2(QTreeWidgetItem* treeItem);
  void addMixerInputsAndOutputsConfigSectionV2(QTreeWidgetItem* treeItem);

  void addClockSourceV1(QTreeWidgetItem* treeItem);
  void addClockSourceV2(QTreeWidgetItem* treeItem);

  void ackCallback(GeneSysLib::CmdEnum command, GeneSysLib::DeviceID deviceID,
                   Word transID, GeneSysLib::commandData_t commandData);

  //////////////////////////////////////////////////////////////////////////////
  /// properties
  //////////////////////////////////////////////////////////////////////////////
  Word startUSBAudioPortID() const;
  Word endUSBAudioPortID(const GeneSysLib::AudioInfo& audioInfo) const;

  QStringList audioFrameOptionsV1() const;
  int getAudioFrameSelectionV1(QComboBox*) const;
  void setAudioFrameSelectionV1(QComboBox*, int value);

  QStringList audioFrameOptionsV2() const;
  int getAudioFrameSelectionV2(QComboBox*) const;
  void setAudioFrameSelectionV2(QComboBox*, int value);

  QStringList audioSyncOptionsV1() const;
  int getAudioSyncSelectionV1(QComboBox*) const;
  void setAudioSyncSelectionV1(QComboBox*, int value);

  QStringList audioSyncOptionsV2() const;
  int getAudioSyncSelectionV2(QComboBox*) const;
  void setAudioSyncSelectionV2(QComboBox*, int value);

  QStringList audioConfigOptionsV1() const;
  int getAudioConfigSelectionV1(QComboBox*) const;
  void setAudioConfigSelectionV1(QComboBox*, int value);

  QStringList audioConfigOptionsV2() const;
  int getAudioConfigSelectionV2(QComboBox*) const;
  void setAudioConfigSelectionV2(QComboBox*, int value);

  QStringList mixerConfigOptionsV2() const;
  int getMixerConfigSelectionV2(QComboBox*) const;
  void setMixerConfigSelectionV2(QComboBox*, int value);

  QStringList outputsPerPortConfigOptionsV2(int audioPortID) const;
  int getOutputsPerPortConfigSelectionV2(int audioPortID, QComboBox*) const;
  void setOutputsPerPortConfigSelectionV2(int audioPortID, QComboBox*, int value);

  QStringList inputsPerPortConfigOptionsV2(int audioPortID) const;
  int getInputsPerPortConfigSelectionV2(int audioPortID, QComboBox*) const;
  void setInputsPerPortConfigSelectionV2(int audioPortID, QComboBox*, int value);

  QString getJackNumberV1(Word portID) const;
  QString getJackNumberV2(Word portID) const;

  int getMaxPortNameV1(Word portID) const;
  QString getPortNameV1(Word portID) const;
  void setPortNameV1(Word portID, QString value);

  int getMaxPortNameV2(Word audioPortID) const;
  QString getPortNameV2(Word audioPortID) const;
  void setPortNameV2(Word audioPortID, QString value);

  bool getMacPCEnabledV1(Word portID) const;
  void setMacPCEnabledV1(Word portID, bool value);

  bool getMacPCEnabledV2(Word audioPortID) const;
  void setMacPCEnabledV2(Word audioPortID, bool value);

  bool getIOSEnabledV1(Word portID) const;
  void setIOSEnabledV1(Word portID, bool value);

  bool getIOSEnabledV2(Word portID) const;
  void setIOSEnabledV2(Word portID, bool value);

  QString getNumInputChannelsLabelV1(Word portID) const;

  QStringList getInputOptionsV1(Word portID) const;
  int getSelectedInputV1(Word portID, QComboBox*) const;
  void setSelectedInputV1(Word portID, QComboBox*, int selection);

  QString getNumInputChannelsLabelV2(Word portID) const;

  QStringList getInputOptionsV2(Word audioPortID) const;
  int getSelectedInputV2(Word audioPortID, QComboBox*) const;
  void setSelectedInputV2(Word audioPortID, QComboBox*, int selection);

  QString getNumOutputChannelsLabelV1(Word portID) const;

  QStringList getOutputOptionsV1(Word portID) const;
  int getSelectedOutputV1(Word portID, QComboBox*) const;
  void setSelectedOutputV1(Word portID, QComboBox*, int selection);

  QString getNumOutputChannelsLabelV2(Word audioPortID) const;

  QStringList getOutputOptionsV2(Word audioPortID) const;
  int getSelectedOutputV2(Word audioPortID, QComboBox*) const;
  void setSelectedOutputV2(Word audioPortID, QComboBox*, int selection);

  QStringList clockSourceOptionsV1() const;
  int getClockSourceV1(QComboBox*) const;
  void setClockSourceV1(QComboBox*, int selection);

  QStringList clockSourceOptionsV2() const;
  int getClockSourceV2(QComboBox*) const;
  void setClockSourceV2(QComboBox*, int selection);

  QString getUSBDeviceHostTypeV2(Word audioPortID) const;

  QString getUSBDeviceHostNameV2(Word audioPortID) const;

  QString getUSBHostDeviceNumberV2(Word audioPortID) const;

  QString getEthernetDeviceNumberV2(Word audioPortID) const;

  QString getAnaloguePortV2(Word audioPortID) const;

  QString getConnectedV2(Word audioPortID) const;

  bool getReservedV2(Word audioPortID) const;
  void setReservedV2(Word audioPortID, bool value);
  bool reservedEnabledV2(Word audioPortID) const;

  QString getMaxInChannelsV2(Word audioPortID) const;
  QString getMaxOutChannelsV2(Word audioPortID) const;

  QString getUSBHostVendorNameV2(Word audioPortID) const;
  QString getUSBHostProductNameV2(Word audioPortID) const;

  //////////////////////////////////////////////////////////////////////////////
  /// variables
  //////////////////////////////////////////////////////////////////////////////
  DeviceInfoPtr device;

  QSharedPointer<Ui::AudioInfoForm> ui;

  bool treeBuilt;

  std::map<GeneSysLib::CmdEnum, long> registeredHandlerIDs;

  std::queue<Bytes> sysexMessages;

  std::vector<IRefreshData*> refreshList;
  std::vector<MyComboBox*> outputBoxes;
  MixerInterface* mixerInterface;

  AudioGlobalParm oldAudioGlobalParm;
  std::vector<AudioDeviceParm*> oldAudioDeviceParms;
  std::vector<AudioPortParm*> oldAudioPortParms;
  AudioClockParm oldAudioClockParm;
  MixerParm oldMixerParm;
  MixerPortParm oldMixerPortParm;

  AudioInfo oldAudioInfo;
  AudioCfgInfo oldAudioCfgInfo;
  std::vector<bool> oldPortMacPCEnabled;
  std::vector<bool> oldPortIOSEnabled;
  std::vector<std::string> oldPortNames;
  std::vector<int> oldPortIn;
  std::vector<int> oldPortOut;
  AudioClockInfo oldAudioClockInfo;

  bool changesHaveBeenMade;
  void checkOutputMixTotals(MyComboBox *changedB);
  void notifyChanges();
  bool refreshing;
};

#endif  // DEVICEINFOFORM_H
