/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ACK.h"
#import "BytesCommandData.h"
#import "CommandData.h"
#import "CommandDefines.h"
#import "CommandList.h"
#import "Communicator.h"
#import "DeviceID.h"
#import "DevicePID.h"
#import "ICDeviceSelectionViewController.h"
#import "ICFileBrowserViewController.h"
#import "ICHelpViewController.h"
#import "ICMIDIIndexTableViewController.h"
#import "ICMixerViewController.h"
#import "ICRunOnMain.h"
#import "ICViewController.h"
#import "ICMainMenuProvider.h"
#import "ICMenuViewController.h"
#import "MyAlgorithms.h"
#import "Reset.h"
#import "ResetList.h"
#import "SaveRestore.h"
#import "SaveRestoreList.h"
#import "SysexCommand.h"
#import "UIImageTintCategory.h"
#import "UIViewController+MMDrawerController.h"
#import "ICBaseInfoViewController.h"
#import "ICAudioInfoViewControllerV2.h"
#import "ICAudioInfoViewControllerV1.h"

#import <CoreGraphics/CoreGraphics.h>
#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>

using namespace MyAlgorithms;
using namespace GeneSysLib;

#define ANIMATE_OUT_TIME 0.2
#define ANIMATE_IN_TIME 0.15
#define BORDER_WIDTH 3.5

#define INDICATOR_WIDTH 24.0
#define INDICATOR_HEIGHT 24.0
#define INDICATOR_OFFSET 4.0

@interface ICViewController () {
  UIActionSheet *deviceFunctionActionSheet;
  UIActionSheet *configurationActionSheet;
  UIActionSheet *helpActionSheet;

  UIAlertView *saveToFlashAlertView;
  UIAlertView *restoreToFlashAlertView;
  UIAlertView *restoreToDefaultsAlertView;
  UIAlertView *resetDeviceAlertView;
  UIAlertView *saveConfigurationAlertView;
  UIAlertView *loadConfigurationAlertView;
  UIAlertView *communicationErrorAlert;

  UIAlertView *overrideFileAlertView;

  UIAlertView *switchDeviceAlertView;

  UIAlertView *invalidFileNameAlertView;

}

- (void)handleDeviceFunctionActionSheet:(NSInteger)buttonIndex;
- (void)handleConfigurationActionSheet:(NSInteger)buttonIndex;
- (void)handleHelpActionSheet:(NSInteger)buttonIndex;

- (void)handleSaveToFlashAlertView:(NSInteger)buttonIndex;
- (void)handleRestoreToFlashAlertView:(NSInteger)buttonIndex;
- (void)handleRestoreToDefaultsAlertView:(NSInteger)buttonIndex;
- (void)handleResetDeviceAlertView:(NSInteger)buttonIndex;
- (void)handleOverrideFileAlertView:(NSInteger)buttonIndex;
- (void)handleSwitchDeviceAlertView:(NSInteger)buttonIndex;
- (void)handleCommunicationErrorAlertView:(NSInteger)buttonIndex;

- (void)requestReset:(NSNotification *)notification;

@end

@implementation ICViewController

@synthesize providerButtons;
@synthesize mainView;

- (void)resetBarButtonTints {
  [_deviceInfoButton setTintColor:nil];
  [_audioInfoButton setTintColor:nil];
  [_audioPatchbayButton setTintColor:nil];
  [_audioMixerButton setTintColor:nil];
  [_midiInfoButton setTintColor:nil];
  [_midiPatchbayButton setTintColor:nil];
}

- (IBAction)onDeviceInfoButtonTouched:(id)sender {
  [self resetBarButtonTints];
  [_deviceInfoButton setTintColor:[UIColor redColor]];
  [self.provider onButtonDown:self index:0];
}

- (IBAction)onAudioInfoButtonTouched:(id)sender {
  [self resetBarButtonTints];
  [_audioInfoButton setTintColor:[UIColor redColor]];
  [self.provider onButtonDown:self index:1];
}

- (IBAction)onAudioPatchbayButtonTouched:(id)sender {
  [self resetBarButtonTints];
  [_audioPatchbayButton setTintColor:[UIColor redColor]];
  [self.provider onButtonDown:self index:2];
}

- (IBAction)onAudioMixerButtonTouched:(id)sender {
  [self resetBarButtonTints];
  [_audioMixerButton setTintColor:[UIColor redColor]];
  [self.provider onButtonDown:self index:3];
}

- (IBAction)onMidiInfoButtonTouched:(id)sender {
  [self resetBarButtonTints];
  [_midiInfoButton setTintColor:[UIColor redColor]];
  [self.provider onButtonDown:self index:4];
}

- (IBAction)onMidiPatchbayButtonTouched:(id)sender {
  [self resetBarButtonTints];
  [_midiPatchbayButton setTintColor:[UIColor redColor]];
  [self.provider onButtonDown:self index:5];
}

+ (UIImage *)imageFromColor:(UIColor *)color {
  const auto &rect = CGRectMake(0, 0, 1, 1);
  UIGraphicsBeginImageContext(rect.size);
  const auto &context = UIGraphicsGetCurrentContext();
  CGContextSetFillColorWithColor(
      context, [[color colorWithAlphaComponent:0.75f] CGColor]);
  CGContextFillRect(context, rect);
  auto *const img = UIGraphicsGetImageFromCurrentImageContext();
  UIGraphicsEndImageContext();
  return img;
}

