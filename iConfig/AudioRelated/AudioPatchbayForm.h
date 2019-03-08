/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef AUDIOPATCHBAYFORM_H
#define AUDIOPATCHBAYFORM_H

#include "AudioPortPatchbay.h"
#include "Communicator.h"
#include "DeviceInfo.h"
#include "RefreshObject.h"

#include <QTreeWidgetItem>
#include <QSignalMapper>
#include <QComboBox>
#include <QSharedPointer>

namespace GeneSysLib {
struct AudioInfo;
struct AudioCfgInfo;
struct AudioPortCfgInfo;
struct AudioPortInfo;
}

namespace Ui { class AudioPatchbayForm; }

class AudioPatchbayForm : public RefreshObject {
  Q_OBJECT
 public:
  explicit AudioPatchbayForm(DeviceInfoPtr deviceReference,
                             QWidget *parent = 0);
  ~AudioPatchbayForm();

  static const QString DefaultWindowTitle;
  static const float kBatchTime;
  static QList<GeneSysLib::CmdEnum> Query();

 signals:
  void informationAvailable();
  void updateMessage(QString message, int timeout);

 private slots:
  void outputChannelChanged(int comboBoxID);

 private:
  void createTree();
  void createAudioPortBranch(GeneSysLib::AudioCfgInfo &audioCfgInfo,
                             int &comboBoxID, const QStringList &outputOptions,
                             GeneSysLib::AudioPortInfo &audioPortInfo);
  void addPortComboBox(QTreeWidgetItem *parent,
                       GeneSysLib::AudioPortInfo &audioPortInfo,
                       const GeneSysLib::AudioPortPatchbay &portPatchbay,
                       const QStringList &outputOptions, int inChannel,
                       int &comboBoxID);
  void ackCallback(GeneSysLib::CmdEnum command, GeneSysLib::DeviceID deviceID,
                   Word transID, GeneSysLib::commandData_t commandData);
  QStringList generateOutputOptions(GeneSysLib::AudioCfgInfo &audioCfgInfo);

  bool eventFilter(QObject *obj, QEvent *event);
  void refreshWidget();

  DeviceInfoPtr device;

  std::map<GeneSysLib::CmdEnum, long> registeredHandlerIDs;

  std::queue<Bytes> sysexMessages;

  QSharedPointer<Ui::AudioPatchbayForm> ui;

  QSignalMapper *outputSignalMapper;
  std::map<int, QComboBox *> idComboBoxMap;
  std::map<Word, std::map<int, int> > portToChannelToChoiceMap;
  std::map<int, std::pair<Word, int> > choiceToPortChannelPairMap;
  std::map<int, std::pair<Word, int> > comboBoxToInPortIDChannelPairMap;
};

#endif  // AUDIOPATCHBAYFORM_H
