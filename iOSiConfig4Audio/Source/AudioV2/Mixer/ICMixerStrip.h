/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <UIKit/UIKit.h>
#import "DeviceInfo.h"
#import "IOSKnobControl.h"

#import "MixerInterface.h"
#import "MixerInputInterface.h"
#import "MixerOutputInterface.h"
#import "ICAnalogInterface.h"

enum MixerType
{
  out, in
};

@interface ICMixerStrip : UIView {
  MixerType mixerType;
  bool stereoLinked;
  bool outputStereoLinked;
  bool isAudioStrip;

  DeviceInfoPtr device;
  CommPtr comm;

  Word audioPortID;
  Byte inputNumber;
  Byte outputNumber;
  Byte audioController;

  int meterMax;

  NSMutableArray *views;

  UIButton *channelNameButton;
  UIButton *muteButton;
  UIButton *phantomPowerButton;
  UIButton *highImpedenceButton;
  UIButton *soloButton;
  UIButton *soloPFLButton;

  UIButton *invertButton;
  UIButton *invertRightButton;
  UIView *invertButtonContainer;

  UISlider *volumeSlider;
  UIView *volumeSliderContainer;
  UIView *totalVolumeSliderContainer;
  UIProgressView *meterLeft;
  UIView *meterLeftContainer;
  UIProgressView *meterRight;
  UIView *meterRightContainer;
  UIView *meterContainer;

  IOSKnobControl *panDial;
  IOSKnobControl *soloDial;
  UIView *panDialContainer;
  UIView *soloDialContainer;
  UIView *panSoloContainer;

  UIButton *panTitleLabel;
  UIButton *panLabel;
  UIButton *soloLabel;

  UIView *panSoloTitleLabelContainer;
  UIView *panSoloLabelContainer;

  UIButton *volumeLabel;

  MixerInterface *mixerInterface;
  MixerInputInterface *mixerInputInterface;
  MixerOutputInterface *mixerOutputInterface;
  ICAnalogInterface *analogInterface;

  BOOL changingVolume, changingPan, changingSolo;
}

- (id)initWithCommunicator:(CommPtr)m_comm device:(DeviceInfoPtr)m_device isAnalog:(bool)m_isAnalog audioPortID:(Word)m_audioPortID inputNumber:(Byte)m_inputNumber outputNumber:(Byte)m_outputNumber;
- (bool) isStereoLinked;
- (bool) isStereoLinkable;
- (void) toggleStereoLink;
- (void) updateMeters;

@end