- (id)initWithProvider:(ICMainMenuProvider *)provider
          communicator:(CommPtr)communicator
                device:(DeviceInfoPtr)deviceInformation {
  self =
      [super initWithNibName:(@"ICViewController_iPad")
                      bundle:nil];

  mainView = nil;

  if (self != nil) {
    self.storedUserInformation = [NSMutableDictionary dictionary];

    NSParameterAssert(provider);
    NSParameterAssert(communicator);
    NSParameterAssert(deviceInformation);

    self.provider = provider;
    self.comm = communicator;
    self.device = deviceInformation;

    auto *const functionTitleArray = [NSMutableArray array];
    auto *const tempDeviceFunctionArray = [NSMutableArray array];

    if (self.device->containsCommandData(Command::RetCommandList)) {
      const auto &commandList = self.device->get<CommandList>();

      if (commandList.contains(Command::SaveRestore)) {
        assert(self.device->containsCommandData(Command::RetSaveRestoreList));
        const auto &saveRestoreList = self.device->get<SaveRestoreList>();

        if (saveRestoreList.contains(SaveRestoreID::SaveToFlash)) {
          [functionTitleArray addObject:@"Save to Flash"];
          [tempDeviceFunctionArray
              addObject:[^{ [saveToFlashAlertView show]; } copy]];
        }
        if (saveRestoreList.contains(SaveRestoreID::RestoreFromFlash)) {
          [functionTitleArray addObject:@"Restore from Flash"];
          [tempDeviceFunctionArray
              addObject:[^{ [restoreToFlashAlertView show]; } copy]];
        }
        if (saveRestoreList.contains(SaveRestoreID::FactoryDefault)) {
          [functionTitleArray addObject:@"Restore to Defaults"];
          [tempDeviceFunctionArray
              addObject:[^{ [restoreToDefaultsAlertView show]; } copy]];
        }

        if (commandList.contains(Command::GetResetList)) {
          const auto &resetList = self.device->get<ResetList>();

          if (resetList.contains(BootMode::AppMode)) {
            [functionTitleArray addObject:@"Reset Device"];
            [tempDeviceFunctionArray
                addObject:[^{ [resetDeviceAlertView show]; } copy]];
          }
        }
      }
    }

    deviceFunctionActionSheet = [[UIActionSheet alloc] initWithTitle:nil
                                                            delegate:nil
                                                   cancelButtonTitle:nil
                                              destructiveButtonTitle:nil
                                                   otherButtonTitles:nil];
    for (NSString *const title in functionTitleArray) {
      [deviceFunctionActionSheet addButtonWithTitle:title];
    }
    [deviceFunctionActionSheet addButtonWithTitle:@"Cancel"];
    [deviceFunctionActionSheet
        setCancelButtonIndex:deviceFunctionActionSheet.numberOfButtons - 1];
    [deviceFunctionActionSheet setDelegate:self];
    deviceFunctionArray = tempDeviceFunctionArray;

    // Configuration Action Sheet
    configurationActionSheet =
        [[UIActionSheet alloc] initWithTitle:nil
                                    delegate:self
                           cancelButtonTitle:@"Cancel"
                      destructiveButtonTitle:nil
                           otherButtonTitles:@"Save Configuration",
                                             @"Load Configuration", nil];
    // Help Action Sheet
    helpActionSheet =
        [[UIActionSheet alloc] initWithTitle:nil
                                    delegate:self
                           cancelButtonTitle:@"Canel"
                      destructiveButtonTitle:nil
                           otherButtonTitles:@"Manual", @"Registration", nil];

    // Save to Flash
    saveToFlashAlertView = [[UIAlertView alloc]
            initWithTitle:@"Save to Flash"
                  message:@"Settings will be committed to Flash."
                 delegate:self
        cancelButtonTitle:@"Cancel"
        otherButtonTitles:@"Continue", nil];

    // Restore from Flash
    restoreToFlashAlertView = [[UIAlertView alloc]
            initWithTitle:@"Restore from Flash"
                  message:@"All settings will be restored from Flash."
                 delegate:self
        cancelButtonTitle:@"Cancel"
        otherButtonTitles:@"Continue", nil];

    // Restore to Defaults
    restoreToDefaultsAlertView = [[UIAlertView alloc]
            initWithTitle:@"Restore to Defaults"
                  message:@"All settings will be set to factory defaults."
                 delegate:self
        cancelButtonTitle:@"Cancel"
        otherButtonTitles:@"Continue", nil];

    // Reset Device
    resetDeviceAlertView = [[UIAlertView alloc]
            initWithTitle:@"Reset Device"
                  message:@"All uncommitted changes will be lost."
                 delegate:self
        cancelButtonTitle:@"Cancel"
        otherButtonTitles:@"Continue", nil];

    // Save Configuration
    saveConfigurationAlertView =
        [[UIAlertView alloc] initWithTitle:@"Save Configuration"
                                   message:nil
                                  delegate:self
                         cancelButtonTitle:@"Ok"
                         otherButtonTitles:nil];

    // Load Configuration
    loadConfigurationAlertView =
        [[UIAlertView alloc] initWithTitle:@"Load Configuration"
                                   message:nil
                                  delegate:self
                         cancelButtonTitle:@"Ok"
                         otherButtonTitles:nil];

    // Override file
    overrideFileAlertView =
        [[UIAlertView alloc] initWithTitle:@"Overwrite file?"
                                   message:nil
                                  delegate:self
                         cancelButtonTitle:@"Cancel"
                         otherButtonTitles:@"Continue", nil];

    // Switch Devices
    switchDeviceAlertView =
        [[UIAlertView alloc] initWithTitle:@"Switch Devices?"
                                   message:nil
                                  delegate:self
                         cancelButtonTitle:@"Cancel"
                         otherButtonTitles:@"Continue", nil];

    communicationErrorAlert =
        [[UIAlertView alloc] initWithTitle:@"Communication Lost."
                                   message:nil
                                  delegate:self
                         cancelButtonTitle:@"Ok"
                         otherButtonTitles:nil];

    invalidFileNameAlertView =
        [[UIAlertView alloc] initWithTitle:@"Invalid Filename"
                                   message:nil
                                  delegate:nil
                         cancelButtonTitle:@"Ok"
                         otherButtonTitles:nil];

    ackHandlerID = -1;

    UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
    isPortrait = UIDeviceOrientationIsPortrait(orientation);
    isLandscape = UIDeviceOrientationIsLandscape(orientation);

    [self.provider initializeProviderButtons:self];
    [self.provider onButtonDown:self index:0];

    [_deviceInfoButton setTintColor:[UIColor redColor]];
  }

  return self;
}

