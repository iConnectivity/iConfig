/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <UIKit/UIKit.h>
#import "ICMIDIPatchbayDelegate.h"

enum popupType {
  PortFilters,
  ChannelRemap,
  ControllerFilters,
  ControllerRemap
};

enum popupPuts {
  Input = 1,
  Output
};

enum pickerTypes {
  Port,
  Controller
};

@interface ICMIDIPatchbayGridViewController : UIViewController <UIPickerViewDataSource, UIPickerViewDelegate> {
  IBOutlet UIImageView *imageView;
  IBOutlet UIView *grayView;
  IBOutlet UIView *popupView;
  IBOutlet UILabel *popupTitle;
  IBOutlet UIButton *popupPortButton;

  IBOutlet UIButton *bubbleButton;
  
  IBOutlet UIBarButtonItem *portFiltersButton;
  IBOutlet UIBarButtonItem *channelRemapButton;
  IBOutlet UIBarButtonItem *controllerFiltersButton;
  IBOutlet UIBarButtonItem *controllerRemapButton;

  IBOutlet UIView *popupSettingsView;
  IBOutlet UIView *popupSettingsTop;
  IBOutlet UIView *popupSettingsLeft;
  bool popupShowing;
  bool firstTouchUpSinceLPGR;

  popupType currentPopupType;

  Word popupDevicePort;
  Word popupPort;
  popupPuts popupPut;

  pickerTypes pickerType;

  NSMutableArray *popupButtonRows;
  NSArray *popupChannelRemapChannelRow;
  NSMutableArray *popupControllerIDCol;
  NSMutableArray *popupControllerIDSrcCol;
  NSMutableArray *popupControllerIDDstCol;
  UIButton* lastButtonToggled;

  UIButton* currentButtonBeingPickered;

  NSArray* controllerNames;
}

@property(nonatomic, strong) ICMIDIPatchbayDelegate* midiPatchbayDelegate;

- (id) initWithDelegate:(ICMIDIPatchbayDelegate*)midiPatchbayDelegate;
- (IBAction)popupTabChanged:(id)sender;
- (IBAction)popupPortTouched:(id)sender;
- (IBAction)bubbleButtonTouched:(id)sender;

@end
