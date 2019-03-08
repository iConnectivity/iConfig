/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <UIKit/UIKit.h>
#import "ICMixerStrip.h"
#import "ICMixerMixerGroup.h"
#import "ICMixerAnalogGroup.h"
#import "ICMixerSelector.h"

@interface ICMixerViewController : UIViewController {
  DeviceInfoPtr device;
  CommPtr comm;

  ICMixerAnalogGroup *inputGroup;
  ICMixerAnalogGroup *outputGroup;
  ICMixerSelector *mixerSelector;

  UILabel *inputLabel;
  UIView *inputView;
  UIView *inputSpacer;

  UILabel *outputLabel;
  UIView *outputView;
  UIView *outputSpacer;
}

- (id)initWithCommunicator:(CommPtr)m_comm device:(DeviceInfoPtr)m_device;
- (void) updateMeters;
- (void)resetView;

@end