- (void)setAudioInfoActive {
  [self resetBarButtonTints];
  [_audioInfoButton setTintColor:[UIColor redColor]];}

- (void)continueReplaceMainView {
  [self replaceMainView:tempWithViewController withAutoLayout:tempWithAutoLayout startMeterUpdate:tempStartMeterUpdate];
}

- (void)replaceMainView:(UIViewController *)withViewController {
  [self replaceMainView:withViewController withAutoLayout:NO];
}

- (void)replaceMainView:(UIViewController *)withViewController withAutoLayout:(BOOL)isAutoLayout {
  [self replaceMainView:withViewController withAutoLayout:isAutoLayout startMeterUpdate:NO];
}

- (void)replaceMainView:(UIViewController *)withViewController withAutoLayout:(BOOL)isAutoLayout startMeterUpdate:(BOOL)doStartAutoTimer{
  bool confirm = true;

  if ([mainView isKindOfClass:[ICAudioInfoViewControllerV2 class]]) {
    ICAudioInfoViewControllerV2* v = (ICAudioInfoViewControllerV2*) mainView;
    confirm = [v confirm];

    if (!confirm) {
      tempWithViewController = withViewController;
      tempWithAutoLayout = isAutoLayout;
      tempStartMeterUpdate = doStartAutoTimer;
      [v onMainMenuPressed];
      return;
    }
  }
  else if ([mainView isKindOfClass:[ICAudioInfoViewControllerV1 class]]) {
    ICAudioInfoViewControllerV1* v = (ICAudioInfoViewControllerV1*) mainView;
    confirm = [v confirm];

    if (!confirm) {
      tempWithViewController = withViewController;
      tempWithAutoLayout = isAutoLayout;
      tempStartMeterUpdate = doStartAutoTimer;
      [v onMainMenuPressed];
      return;
    }
  }

  if (confirm) {
    for(UIView *subview in [scrollView subviews]) {
      [subview removeFromSuperview];
    }
    withViewController.view.frame = scrollView.bounds;
    mainView = withViewController;
    [scrollView addSubview:mainView.view];
    if (isAutoLayout) {
      NSArray *MAINVIEW_H = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|[mainView(width)]|"
                                                                    options:0
                                                                    metrics:@{@"width": [NSNumber numberWithFloat:scrollView.bounds.size.width]}
                                                                      views:@{ @"mainView" : mainView.view}];
      NSArray *MAINVIEW_V = [NSLayoutConstraint constraintsWithVisualFormat:@"V:|[mainView(height)]|"
                                                                    options:0
                                                                    metrics:@{@"height": [NSNumber numberWithFloat:scrollView.bounds.size.height]}
                                                                      views:@{ @"mainView" : mainView.view}];
      [scrollView addConstraints:MAINVIEW_V];
      [scrollView addConstraints:MAINVIEW_H];

      NSArray *SCROLLVIEW_H = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|[scrollView]|"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:@{ @"scrollView" : scrollView}];
      NSArray *SCROLLVIEW_V = [NSLayoutConstraint constraintsWithVisualFormat:@"V:|[scrollView]|"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:@{ @"scrollView" : scrollView}];


      [self.view addConstraints:SCROLLVIEW_H];
      [self.view addConstraints:SCROLLVIEW_V];
    }
    else {
      NSArray *MAINVIEW_H = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|[mainView(width)]|"
                                                                    options:0
                                                                    metrics:@{@"width": [NSNumber numberWithFloat:scrollView.bounds.size.width]}
                                                                      views:@{ @"mainView" : mainView.view}];
      NSArray *MAINVIEW_V = [NSLayoutConstraint constraintsWithVisualFormat:@"V:|[mainView(height)]|"
                                                                    options:0
                                                                    metrics:@{@"height": [NSNumber numberWithFloat:scrollView.bounds.size.height]}
                                                                      views:@{ @"mainView" : mainView.view}];
      [scrollView removeConstraints:MAINVIEW_V];
      [scrollView removeConstraints:MAINVIEW_H];

      NSArray *SCROLLVIEW_H = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|[scrollView]|"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:@{ @"scrollView" : scrollView}];
      NSArray *SCROLLVIEW_V = [NSLayoutConstraint constraintsWithVisualFormat:@"V:|[scrollView]|"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:@{ @"scrollView" : scrollView}];


      [self.view removeConstraints:SCROLLVIEW_H];
      [self.view removeConstraints:SCROLLVIEW_V];

      scrollView.translatesAutoresizingMaskIntoConstraints = YES;
    }

    if (doStartAutoTimer) {
      if (updateTimer) {
        [updateTimer invalidate];
        updateTimer = nil;
      }

      if (!updateTimer) {
          //zx, 2017-06-16
/*        updateTimer = [NSTimer timerWithTimeInterval:0.1
                                              target:self
                                            selector:@selector(updateMeters)
                                            userInfo:nil
                                             repeats:NO];

        [[NSRunLoop currentRunLoop] addTimer:updateTimer
                                     forMode:NSDefaultRunLoopMode];
*/
          updateTimer = [NSTimer scheduledTimerWithTimeInterval:0.1
                                                target:self
                                              selector:@selector(updateMeters)
                                              userInfo:nil
                                               repeats:NO];
        //NSLog(@"Added updateTimer");
      }
    }
    else {
      [self stopUpdateTimer];
    }
  }
}

