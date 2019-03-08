/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef MIXERCHANNELWIDGET_H
#define MIXERCHANNELWIDGET_H

#include "LibTypes.h"
#include "DeviceInfo.h"

#ifndef Q_MOC_RUN
#include <boost/shared_ptr.hpp>
#endif
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QDial>
#include <QPushButton>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>

#include "IRefreshWidget.h"
#include "MixerInterface.h"
#include "MixerInputInterface.h"
#include "MixerOutputInterface.h"

#include "QClickyDial.h"
#include "QClickySlider.h"
#include "QClickyDbLabel.h"
#include "IQClickyDbLabelAcceptor.h"

using boost::shared_ptr;

/*  +-------------- MixerWidget
 *  |    +--------- MixerPortWidget
 *  V    |    +---- containing widget (this)
 * +-----V----|------------------------------+
 * |+--[MPW]--V------------++--[MPW]--------+|
 * ||+------+--//--+------+||+------+--//--+||
 * |||      |      |      ||||      |      |||
 * ||| MCW  | MCW  | MCW  |||| MCW  | MCW  |||
 * |||      |      |      ||||      |      |||
 * ||+------+--//--+------+||+------+--//--+||
 * |+----------------------++---------------+|
 * +-----------------------------------------+
 */
enum MixerType
{
  out, in
};

class MixerPortWidget;

class MixerChannelWidget : public QWidget, public IRefreshWidget, IQClickyDbLabelAcceptor {
 Q_OBJECT
 public:
  static const double dbConversionFactor;// = 1.0 / 256.0;
  explicit MixerChannelWidget(DeviceInfoPtr device, Word audioPortID,
                                   Byte mixerInputNumber, Byte mixerOutputNumber,
                                   MixerType mixerType, MixerPortWidget* parent = 0);
  virtual ~MixerChannelWidget();
  void notifyChannelChange();

  static double pixelsToDb(Word pixels);
  static int16_t pixelsToIntForICA(Word pixels);
  static Word toPixelsFromICA(int16_t theInt);
  void labelDone(double value);

public slots:
  void updateSoloLightPushButtonValue();
  void turnOnClipping1();
  void turnOffClipping1();
  void turnOnClipping2();
  void turnOffClipping2();

private slots:
 void volumeSliderChanged(int state);
 void panDialChanged(int state);
 void soloDialChanged(int state);
 void muteStateChanged(bool state);
 void stereoLinkStateChanged(bool state);
 void invertStateChanged(bool state);
 void rightInvertStateChanged(bool state);
 void soloStateChanged(bool state);
 void soloPFLStateChanged(bool state);
 void channelDropDownChanged(int state);
 void channelDropDownChangedLinked(int state);

 void channelConfigPressed();

 void refreshWidget();
 void refreshMeters();
private:
  void buildAll();
  void buildChannelNameLabel();
  void buildVolumeSlider();
  void buildPanDial();
  void buildSoloDial();
  void buildMutePushButton();
  void buildSoloPushButton();
  void buildSoloPFLPushButton();
  void buildInvertPushButton();
  void buildRightInvertPushButton();

  void updateAll();
  void updateMeters();
  void updateChannelNameLabel();
  void updateVolumeValue();
  void updatePanValue();
  void updateMuteValue();
  void updateStereoLinkValue();
  void updateSoloPushButtonValue();
  void updateSoloDialValue();
  void updateSoloPFLValue();
  void updateInvertValue();
  void updateRightInvertValue();

  void setProgressBarColor(QProgressBar *pb, double value);

  QComboBox *channelDropdown;
  QComboBox *channelDropdownLinked;
  QPushButton *outputConfigPushButton;
  QPushButton *outputConfigPushButtonLinked;
  QClickySlider *volumeSlider;
  QClickyDbLabel *currentVolumeLabel;
  QClickyDial *panDial;
  QClickyDial *soloDial;
  QLabel *currentPanLabel;
  QLabel *currentSoloLabel;
  QPushButton *mutePushButton;
  QPushButton *stereoLinkPushButton;
  QPushButton *invertPushButton;
  QPushButton *rightInvertPushButton;
  QPushButton *soloPushButton;
  QPushButton *soloLightPushButton;
  QPushButton *soloPFLPushButton;
  QPushButton *clippingPushButton1;
  QPushButton *clippingPushButton2;

  QTimer *clippingTimer1;
  QTimer *clippingTimer2;

  QProgressBar* meterBar1;
  QProgressBar* meterBar2;

  bool soloLightOn;

  QHBoxLayout *panBoxLayout;
  QWidget *panBox;

  QHBoxLayout *invertBoxLayout;
  QWidget *invertBox;

  QList<QPair<int,int>> allAvailableInChannels;
  QList<QString> portNames;

  QWidget *hVolBoxWidget;
  bool disabled;


  MixerInterface *mixerInterface;
  MixerInputInterface *mixerInputInterface;
  MixerOutputInterface *mixerOutputInterface;

  bool stereoLinked;
  bool outputStereoLinked;
  int otherChannel;

  DeviceInfoPtr device;
  Word audioPortID;
  Byte mixerInputNumber;
  Byte mixerOutputNumber;
  MixerType mixerType;
  MixerPortWidget* parent;
};

#endif // MIXERCHANNELWIDGET_H
