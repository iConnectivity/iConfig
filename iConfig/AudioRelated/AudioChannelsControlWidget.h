/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __AUDIOCHANNELSCONTROLWIDGET_H__
#define __AUDIOCHANNELSCONTROLWIDGET_H__

#include "AudioFeatureControlWidget.h"
#include "IAudioControlFeatureSource.h"
#include "../MixerRelated/MixerWidget.h"

#include <QGroupBox>
#include <QList>
#include <QSignalMapper>

struct IRefreshWidget;

/*  +-------------------------------------- containing widget (this)
 *  |               +------------------+--- audio feature controls
 *  V               |                  |
 * +-[QGroupWidget]-V------------------V-------------------------+
 * |+---------------------------+   +---------------------------+|
 * || AudioFeatureControlWidget |...| AudioFeatureControlWidget ||
 * |+---------------------------+   +---------------------------+|
 * +-------------------------------------------------------------+
 */

class AudioChannelsControlWidget : public QWidget, public IRefreshWidget {
  Q_OBJECT
 public:
  explicit AudioChannelsControlWidget(
      IAudioControlFeatureSourcePtr audioControlFeatureSource,
      DeviceInfoPtr device,
      Word audioPortID,
      MixerWidget *parent = 0,
      int nTotalOutputChannel = 6); //Bugfixing for PlayAudio, zx-03-02

  void refreshWidget();
  void refreshMeters();
  AudioFeatureControlWidget* getAudioFeatureControlWidget(Byte channelID);

 signals:

 public slots:

  void notifyStereoLinkHappened();
private slots:
  void finishStereoLinkHappened();
  void linkButtonClicked(int whichButton);

private:
  void buildAll();
  void buildChannelWidgets();

  MixerWidget* parent;
  IAudioControlFeatureSourcePtr audioControlFeatureSource;
  std::vector<AudioFeatureControlWidget*> audioFeatureControls;

  std::vector<IRefreshWidget*> refreshList;
  void remove(QLayout *layout);

  DeviceInfoPtr device;
  Word audioPortID;

  int         m_TotalOutputChannelInPort;  //Bugfixing for PlayAudio, zx-03-02

  QSignalMapper* signalMapper;
};

#endif  // __AUDIOCHANNELSCONTROLWIDGET_H__
