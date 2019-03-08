/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <UIKit/UIKit.h>
#import "ICMixerStrip.h"

@class ICMixerViewController;

@interface ICMixerAnalogGroup : UIView {
  DeviceInfoPtr device;
  CommPtr comm;

  bool isInput;
  int numDisplayedPorts;
  NSMutableArray *mixerStrips;
  NSMutableArray *stereoLinkButtons;

  UIView *topView;
  UIView *bottomView;

  __weak ICMixerViewController *topMVC;
}

- (id)initWithCommunicator:(CommPtr)m_comm device:(DeviceInfoPtr)m_device isInput:(bool)m_isInput mvc:(ICMixerViewController*)mvc;
- (int) postReadyInit;
- (void) updateMeters;

@end
