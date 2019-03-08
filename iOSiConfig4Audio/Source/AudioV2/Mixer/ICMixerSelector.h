/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "DeviceInfo.h"

#import "MixerInterface.h"
#import "MixerInputInterface.h"
#import "MixerOutputInterface.h"

#import <UIKit/UIKit.h>
#import "ICMixerMixerGroup.h"

@class ICMixerViewController;

@interface ICMixerSelector : UIView {
  DeviceInfoPtr device;
  CommPtr comm;

  MixerInterface* mixerInterface;
  MixerInputInterface* mixerInputInterface;
  MixerOutputInterface* mixerOutputInterface;

  Word audioPortID;
  Byte outputNumber;
  
  UILabel *mixerLabel;
  UIView *mixerButtonsView;
  NSMutableArray *mixerButtons;
  ICMixerMixerGroup* mixerGroup;
  __weak ICMixerViewController* topMVC;
}

- (id)initWithCommunicator:(CommPtr)m_comm device:(DeviceInfoPtr)m_device currentAudioPortID:(Word)audioPortID currentOutputNumber:(Byte)outputNumber mvc:(ICMixerViewController*)mvc;
- (int) postReadyInit;
- (void) updateMeters;

@end
