/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "Communicator.h"
#import "DeviceInfo.h"
#import "ICIndicatorButton.h"

#import <UIKit/UIKit.h>

#define kRequestResetNotification @"RequestResetNotification"

@class ICViewController;
@class ICMainMenuProvider;

@interface ICViewController
    : UIViewController<UIScrollViewDelegate, UIActionSheetDelegate,
                       UIAlertViewDelegate> {
  IBOutlet UIScrollView *scrollView;

  NSMutableArray *providers;
  __weak IBOutlet UIToolbar *myToolbar;

  NSArray *deviceFunctionArray;

  NSTimer *updateTimer;

  long ackHandlerID;

  bool isPortrait;
  bool isLandscape;
  bool bouncedAlready;


  UIViewController* tempWithViewController;
  BOOL tempWithAutoLayout;
  BOOL tempStartMeterUpdate;
}

@property(strong, nonatomic) NSArray *providerButtons;
@property(assign, nonatomic) ICMainMenuProvider* provider;
@property(assign, nonatomic) GeneSysLib::CommPtr comm;
@property(assign, nonatomic) DeviceInfoPtr device;
@property(strong, nonatomic) NSMutableDictionary *storedUserInformation;
@property(strong, nonatomic) IBOutlet UIView *readingView;
@property(strong, nonatomic) IBOutlet UILabel *statusText;
@property(weak, nonatomic)
    IBOutlet UIBarButtonItem *deviceFunctionBarButtonItem;
@property(weak, nonatomic) IBOutlet UIBarButtonItem *configurationBarButtonItem;

@property(strong, nonatomic) IBOutlet UIView *fileNameView;
@property(strong, nonatomic) IBOutlet UITextField *fileNameTextField;
@property(strong, nonatomic) IBOutlet UIView *BackColorView;

@property(strong, nonatomic) UIViewController* mainView;

@property (strong, nonatomic) IBOutlet UIBarButtonItem *deviceInfoButton;
@property (strong, nonatomic) IBOutlet UIBarButtonItem *audioInfoButton;
@property (strong, nonatomic) IBOutlet UIBarButtonItem *audioPatchbayButton;
@property (strong, nonatomic) IBOutlet UIBarButtonItem *audioMixerButton;
@property (strong, nonatomic) IBOutlet UIBarButtonItem *midiInfoButton;
@property (strong, nonatomic) IBOutlet UIBarButtonItem *midiPatchbayButton;

- (IBAction)onDeviceInfoButtonTouched:(id)sender;
- (IBAction)onAudioInfoButtonTouched:(id)sender;
- (IBAction)onAudioPatchbayButtonTouched:(id)sender;
- (IBAction)onAudioMixerButtonTouched:(id)sender;
- (IBAction)onMidiInfoButtonTouched:(id)sender;
- (IBAction)onMidiPatchbayButtonTouched:(id)sender;

+ (UIImage *)imageFromColor:(UIColor *)color;

- (id)initWithProvider:(ICMainMenuProvider *)provider
          communicator:(GeneSysLib::CommPtr)communicator
                device:(DeviceInfoPtr)deviceInfomation;

- (void)replaceMainView:(UIViewController *)withViewController;
- (void)replaceMainView:(UIViewController *)withViewController withAutoLayout:(BOOL)isAutoLayout;
- (void)replaceMainView:(UIViewController *)withViewController withAutoLayout:(BOOL)isAutoLayout startMeterUpdate:(BOOL)doStartAutoTimer;
- (void)continueReplaceMainView;
- (void)setAudioInfoActive;

- (void) stopUpdateTimer;
- (void) hideDrawer;

- (NSArray *)rectsBounds:(CGRect)bounds
                withRows:(NSUInteger)rows
         numberOfColumns:(NSUInteger)cols
               withSpans:(NSArray *)spans;
- (NSArray *)rectsWithBound:(CGRect)bounds
       numberOfSubdivisions:(NSUInteger)number;
- (NSArray *)splitRectangle:(CGRect)rectangle;

- (void)reloadData:(BOOL)animated;

- (IBAction)onSelectDevice:(id)sender;

- (IBAction)deviceFunctionsPressed;
- (IBAction)configurationFunctionsPressed;
- (IBAction)helpButtonPressed:(UIBarButtonItem *)sender;

- (void)switchDevices;
- (void)showReadingView;
- (void)hideReadingView;

- (void)showWritingView;
- (void)hideWritingView;
- (void)showSaving;
- (void)showLoading;

- (void)showOverlay:(NSString *)message;
- (void)hideOverlay;

- (void)showEnterFileNameView;
- (void)hideEnterFileNameView;

- (IBAction)saveButtonPressed;
- (IBAction)cancelButtonPressed;

- (void)startSaveQuery;
- (void)queryComplete:(NSNotification *)notification;

- (void)writeFileNamed:(NSString *)fileName;
- (NSString *)fileNameWithExtension:(NSString *)path;

- (void)onTimeout;

- (void)orientationChanged:(NSNotification *)aNotification;

@end