- (void) stopUpdateTimer {
  if (updateTimer) {
    [updateTimer invalidate];
    updateTimer = nil;
  }
}

- (void) updateMeters {
  NSLog(@"Requesting update of Meters from iCA");
  self.device->rereadMeters();
}


- (void)viewWillAppear:(BOOL)animated {
  [super viewWillAppear:animated];

  self.deviceFunctionBarButtonItem.enabled = ([deviceFunctionArray count] != 0);
  self.configurationBarButtonItem.enabled = ([deviceFunctionArray count] != 0);

  [self.navigationController setNavigationBarHidden:YES animated:YES];

  [scrollView setContentInset:UIEdgeInsetsZero];
  const auto &scrollBounds = scrollView.bounds;
  [scrollView setBounds:CGRectMake(0, 0, scrollBounds.size.width,
                                   scrollBounds.size.height)];

  [((ICMenuViewController*)self.mm_drawerController.leftDrawerViewController) setICViewController:self];

  if (!bouncedAlready) {
    if (self.mm_drawerController.leftDrawerViewController != nil) {
      [self.mm_drawerController openDrawerSide:MMDrawerSideLeft animated:NO completion:^(BOOL){}];
    }
  }
}

- (void) hideDrawer {
  NSLog(@"hide drawer being called");
  if (self.mm_drawerController.leftDrawerViewController != nil) {
    [self.mm_drawerController closeDrawerAnimated:NO completion:^(BOOL){}];
  }
}

- (void)viewDidLayoutSubviews {
  [super viewDidLayoutSubviews];

  [scrollView setContentInset:UIEdgeInsetsZero];
  const auto &scrollBounds = scrollView.bounds;
  [scrollView setBounds:CGRectMake(0, 0, scrollBounds.size.width,
                                   scrollBounds.size.height)];
}

- (void)dealloc {
  [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)viewDidLoad {
  [super viewDidLoad];
  //[self addButtonSubviews:NO];

  [[NSNotificationCenter defaultCenter] removeObserver:self];

  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(onTimeout)
                                               name:kCommunicationTimeout
                                             object:nil];

  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(showLoading)
                                               name:@"writingStarted"
                                             object:nil];

  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(hideOverlay)
                                               name:@"writeCompleted"
                                             object:nil];

  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(queryComplete:)
                                               name:@"queryCompleted"
                                             object:nil];

  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(requestReset:)
                                               name:kRequestResetNotification
                                             object:nil];


}

