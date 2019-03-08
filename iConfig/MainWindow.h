/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Communicator.h"
#include "CommandList.h"
#include "DeviceInfo.h"
#include "SaveRestoreList.h"
#include "DeviceInformationDialog.h"

#include <QMainWindow>
#include <QAction>
#include <QMdiSubWindow>
#include <QCloseEvent>
#include <QProgressDialog>
#include <QMessageBox>
#include <QSpacerItem>
#include <QSignalMapper>
#include <QSharedPointer>

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow {
  Q_OBJECT
 public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

  static QString extensionForPID(Word pid);

public slots:
  void doReset();
  void doResetForOpen();
  void doResetBootloader();
  void doneAudioRefresh();
  void doneMetersRefresh();

private slots:
  void on_actionClose_triggered();
  void on_actionCommit_Changes_to_FLASH_triggered();
  void on_actionRestore_from_FLASH_triggered();
  void on_actionRestore_to_Factory_Defaults_triggered();
  void on_actionReset_triggered();
  void on_actionReset_to_Bootloader_Mode_triggered();
  void on_actionSave_triggered();
  void on_actionOpen_triggered();
  void on_actionFirmwareConfig_triggered();
  void on_actionOpenPresetsFolder_triggered();
  void on_actionReread_Settings_triggered();

  void deviceInfo_triggered();
  void midiInfo_triggered();
  void portRouting_triggered();
  void portFilter_triggered();
  void channelRemap_triggered();
  void ccFilter_triggered();
  void ccRemap_triggered();
  void audioInfo_triggered();
  void audioPatchbay_triggered();
  void audioMixerControl_triggered();
  void audioControl_triggered();

  void queryCompleted(Screen screen, CommandQList foundItems);

  void requestRefresh();
  void rereadAudioControls();
  void rereadMeters();

  void enableDevInfoAction();
  void enableMidiInfoAction();
  void enablePortRoutingAction();
  void enablePortFilterAction();
  void enableChannelRemapAction();
  void enableCCFilterAction();
  void enableCCRemapAction();
  void enableAudioInfoAction();
  void enableAudioPatchbayAction();
  void enableAudioMixerAction();
  void enableAudioControlAction();

  void updateFileMenu();
  void updateDeviceMenu();

  void writingStarted(int max);
  void writingProgress(int value);
  void writingCompleted();

  void onTimeout();

  void closeEvent(QCloseEvent *event);

  void on_actionAbout_triggered();

  void closeAllInTime();

  void on_actionIConnectivity_Website_triggered();

  void on_actionSoftware_Manual_triggered();

  void on_actionRegister_triggered();

  void on_actionUpgrade_Firmware_triggered();

  void on_actionUpgrade_Firmware_From_Local_Drive_triggered();


 signals:
  void refreshAll();
  void refreshAudio();
  void refreshMeters();

  void closeAll();

 private:
  void showConnectionError();
  void showErrorWithTitleAndMsg(QString title, QString msg, bool bShutdownApp); //zx, 2017-04-18
  void clearToolBar();
  void generateToolBar(GeneSysLib::CommandList commandList);
  void generateToolBarMidi(GeneSysLib::CommandList commandList);
  void generateToolBarAudio(GeneSysLib::CommandList commandList);

  void createActions();
  void startSave(Screen screen);
  void ackCallback(GeneSysLib::CmdEnum, GeneSysLib::DeviceID, Word,
                   GeneSysLib::commandData_t commandData);

  void writeSettings();
  void readSettings();

  static const int kButtonDisableTime;
  static const int kShortButtonDisableTime;

  GeneSysLib::CommPtr comm;

  DeviceInfoPtr currentDevice;

  QAction *audioInfoAction;
  QAction *audioPatchbayAction;
  QAction *audioMixerAction;
  QAction *audioControlAction;
  QAction *ccFilterAction;
  QAction *ccRemapAction;
  QAction *channelRemapAction;
  QAction *devInfoAction;
  QAction *midiInfoAction;
  QAction *portFilterAction;
  QAction *portRoutingAction;

  QSharedPointer<QProgressDialog> progressDialog;
  QSharedPointer<QMessageBox> resettingMessageBox;

  QString fileName;

  QSpacerItem *spacerItem;
  QProgressBar *progressBar;

  QSharedPointer<Ui::MainWindow> ui;

  DeviceInformationDialog* devInfoDialog;

  bool basicMode;
  bool connected;

  //Re-factory device selection UI functions, zx, 2017-04-06
  bool m_FirstSelectedDevice;

  long ackHandler;

  QString continuedOpeningFileName;
  bool rebootMe;

  QTimer *refreshAudioControlsTimer;
  QTimer *refreshMetersTimer;
  void clearLayout(QLayout *layout);
  bool ensureAudioInfoSave();

  std::queue<Bytes> sysexMessages;

  std::set<GeneSysLib::Command::Enum> getPreRebootCommands();

  //Re-factory device selection UI functions, zx, 2017-04-06
  void ReconnectPreviousOpenDevice(DeviceInfoPtr prevDevice);

  //Re-factory warning message UI, zx, 2017-04-21
  enum WARNMSGTYPE {
    WMSG_UNSPECIFIED = 0,
    WMSG_POORNETWORK4FIRMWAREUPDATE,
    WMSG_PRESETSAVEERROR,
    WMSG_PRESETLOADERROR,
    WMSG_DEVICECONNECTIONLOST
  };
  WARNMSGTYPE m_WarningState;

};  // MainWindow

#endif  // MAINWINDOW_H
