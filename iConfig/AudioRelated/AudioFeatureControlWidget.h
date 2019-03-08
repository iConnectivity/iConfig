/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __AUDIOFEATURECONTROLWIDGET_H__
#define __AUDIOFEATURECONTROLWIDGET_H__

#include "IAudioControlFeatureSource.h"
#include "LibTypes.h"

#ifndef Q_MOC_RUN
#include <boost/shared_ptr.hpp>
#endif
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QProgressBar>
#include <QHBoxLayout>

#include "IRefreshWidget.h"
#include "QClickyDial.h"
#include "QClickyDbLabel.h"
#include "IQClickyDbLabelAcceptor.h"
#include "DeviceInfo.h"

using boost::shared_ptr;

/*   +-------------------+ <- containing widget (this)
 *   |+-----------------+|
 *   ||     QLabel      |<--- channel name label
 *   |+=================+|
 *   ||                 ||
 *   ||                 ||
 *   ||                 ||
 *   ||     QSlider     |<--- volume slider
 *   ||                 ||
 *   ||                 ||
 *   ||                 ||
 *   |+=================+|
 *   ||    QPushButton    |<--- mute PushButton
 *   |+=================+|
 *   ||    QPushButton    |<--- phantom power PushButton
 *   |+=================+|
 *   || QVerticalSpacer |<--- vertical spacer
 *   |+=================+|
 *   ||    QPushButton    |<--- high impedence label (either "HiZ" or "")
 *   |+-----------------+|
 *   +-------------------+
 */

class AudioChannelsControlWidget;

class AudioFeatureControlWidget : public QWidget, public IRefreshWidget, IQClickyDbLabelAcceptor {
 Q_OBJECT
 public:
  explicit AudioFeatureControlWidget(
      IAudioControlFeatureSourcePtr audioFeatureSoure, Word audioPortID, Byte channelID, DeviceInfoPtr device,
      QWidget *parent = 0, int totalOutputChannelInPort = 6);  //Bugfixing for PlayAudio, zx-03-02


 public slots:
  void turnOnClipping1();
  void turnOffClipping1();
  void turnOnClipping2();
  void turnOffClipping2();
  void labelDone(double value);

private slots:
  void volumeSliderChanged(int state);
  void panDialChanged(int state);
  void muteStateChanged(bool state);
  void stereoLinkStateChanged(bool state);
  void phantomPowerStateChanged(bool state);
  void highImpedanceStateChanged(bool state);
  void rightPhantomPowerStateChanged(bool state);
  void rightHighImpedanceStateChanged(bool state);

  void refreshWidget();
  void refreshMeters();

  void channelConfigPressed();
private:
  void buildAll();
  void buildChannelNameLabel();
  void buildVolumeSlider();
  void buildPanDial();
  void buildMutePushButton();
  void buildStereoLinkPushButton();
  void buildPhantomPowerPushButton();
  void buildHighImpedancePushButton();
  void buildClippingPushButton();

//Bugfixing for PlayAudio, zx, 2017-03-01
public:
  void updateAll();
  void updateChannelNameLabel();
  void updateVolumeValue();
  void updatePanValue();
  void updateMuteValue();
  void updateStereoLinkValue();
  void updatePhantomValue();
  void updateHighImpedanceValue();
  void updateRightPhantomValue();
  void updateRightHighImpedanceValue();
  void updateMeters();

  void setProgressBarColor(QProgressBar* pb, double value);

  void setWidgetVisibility();

  Byte getPairedChannelID(Byte channelID);

  IAudioControlFeatureSourcePtr audioFeatureSource;
  Byte channelID;
  Word audioPortID;

  QList<QPair<int,int>> allAvailableInChannels;
  QList<QString> portNames;

  QLabel *channelNameLabel;
  QSlider *volumeSlider;
  QClickyDbLabel *currentVolumeLabel;
  QClickyDial *panDial;
  QLabel *currentPanLabel;
  QPushButton *mutePushButton;
  QPushButton *stereoLinkPushButton;
  QPushButton *phantomPowerPushButton;
  QPushButton *highImpedancePushButton;
  QPushButton *rightPhantomPowerPushButton;
  QPushButton *rightHighImpedancePushButton;
  QPushButton *clippingPushButton1;
  QPushButton *clippingPushButton2;

  QPushButton *outputConfigPushButton;
  QPushButton *outputConfigPushButtonLinked;

  QTimer *clippingTimer1;
  QTimer *clippingTimer2;

  QProgressBar *meterBar1;
  QProgressBar *meterBar2;

  QHBoxLayout *panBoxLayout;
  QWidget *panBox;

  AudioChannelsControlWidget* parentCCW;
  DeviceInfoPtr device;

  bool visibilitySet;
  bool stereoLinked;

  int         m_TotalOutputChannelInPort;  //Bugfixing for PlayAudio, zx-03-02

};

#endif  // __AUDIOFEATURECONTROLWIDGET_H__
