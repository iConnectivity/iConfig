/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <UIKit/UIKit.h>
#import "ICBaseInfoViewController.h"
#include "Communicator.h"
#include "DeviceInfo.h"
#include "AudioGlobalParm.h"
#include "AudioDeviceParm.h"
#include "AudioPortParm.h"
#include "AudioClockParm.h"
#include "MixerParm.h"
#include "MixerPortParm.h"

using namespace GeneSysLib;

@interface ICAudioInfoViewControllerV2
    : ICBaseInfoViewController<UIAlertViewDelegate> {
  UIBarButtonItem *mainMenuButton;
  UIBarButtonItem *commitButton;
  UIAlertView *confirmAlert;
  UIAlertView *commitConfirmAlert;
  UIAlertView *commitCompleteAlert;
  UIAlertView *commitFailedAlert;
  BOOL change;
  BOOL justCanceled;

  GeneSysLib::CommPtr comm;
  DeviceInfoPtr device;

  AudioGlobalParm oldAudioGlobalParm;
  std::vector<AudioDeviceParm> oldAudioDeviceParms;
  std::vector<AudioPortParm> oldAudioPortParms;
  AudioClockParm oldAudioClockParm;
  MixerParm oldMixerParm;
  MixerPortParm oldMixerPortParm;
}

- (id)initWithCommunicator:(GeneSysLib::CommPtr)comm
                    device:(DeviceInfoPtr)device;

- (void)onMainMenuPressed;
- (void)onCommitPressed;
- (bool)confirm;

- (void)registerExclusiveAckHandler;

- (void)alertView:(UIAlertView *)alertView
    clickedButtonAtIndex:(NSInteger)buttonIndex;

@end