- (void)viewDidUnload {
  //NSLog(@"viewDidUnload");
  myToolbar = nil;
  [self setBackColorView:nil];
  [self setConfigurationBarButtonItem:nil];
  [self setDeviceFunctionBarButtonItem:nil];
  [self setFileNameView:nil];
  [self setStatusText:nil];

  [super viewDidUnload];
  // Release any retained subviews of the main view.

  [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)viewDidAppear:(BOOL)animated {
  [super viewDidAppear:animated];

  [scrollView setContentInset:UIEdgeInsetsZero];
  const auto &scrollBounds = scrollView.bounds;
  [scrollView setBounds:CGRectMake(0, 0, scrollBounds.size.width,
                                   scrollBounds.size.height)];
  [scrollView flashScrollIndicators];

  [[NSNotificationCenter defaultCenter]
      addObserver:self
         selector:@selector(onDidEnterBackground)
             name:UIApplicationDidEnterBackgroundNotification
           object:nil];
  [[NSNotificationCenter defaultCenter]
      addObserver:self
         selector:@selector(onDidEnterBackground)
             name:UIApplicationWillResignActiveNotification
           object:nil];
  [[NSNotificationCenter defaultCenter]
      addObserver:self
         selector:@selector(onWillEnterForground)
             name:UIApplicationWillEnterForegroundNotification
           object:nil];
  [[NSNotificationCenter defaultCenter]
      addObserver:self
         selector:@selector(onWillEnterForground)
             name:UIApplicationDidBecomeActiveNotification
           object:nil];

  // Register ACK Handler ID
  const auto &ackHandler = [self](CmdEnum, DeviceID, Word,
                                  commandData_t commandData) {
    const auto &ack = commandData.get<ACK>();

    if (ack.errorCode() != 0) {
      // //NSLog(@"NACK for %04X", ack.commandID);
      // Show connection error
      // REFACTOR: rename onTimeout to onConnectionLost
      [self performSelectorOnMainThread:@selector(onTimeout)
                             withObject:nil
                          waitUntilDone:NO];
    } else {
      // //NSLog(@"ACK for %04X", ack.commandID);
      if (ack.commandID() == Command::SaveRestore) {
        // //NSLog(@"Save Restore ACK");
        [self showReadingView];
        self.device->rereadStored();
      }
    }
  };

  ackHandlerID = self.comm->registerHandler(Command::ACK, ackHandler);
  // //NSLog(@"Register ACK");

  if ([[UIDevice currentDevice] userInterfaceIdiom] ==
      UIUserInterfaceIdiomPad) {
    [[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
    [[NSNotificationCenter defaultCenter]
        addObserver:self
           selector:@selector(orientationChanged:)
               name:UIDeviceOrientationDidChangeNotification
             object:nil];
  }

  if (!bouncedAlready) {
    if (self.mm_drawerController != nil) {
      //NSLog(@"Have drawer");
      if (self.mm_drawerController.leftDrawerViewController != nil) {
        runOnMainAfter(0.5, ^{
          [self.mm_drawerController closeDrawerAnimated:YES completion:^(BOOL){}];
        });

      }
    }
    else {
      //NSLog(@"don't have drawer");
    }
  }
  bouncedAlready = true;

  if ([mainView respondsToSelector:@selector(refreshInfo)]) {
    [((ICBaseInfoViewController*)mainView) refreshInfo];
  }
}

- (void)viewWillDisappear:(BOOL)animated {
  [super viewWillDisappear:animated];

  [[NSNotificationCenter defaultCenter]
      removeObserver:self
                name:UIApplicationDidEnterBackgroundNotification
              object:nil];
  [[NSNotificationCenter defaultCenter]
      removeObserver:self
                name:UIApplicationWillResignActiveNotification
              object:nil];
  [[NSNotificationCenter defaultCenter]
      removeObserver:self
                name:UIApplicationWillEnterForegroundNotification
              object:nil];
  [[NSNotificationCenter defaultCenter]
      removeObserver:self
                name:UIApplicationDidBecomeActiveNotification
              object:nil];

  if ([[UIDevice currentDevice] userInterfaceIdiom] ==
      UIUserInterfaceIdiomPad) {
    [[NSNotificationCenter defaultCenter]
        removeObserver:self
                  name:UIDeviceOrientationDidChangeNotification
                object:nil];
  }

  if (ackHandlerID >= 0) {
    self.comm->unRegisterHandler(Command::ACK, ackHandlerID);
    // //NSLog(@"UnRegister ACK");
    ackHandlerID = -1;
  }
}

- (void)onDidEnterBackground {

}

- (void)onWillEnterForground {

}

- (BOOL)shouldAutorotateToInterfaceOrientation:
            (UIInterfaceOrientation)interfaceOrientation {
  const auto &isiPad = ([[UIDevice currentDevice] userInterfaceIdiom] ==
                        UIUserInterfaceIdiomPad);
  const auto &isPortait =
      UIInterfaceOrientationIsPortrait(interfaceOrientation);

  // should only rotate with an iPad or if the orientation is portrait
  return (isiPad || isPortait);
}

- (NSArray *)rectsBounds:(CGRect)bounds
                withRows:(NSUInteger)rows
         numberOfColumns:(NSUInteger)cols
               withSpans:(NSArray *)spans {
  auto *const result = [[NSMutableArray alloc] init];

  const auto &deltaX =
      (bounds.size.width + (((float)cols - 1.0f)) * BORDER_WIDTH) /
      (1.0f * cols);
  const auto &deltaY =
      (bounds.size.height + ((float)rows - 1.0f) * BORDER_WIDTH) /
      (1.0f * rows);

  NSUInteger i = 0;
  for (NSUInteger r = 0; r < rows; r++) {
    for (NSUInteger c = 0; c < cols; c++) {
      auto rect =
          CGRectMake(floorf(bounds.origin.x + c * (deltaX - BORDER_WIDTH)),
                     floorf(bounds.origin.y + r * (deltaY - BORDER_WIDTH)),
                     ceilf(deltaX), ceilf(deltaY));
      if ((spans) && (i < [spans count]) &&
          ([spans[i] isKindOfClass:[NSNumber class]])) {
        NSUInteger span = [spans[i] intValue];
        c += span - 1;
        rect.size.width += (span - 1) * (float)(deltaX - BORDER_WIDTH);
        ++i;
      }
      [result addObject:[NSValue valueWithCGRect:rect]];
    }
  }

  return result;
}

- (NSArray *)rectsWithBound:(CGRect)bounds
       numberOfSubdivisions:(NSUInteger)number {
  assert(number > 0);
  auto *const result = [[NSMutableArray alloc] init];

  // prime the array
  [result addObject:[NSValue valueWithCGRect:bounds]];

  for (NSUInteger i = 0; i < number - 1; i++) {
    const NSValue *const biggestRect = result[0];
    [result removeObject:biggestRect];
    [result
        addObjectsFromArray:[self splitRectangle:[biggestRect CGRectValue]]];
  }
  [result sortUsingComparator:^(id obj1, id obj2) {
      const auto &rect1 = [obj1 CGRectValue];
      const auto &rect2 = [obj2 CGRectValue];
      const auto &index1 =
          rect1.origin.x + rect1.size.width +
          bounds.size.width * (rect1.origin.y + rect1.size.height);
      const auto &index2 =
          rect2.origin.x + rect2.size.width +
          bounds.size.width * (rect2.origin.y + rect2.size.height);

      return (NSComparisonResult)(index1 >= index2);
  }];

  return result;
}

- (NSArray *)splitRectangle:(CGRect)rectangle {
  auto *const result = [[NSMutableArray alloc] init];

  if (rectangle.size.width <= rectangle.size.height) {
    const auto &height = rectangle.size.height * 0.5 + BORDER_WIDTH * 0.5;
    const auto &offset = rectangle.size.height * 0.5 - BORDER_WIDTH * 0.5;
    [result addObject:[NSValue valueWithCGRect:CGRectMake(rectangle.origin.x,
                                                          rectangle.origin.y,
                                                          rectangle.size.width,
                                                          height)]];

    [result
        addObject:[NSValue valueWithCGRect:CGRectMake(
                                               rectangle.origin.x,
                                               rectangle.origin.y + offset,
                                               rectangle.size.width, height)]];
  } else {
    const auto &width = rectangle.size.width * 0.5 + BORDER_WIDTH * 0.5;
    const auto &offset = rectangle.size.width * 0.5 - BORDER_WIDTH * 0.5;
    [result
        addObject:[NSValue valueWithCGRect:CGRectMake(rectangle.origin.x,
                                                      rectangle.origin.y, width,
                                                      rectangle.size.height)]];

    [result addObject:[NSValue valueWithCGRect:CGRectMake(
                                                   rectangle.origin.x + offset,
                                                   rectangle.origin.y, width,
                                                   rectangle.size.height)]];
  }

  return result;
}

- (UIView *)viewForZoomingInScrollView:(UIScrollView *)_scrollView {
  //NSLog(@"viewForZoomingInScrollView");
  return [_scrollView subviews][0];
}

- (void)reloadData:(BOOL)animated {

}



- (IBAction)onSelectDevice:(id)sender {
  if (self.mm_drawerController != nil)
    [self.mm_drawerController closeDrawerAnimated:YES completion:^(BOOL){}];
  [switchDeviceAlertView show];
}

- (IBAction)deviceFunctionsPressed {
  if (self.mm_drawerController != nil)
    [self.mm_drawerController closeDrawerAnimated:YES completion:^(BOOL){}];
  [deviceFunctionActionSheet showInView:self.view];
}

- (IBAction)configurationFunctionsPressed {
  if (self.mm_drawerController != nil)
    [self.mm_drawerController closeDrawerAnimated:YES completion:^(BOOL){}];

  [configurationActionSheet showInView:self.view];
}

- (IBAction)helpButtonPressed:(UIBarButtonItem *)sender {
  if (self.mm_drawerController != nil)
    [self.mm_drawerController closeDrawerAnimated:YES completion:^(BOOL){}];

  [helpActionSheet showInView:self.view];
}

- (void)actionSheet:(UIActionSheet *)actionSheet
    clickedButtonAtIndex:(NSInteger)buttonIndex {
  if (actionSheet == deviceFunctionActionSheet) {
    [self handleDeviceFunctionActionSheet:buttonIndex];
  } else if (actionSheet == configurationActionSheet) {
    [self handleConfigurationActionSheet:buttonIndex];
  } else if (actionSheet == helpActionSheet) {
    [self handleHelpActionSheet:buttonIndex];
  }
}

- (void)handleDeviceFunctionActionSheet:(NSInteger)buttonIndex {
  if (buttonIndex < [deviceFunctionArray count]) {
    void (^callback)(void) = deviceFunctionArray[buttonIndex];
    if (callback) {
      callback();
    }
  }
}

- (void)handleConfigurationActionSheet:(NSInteger)buttonIndex {
  switch (buttonIndex) {
    case 0:  // Save Configuration
      [self showEnterFileNameView];
      break;

    case 1:  // Load Configuration
    {
      [self.navigationController
          pushViewController:[ICFileBrowserViewController
                                 fileBrowserWithDevice:self.device]
                    animated:YES];
    } break;

    default:
      break;
  }
}

- (void)handleHelpActionSheet:(NSInteger)buttonIndex {
  switch (buttonIndex) {
    case 0:  // Manual
    {
      const auto &deviceID = self.device->getDeviceID();

      const auto *const productString =
          [NSString stringWithFormat:@"%04d", deviceID.pid()];
      NSString *urlString = NULL;
        if ( deviceID.pid() == GeneSysLib::DevicePID::iConnect4Audio ) {
           //4.2.5 online URL update. zx, 2017-07-24
//        urlString =
//          [NSString stringWithFormat:
//                        @"http://www.iconnectivity.com/downloads/manuals/iCA4PManualVer1-1.pdf"];
            urlString = [NSString stringWithFormat:@"https://support.iconnectivity.com/support/iconfig/0007/iOS/"];
            
        }
      else if (deviceID.pid() == GeneSysLib::DevicePID::iConnect2Audio )
      {
        //urlString =
        //[NSString stringWithFormat:
        //    @"https://www.iconnectivity.com/downloads/manuals/iCA2plus_user_guide_ver1.pdf"];
          urlString = [NSString stringWithFormat:@"https://support.iconnectivity.com/support/iconfig/0008/iOS/"];

      }
      NSURL *const url = [NSURL URLWithString:urlString];

      NSURLRequest *const request = [NSURLRequest requestWithURL:url];

      auto *const helpViewController =
          [[ICHelpViewController alloc] initWithNibName:@"ICHelpViewController"
                                                 bundle:nil
                                                request:request];
      [self.navigationController pushViewController:helpViewController
                                           animated:YES];
    } break;

    case 1:  // Registration
    {
        //Updated for new registration URL, zx, 2017-07-26
/*      const auto &deviceID = self.device->getDeviceID();
      const auto &serialNumber = deviceID.serialNumber();

      uint32_t sn = serialNumber[0];
      sn = (sn << 7) | serialNumber[1];
      sn = (sn << 7) | serialNumber[2];
      sn = (sn << 7) | serialNumber[3];
      sn = (sn << 7) | serialNumber[4];

      auto *const productString =
          [NSString stringWithFormat:@"%04d", deviceID.pid()];
      auto *const snString = [NSString stringWithFormat:@"%08X", sn];
      NSString *const urlString = [NSString
          stringWithFormat:@"https://iconnectivity.com/"
                            "Registration?reg_product=%@&reg_serial_number=%@",
                           productString, snString];
*/
      NSString *const urlString = @"https://www.iconnectivity.com/registration/";
  
      NSURL *const url = [NSURL URLWithString:urlString];
      NSURLRequest *const request = [NSURLRequest requestWithURL:url];

      auto *const helpViewController =
          [[ICHelpViewController alloc] initWithNibName:@"ICHelpViewController"
                                                 bundle:nil
                                                request:request];

      [self.navigationController pushViewController:helpViewController
                                           animated:YES];
    } break;

    default:
      break;
  }
}

- (void)alertView:(UIAlertView *)alertView
    clickedButtonAtIndex:(NSInteger)buttonIndex {
  if (alertView == saveToFlashAlertView) {
    [self handleSaveToFlashAlertView:buttonIndex];
  } else if (alertView == restoreToFlashAlertView) {
    [self handleRestoreToFlashAlertView:buttonIndex];
  } else if (alertView == restoreToDefaultsAlertView) {
    [self handleRestoreToDefaultsAlertView:buttonIndex];
  } else if (alertView == resetDeviceAlertView) {
    [self handleResetDeviceAlertView:buttonIndex];
  } else if (alertView == overrideFileAlertView) {
    [self handleOverrideFileAlertView:buttonIndex];
  } else if (alertView == switchDeviceAlertView) {
    [self handleSwitchDeviceAlertView:buttonIndex];
  } else if (alertView == communicationErrorAlert) {
    [self handleCommunicationErrorAlertView:buttonIndex];
  }
}

- (void)handleSaveToFlashAlertView:(NSInteger)buttonIndex {
  if (buttonIndex == 1) {  // continue
    self.device->send<SaveRestoreCommand>(SaveRestoreID::SaveToFlash);
  }
}

- (void)handleRestoreToFlashAlertView:(NSInteger)buttonIndex {
  if (buttonIndex == 1) {  // continue
    self.device->send<SaveRestoreCommand>(SaveRestoreID::RestoreFromFlash);
  }
}

- (void)handleRestoreToDefaultsAlertView:(NSInteger)buttonIndex {
  if (buttonIndex == 1) {  // continue
    self.device->send<SaveRestoreCommand>(SaveRestoreID::FactoryDefault);
  }
}

- (void)handleResetDeviceAlertView:(NSInteger)buttonIndex {
  if (buttonIndex == 1) {  // continue
    self.device->send<ResetCommand>(BootMode::AppMode);
    [self showOverlay:@"Resetting..."];

    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:kCommunicationTimeout
                                                  object:nil];
    runOnMainAfter(7.2, ^{
        [self switchDevices];
        [self hideOverlay];
    });
  }
}

- (void)handleOverrideFileAlertView:(NSInteger)buttonIndex {
  if (buttonIndex == 1) {
    [self startSaveQuery];
    [self hideEnterFileNameView];
  }
}

- (void)handleSwitchDeviceAlertView:(NSInteger)buttonIndex {
  if (buttonIndex == 1) {
    [self switchDevices];
  }
}

- (void)handleCommunicationErrorAlertView:(NSInteger)buttonIndex {
  [self switchDevices];
  [self hideOverlay];
}

- (void)switchDevices {
  self.device->closeDevice();
  NSArray *const viewControllers = @[
    [[ICDeviceSelectionViewController alloc] initWithCommunicator:self.comm],
    self
  ];

  [self.navigationController setViewControllers:viewControllers animated:NO];

  [[NSNotificationCenter defaultCenter] removeObserver:self
                                                  name:kCommunicationTimeout
                                                object:nil];
  runOnMainAfter(0.5, ^{
      [(ICMIDIIndexTableViewController *)
          self.mm_drawerController.rightDrawerViewController clear];
      [self.navigationController popToRootViewControllerAnimated:YES];
  });
}

- (void)showReadingView {
  [self showOverlay:@"Reading..."];
}

- (void)hideReadingView {
  [self hideOverlay];
}

- (void)showWritingView {
  [self showOverlay:@"Writing..."];
}

- (void)hideWritingView {
  [self hideOverlay];
}

- (void)showSaving {
  [self showOverlay:@"Saving..."];
}

- (void)showLoading {
  [self showOverlay:@"Loading..."];
}

- (void)showOverlay:(NSString *)message {
  auto *const source = self.readingView;
  auto *const dest = self.view;
  auto *const label = self.statusText;

  runOnMain(^{
      [label setText:message];
      source.frame = dest.frame;
      [dest addSubview:source];
  });
}

- (void)hideOverlay {
  runOnMain(^{ [self.readingView removeFromSuperview]; });
}

- (void)showEnterFileNameView {
  auto *const source = self.fileNameView;
  auto *const dest = self.view;

  runOnMain(^{
      self.fileNameTextField.text = @"";
      source.frame = dest.frame;
      [dest addSubview:source];
      [self.fileNameTextField becomeFirstResponder];
  });
}

- (void)hideEnterFileNameView {
  runOnMain(^{
      [self.fileNameView removeFromSuperview];
      [self.fileNameTextField resignFirstResponder];
  });
}

- (IBAction)saveButtonPressed {
  if (self.fileNameTextField.text.length > 0) {
    NSString *const rootPath = [NSSearchPathForDirectoriesInDomains(
        NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
    NSString *const fileName =
        [self fileNameWithExtension:[rootPath stringByAppendingPathComponent:
                                                  self.fileNameTextField.text]];

    if (![[NSFileManager defaultManager] fileExistsAtPath:fileName]) {
      [self startSaveQuery];
      [self hideEnterFileNameView];
    } else {
      // Override alert
      [overrideFileAlertView show];
    }
  } else {
    [invalidFileNameAlertView show];
  }
}

- (IBAction)cancelButtonPressed {
  [self hideEnterFileNameView];
}

- (void)startSaveQuery {
  if (self.device->containsCommandData(Command::RetCommandList)) {
    const auto &commandList = self.device->get<CommandList>();
    list<CmdEnum> supportedQueries = {
        Command::GetAudioCfgInfo,     Command::GetAudioClockInfo,
        Command::GetAudioInfo,        Command::GetAudioPortCfgInfo,
        Command::GetAudioPortInfo,    Command::GetAudioPortPatchbay,
        Command::GetCommandList,      Command::GetDevice,
        Command::GetEthernetPortInfo, Command::GetInfo,
        Command::GetInfoList,         Command::GetMIDIInfo,
        Command::GetMIDIPortDetail,   Command::GetMIDIPortFilter,
        Command::GetMIDIPortInfo,     Command::GetMIDIPortRemap,
        Command::GetMIDIPortRoute,    Command::GetResetList,
        Command::GetSaveRestoreList};

    // Query everything
    list<CmdEnum> query;
    for (const auto &cmd : commandList.commandList) {
      const auto &queryCommand = (CmdEnum)(QUERY_BIT | cmd);
      if (contains(supportedQueries, queryCommand)) {
        query.push_back(queryCommand);
      }
    }
    self.device->startQuery(SaveScreen, query);
    [self showSaving];
  }
}

- (void)writeFileNamed:(NSString *)fileName {
  auto const &data = self.device->serialize();
  NSData *const nsData = [NSData dataWithBytes:data.data() length:data.size()];

  [nsData writeToFile:[self fileNameWithExtension:fileName] atomically:YES];
  [self hideOverlay];
}

- (NSString *)fileNameWithExtension:(NSString *)path {
  NSString *fileName = @"";
  NSString *supportedExtensionRegEx = nil;
  NSString *extension = nil;

  const auto &deviceID = self.device->getDeviceID();
  const auto &pid = deviceID.pid();

  if (pid == DevicePID::iConnect2Plus) {
    supportedExtensionRegEx = @"\\.icm2$";
    extension = @".icm2";
  } else if (pid == DevicePID::iConnect4Plus) {
    supportedExtensionRegEx = @"\\.icm4$";
    extension = @".icm4";
  }

  if ((supportedExtensionRegEx) &&
      ([path rangeOfString:supportedExtensionRegEx
                   options:(NSCaseInsensitiveSearch |
                            NSRegularExpressionSearch)].location ==
       NSNotFound)) {
    fileName = [path stringByAppendingString:extension];
  }
  return fileName;
}

- (void)queryComplete:(NSNotification *)notification {
  NSNumber *const screenNumber = (notification.userInfo)[@"screen"];
  NSLog(@"Query Complete! %d", [screenNumber intValue]);
  if ((Screen)[screenNumber intValue] == SaveScreen) {
    NSString *const rootPath = [NSSearchPathForDirectoriesInDomains(
        NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];

    NSString *const fileName =
        [rootPath stringByAppendingPathComponent:self.fileNameTextField.text];
    [self writeFileNamed:fileName];
  }
  if ((Screen)[screenNumber intValue] == RereadAllScreen) {
    [self hideReadingView];
  }
  if ((Screen)[screenNumber intValue] == RereadMeters) {
    if ([mainView class] == [ICMixerViewController class]) {
      [((ICMixerViewController*)mainView) updateMeters];
    }

    if (updateTimer) {
      [updateTimer invalidate];
      updateTimer = nil;
      //zx, 2017-06-16
/*      updateTimer = [NSTimer timerWithTimeInterval:0.1
                                            target:self
                                          selector:@selector(updateMeters)
                                          userInfo:nil
                                           repeats:NO];

      [[NSRunLoop currentRunLoop] addTimer:updateTimer
                                   forMode:NSDefaultRunLoopMode];*/
        updateTimer = [NSTimer scheduledTimerWithTimeInterval:0.1
                                              target:self
                                            selector:@selector(updateMeters)
                                            userInfo:nil
                                             repeats:NO];
        
      //NSLog(@"Added updateTimer");
    }

  }
}

- (void)onTimeout {
  NSLog(@"timer timedout!!");
  if (updateTimer) {
    [updateTimer invalidate];
    updateTimer = nil;
  }
  [[NSNotificationCenter defaultCenter] removeObserver:self
                                                  name:kCommunicationTimeout
                                                object:nil];
  if (![communicationErrorAlert isVisible]) {
    [communicationErrorAlert show];
  }
}

- (void)requestReset:(NSNotification *)notification {
  self.device->send<ResetCommand>(BootMode::AppMode);
  [self showOverlay:@"Resetting..."];

  [[NSNotificationCenter defaultCenter] removeObserver:self
                                                  name:kCommunicationTimeout
                                                object:nil];

  // 4.0 seconds is based on testing. It may need to be increased.
  runOnMainAfter(7.2, ^{
      [self switchDevices];
      [self hideOverlay];
  });
}

- (void)orientationChanged:(NSNotification *)aNotification {
  const auto &orientation = [[UIDevice currentDevice] orientation];
  const auto &nextIsPortrait = UIDeviceOrientationIsPortrait(orientation);
  const auto &nextIsLandscape = UIDeviceOrientationIsLandscape(orientation);

  if ((nextIsPortrait != nextIsLandscape) &&
      ((isPortrait != nextIsPortrait) || (isLandscape != nextIsLandscape))) {

    [self reloadData:NO];

    isPortrait = nextIsPortrait;
    isLandscape = nextIsLandscape;
  }
}

- (void)viewWillTransitionToSize:(CGSize)size withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator {
  //NSLog(@"ICView: viewWillTransitionToSize: %lu", (unsigned long)[[self childViewControllers] count] );
  [super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
  for(UIViewController *subviewController in [self childViewControllers]) {
    [subviewController viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
  }
}


@end
