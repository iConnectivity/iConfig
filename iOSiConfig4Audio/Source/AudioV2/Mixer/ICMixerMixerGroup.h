/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <UIKit/UIKit.h>
#import "ICMixerStrip.h"
#import "MixerInterface.h"
#import "MixerInputInterface.h"
#import "MixerOutputInterface.h"

@class ICMixerViewController;

@interface ICMixerMixerGroup : UIView {
  DeviceInfoPtr device;
  CommPtr comm;

  MixerInterface* mixerInterface;
  MixerInputInterface* mixerInputInterface;
  MixerOutputInterface* mixerOutputInterface;

  NSMutableArray *mixerStrips;
  NSMutableArray *stereoLinkButtons;

  Word audioPortID;
  Word outputNumber;

  UIView *topView;
  UIView *bottomView;

  int numDisplayedPorts;

  __weak ICMixerViewController *topMVC;
}

- (id) initWithCommunicator:(CommPtr)m_comm device:(DeviceInfoPtr)m_device mvc:(ICMixerViewController*)mvc;
- (int) postReadyInit;
- (void) setMixerAudioPort:(Word)audioPort outputNumber:(Byte)output;
- (void) redraw;
- (void) updateMeters;

@end
