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
#import "ICRunOnMain.h"
#import "ICViewController.h"
#import "MyAlgorithms.h"
#import "Reset.h"
#import "ResetList.h"
#import "SaveRestore.h"
#import "SaveRestoreList.h"
#import "SysexCommand.h"
#import "UIImageTintCategory.h"
#import "UIViewController+MMDrawerController.h"

#import <CoreGraphics/CoreGraphics.h>
#import <QuartzCore/QuartzCore.h>

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
@synthesize titleLabel;

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

- (id)initWithProvider:(id<SquareViewProvider>)provider
          communicator:(CommPtr)communicator
                device:(DeviceInfoPtr)deviceInformation {
  self =
      [super initWithNibName:(([[UIDevice currentDevice] userInterfaceIdiom] ==
                               UIUserInterfaceIdiomPhone)
                                  ? (@"ICViewController_iPhone")
                                  : (@"ICViewController_iPad"))
                      bundle:nil];

  if (self != nil) {
    self.storedUserInformation = [NSMutableDictionary dictionary];
    providers = [[NSMutableArray alloc] initWithObjects:provider, nil];

    NSParameterAssert(provider);
    NSParameterAssert(communicator);
    NSParameterAssert(deviceInformation);

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
  }

  return self;
}

- (void)viewWillAppear:(BOOL)animated {
  [super viewWillAppear:animated];
  id<SquareViewProvider> squareProvider = [providers lastObject];

  // set the navigation title for back button
  if (squareProvider) {
    [self.navigationItem setTitle:[squareProvider title]];

    if ([squareProvider respondsToSelector:@selector(providerWillAppear:)]) {
      [squareProvider providerWillAppear:self];
      [self reloadData:NO];
    }
  }

  self.deviceFunctionBarButtonItem.enabled = ([deviceFunctionArray count] != 0);
  self.configurationBarButtonItem.enabled = ([deviceFunctionArray count] != 0);

  [self.navigationController setNavigationBarHidden:YES animated:YES];

  [scrollView setContentInset:UIEdgeInsetsZero];
  const auto &scrollBounds = scrollView.bounds;
  [scrollView setBounds:CGRectMake(0, 0, scrollBounds.size.width,
                                   scrollBounds.size.height)];
}

- (void)viewDidLayoutSubviews {
  [super viewDidLayoutSubviews];

  [scrollView setContentInset:UIEdgeInsetsZero];
  const auto &scrollBounds = scrollView.bounds;
  [scrollView setBounds:CGRectMake(0, 0, scrollBounds.size.width,
                                   scrollBounds.size.height)];
}

- (void)addButtonSubviews:(BOOL)animated {
  // NSLog(@"%@", [NSThread callStackSymbols]);
  const auto &isiPad = ([[UIDevice currentDevice] userInterfaceIdiom] ==
                        UIUserInterfaceIdiomPad);

  // Do any additional setup after loading the view, typically from a nib.
  id<SquareViewProvider> squareProvider = [providers lastObject];

  // Requre a square provider
  if (!squareProvider) {
    return;
  }

  if ([squareProvider
          respondsToSelector:@selector(initializeProviderButtons:)]) {
    [squareProvider initializeProviderButtons:self];
  }

  [self.navigationItem setTitle:[squareProvider title]];
  titleLabel.text = [squareProvider title];

  auto *const imageBackground = [UIImage imageNamed:@"Background.png"];
  self.view.backgroundColor = [UIColor colorWithPatternImage:imageBackground];
  self.BackColorView.backgroundColor =
      [[squareProvider titleBackgroundColor] colorWithAlphaComponent:0.8f];

  if (animated) {
    titleLabel.alpha = 0;
  }

  // Set the top bar
  auto *const font = [UIFont boldSystemFontOfSize:((isiPad) ? (20.0) : (15.0))];

  CGFloat totalButtonWidth = 0;
  CGFloat xOffset = BORDER_WIDTH;

  for (int i = 0; i < [providers count] - 1; i++) {
    CGFloat currWidth =
        [[NSString stringWithFormat:@"%@ >", [providers[i] title]]
            sizeWithFont:font].width +
        BORDER_WIDTH;

    id<SquareViewProvider> provider = providers[i];
    UIButton *const button = [UIButton buttonWithType:UIButtonTypeCustom];
    const auto &rect = CGRectMake(xOffset, titleLabel.frame.origin.y, currWidth,
                                  titleLabel.frame.size.height);
    xOffset += currWidth;
    [button setFrame:rect];
    [button setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
    auto *const buttonTitle =
        [NSString stringWithFormat:@"%@ >", [provider title]];
    [button setTitle:buttonTitle forState:UIControlStateNormal];
    [button setAutoresizingMask:UIViewAutoresizingFlexibleWidth |
                                UIViewAutoresizingFlexibleRightMargin |
                                UIViewAutoresizingFlexibleLeftMargin |
                                UIViewAutoresizingFlexibleTopMargin |
                                UIViewAutoresizingFlexibleBottomMargin];

    [button.titleLabel setTextAlignment:NSTextAlignmentLeft];
    [button.titleLabel setBaselineAdjustment:UIBaselineAdjustmentAlignCenters];

    button.tag = i;
    [button.titleLabel setFont:font];

    [button addTarget:self
                  action:@selector(onBackButtonPressed:)
        forControlEvents:UIControlEventTouchUpInside];
    [self.headerScrollView addSubview:button];

    totalButtonWidth += currWidth;
  }

  const auto &labelWidth =
      [[providers[[providers count] - 1] title] sizeWithFont:font].width +
      BORDER_WIDTH;

  self.headerScrollView.contentSize =
      CGSizeMake(totalButtonWidth + labelWidth + BORDER_WIDTH * 2.0f,
                 self.headerScrollView.contentSize.height);

  titleLabel.frame =
      CGRectMake(BORDER_WIDTH + totalButtonWidth, titleLabel.frame.origin.y,
                 labelWidth, titleLabel.frame.size.height);
  [titleLabel setTextAlignment:NSTextAlignmentCenter];

  if (self.headerScrollView.contentSize.width > headerView.frame.size.width) {
    [self.headerScrollView
        setContentOffset:CGPointMake(self.headerScrollView.contentSize.width -
                                         headerView.frame.size.width,
                                     0)
                animated:YES];
  }

  auto scaleFactor = CGSizeMake(1.0f, 1.0f);
  auto *const buttonNames = [squareProvider buttonNames];

  if ((squareProvider) &&
      ([squareProvider respondsToSelector:@selector(contentScale)])) {
    scaleFactor = [squareProvider contentScale];
  }

  if (buttonNames != nil) {
    // NSLog(@"scale factor (%f, %f)", scaleFactor.width, scaleFactor.height);
    [scrollView
        setContentSize:CGSizeMake(
                           scrollView.bounds.size.width * scaleFactor.width,
                           scrollView.bounds.size.height * scaleFactor.height)];

    const auto &bounds = CGRectMake(
        0, 0, scrollView.contentSize.width,  // - scrollView.contentInset.left -
        // scrollView.contentInset.right,
        scrollView.contentSize.height);  // - scrollView.contentInset.top -
    // scrollView.contentInset.bottom);

    NSArray *buttonRects = nil;
    if (([squareProvider respondsToSelector:@selector(numberOfRows)]) &&
        ([squareProvider respondsToSelector:@selector(numberOfColumns)])) {
      NSMutableArray *spans = nil;
      if ([squareProvider respondsToSelector:@selector(spanForIndex:)]) {
        spans = [[NSMutableArray alloc] init];

        for (NSUInteger i = 0; i < [buttonNames count]; ++i) {
          [spans addObject:@((int)[squareProvider spanForIndex:i])];
        }
      }
      buttonRects = [self rectsBounds:bounds
                             withRows:[squareProvider numberOfRows]
                      numberOfColumns:[squareProvider numberOfColumns]
                            withSpans:spans];
    } else {
      buttonRects =
          [self rectsWithBound:bounds numberOfSubdivisions:[buttonNames count]];
    }

    auto *const buttons = [NSMutableArray array];
    for (NSUInteger i = 0; i < [buttonNames count]; i++) {
      NSValue *const rectValue = buttonRects[i];
      const auto &rect = [rectValue CGRectValue];

      NSString *title;

      if ([squareProvider respondsToSelector:@selector(isIndexEnabled:)]) {
        title = ([squareProvider isIndexEnabled:i])
                    ? buttonNames[i]
                    : [NSString stringWithFormat:
                                    @"%@\nDisabled",
                                    [buttonNames[i]
                                        componentsSeparatedByString:@"\n"][0]];
      } else {
        title = buttonNames[i];
      }

      auto *const button = [[ICIndicatorButton alloc]
           initWithFrame:rect
          indicatorFrame:CGRectMake(rect.size.width - INDICATOR_WIDTH -
                                        INDICATOR_OFFSET,
                                    INDICATOR_OFFSET, INDICATOR_WIDTH,
                                    INDICATOR_HEIGHT)
                   title:title];

      [button setTitleColor:[UIColor colorWithWhite:1.0 alpha:0.9]
                   forState:UIControlStateNormal];

      [button setBackgroundColor:[squareProvider colorForIndex:i]
                        forState:UIControlStateNormal];
      [button setBackgroundColor:[squareProvider selectedColorForIndex:i]
                        forState:UIControlStateSelected];
      [button setBackgroundColor:[UIColor colorWithWhite:0.8 alpha:0.7]
                        forState:UIControlStateDisabled];

      button.layer.borderColor = [squareProvider borderColorForIndex:i].CGColor;
      button.layer.borderWidth = BORDER_WIDTH;

      button.tag = i;

      if ([squareProvider respondsToSelector:@selector(isIndexSelected:)]) {
        button.selected = [squareProvider isIndexSelected:i];
      }

      if (animated) {
        [button setTransform:CGAffineTransformMakeScale(0.01, 0.01)];
      }

      if ([squareProvider respondsToSelector:@selector(isIndexEnabled:)]) {
        [button setEnabled:[squareProvider isIndexEnabled:i]];
      }

      [button addTarget:self
                    action:@selector(onButtonPressed:)
          forControlEvents:UIControlEventTouchUpInside];
      [button addTarget:self
                    action:@selector(onButtonDown:)
          forControlEvents:UIControlEventTouchUpInside];
      [button addTarget:self
                    action:@selector(onButtonDragEnter:)
          forControlEvents:UIControlEventTouchDragEnter];
      [button addTarget:self
                    action:@selector(onButtonDragExit:)
          forControlEvents:UIControlEventTouchDragExit];

      [scrollView addSubview:button];
      [buttons addObject:button];
    }
    self.providerButtons = buttons;
    [scrollView setDelaysContentTouches:YES];
    [scrollView setScrollEnabled:YES];
    [scrollView setContentOffset:CGPointZero animated:NO];

    if (animated) {
      const auto &animations = ^{
          for (UIButton *button in self.providerButtons) {
            [button setTransform:CGAffineTransformMakeScale(1.0, 1.0)];
          }
          [titleLabel setAlpha:1];
      };
      const auto &completion = ^(BOOL finished) {
        if (finished) {
          [scrollView setContentOffset:CGPointZero animated:NO];
        }
      };
      [UIView animateWithDuration:ANIMATE_IN_TIME
                       animations:animations
                       completion:completion];
    } else {
      [scrollView setContentOffset:CGPointZero animated:YES];
    }
  }

  if ([squareProvider respondsToSelector:@selector(providerWillAppear:)]) {
    [squareProvider providerWillAppear:self];
  }

  if ([[UIDevice currentDevice] userInterfaceIdiom] ==
      UIUserInterfaceIdiomPhone) {
    [scrollView zoomToRect:CGRectMake(0, 0, scrollView.contentSize.width,
                                      scrollView.contentSize.height)
                  animated:NO];
  }
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
  myToolbar = nil;
  [self setBackColorView:nil];
  [self setConfigurationBarButtonItem:nil];
  [self setDeviceFunctionBarButtonItem:nil];
  [self setFileNameView:nil];
  [self setHeaderScrollView:nil];
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
      // NSLog(@"NACK for %04X", ack.commandID);
      // Show connection error
      // REFACTOR: rename onTimeout to onConnectionLost
      [self performSelectorOnMainThread:@selector(onTimeout)
                             withObject:nil
                          waitUntilDone:NO];
    } else {
      // NSLog(@"ACK for %04X", ack.commandID);
      if (ack.commandID() == Command::SaveRestore) {
        // NSLog(@"Save Restore ACK");
        [self showReadingView];
        self.device->rereadStored();
      }
    }
  };

  ackHandlerID = self.comm->registerHandler(Command::ACK, ackHandler);
  // NSLog(@"Register ACK");

  if ([[UIDevice currentDevice] userInterfaceIdiom] ==
      UIUserInterfaceIdiomPad) {
    [[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
    [[NSNotificationCenter defaultCenter]
        addObserver:self
           selector:@selector(orientationChanged:)
               name:UIDeviceOrientationDidChangeNotification
             object:nil];
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

  id<SquareViewProvider> squareProvider = [providers lastObject];

  if ([squareProvider respondsToSelector:@selector(providerWillDissappear:)]) {
    [squareProvider providerWillDissappear:self];
  }

  if (ackHandlerID >= 0) {
    self.comm->unRegisterHandler(Command::ACK, ackHandlerID);
    // NSLog(@"UnRegister ACK");
    ackHandlerID = -1;
  }
}

- (void)onDidEnterBackground {
  id<SquareViewProvider> squareProvider = [providers lastObject];

  if ([squareProvider respondsToSelector:@selector(providerWillDissappear:)]) {
    [squareProvider providerWillDissappear:self];
  }
}

- (void)onWillEnterForground {
  id<SquareViewProvider> squareProvider = [providers lastObject];

  if ([squareProvider respondsToSelector:@selector(providerWillAppear:)]) {
    [squareProvider providerWillAppear:self];
  }
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
  return [_scrollView subviews][0];
}

- (void)pushToProvider:(id<SquareViewProvider>)provider
              animated:(BOOL)animated {
  assert(provider != nil);
  id<SquareViewProvider> squareProvider = [providers lastObject];

  void (^completion)(BOOL) = ^(BOOL finished) {
    for (UIButton *const button in self.providerButtons) {
      [button removeFromSuperview];
    }
    self.providerButtons = nil;

    for (UIView *const subView in self.headerScrollView.subviews) {
      if ([subView isKindOfClass:[UIButton class]]) {
        [subView removeFromSuperview];
      }
    }

    [self addButtonSubviews:animated];
  };

  if ([squareProvider respondsToSelector:@selector(providerWillDissappear:)]) {
    [squareProvider providerWillDissappear:self];
  }

  [providers addObject:provider];

  if (animated) {
    [UIView animateWithDuration:ANIMATE_OUT_TIME
                     animations:^{
                         for (UIButton *const button in self.providerButtons) {
                           [button setTransform:CGAffineTransformMakeScale(
                                                    0.01, 0.01)];
                           [titleLabel setAlpha:0];
                         }
                     }
                     completion:completion];
  } else {
    completion(YES);
  }
}

- (void)popProvider:(BOOL)animated {
  [self popToIndex:[providers count] - 2 animated:YES];
}

- (void)popToIndex:(NSInteger)index animated:(BOOL)animated {
  if (index < [providers count] - 1) {
    id<SquareViewProvider> squareProvider = [providers lastObject];

    if ([squareProvider
            respondsToSelector:@selector(providerWillDissappear:)]) {
      [squareProvider providerWillDissappear:self];
    }

    [providers removeObjectsInRange:NSMakeRange(index + 1,
                                                [providers count] - index - 1)];

    void (^removeAndAddButtons)() = ^{
        [self removeAllButtonsFromSubview];
        [self addButtonSubviews:animated];
    };

    if (animated) {
      auto animations = ^{
          for (UIButton *button in self.providerButtons) {
            [button setTransform:CGAffineTransformMakeScale(0.01, 0.01)];
          }
          [titleLabel setAlpha:0];
      };
      auto completion = ^(BOOL finished) {
        removeAndAddButtons();
      };
      [UIView animateWithDuration:ANIMATE_OUT_TIME
                       animations:animations
                       completion:completion];
    } else {
      removeAndAddButtons();
    }
  }
}

- (void)removeAllButtonsFromSubview {
  for (UIButton *const button in self.providerButtons) {
    [button removeFromSuperview];
  }
  self.providerButtons = nil;

  for (UIView *const subView in self.headerScrollView.subviews) {
    if ([subView isKindOfClass:[UIButton class]]) {
      [subView removeFromSuperview];
    }
  }
}

- (id<SquareViewProvider>)currentProvider {
  return [providers lastObject];
}

- (void)reloadData:(BOOL)animated {
  if (animated) {
    const auto &animations = ^{
        for (UIButton *const button in self.providerButtons) {
          [button setTransform:CGAffineTransformMakeScale(0.01, 0.01)];
        }
    };
    const auto &completion = ^(BOOL finished) {
      for (UIButton *const button in self.providerButtons) {
        [button removeFromSuperview];
      }
      for (UIView *const subView in self.headerScrollView.subviews) {
        if ([subView isKindOfClass:[UIButton class]]) {
          [subView removeFromSuperview];
        }
      }
      [self addButtonSubviews:animated];
    };
    [UIView animateWithDuration:ANIMATE_OUT_TIME
                     animations:animations
                     completion:completion];
  } else {
    for (UIButton *const button in self.providerButtons) {
      [button removeFromSuperview];
    }

    for (UIView *const subView in self.headerScrollView.subviews) {
      if ([subView isKindOfClass:[UIButton class]]) {
        [subView removeFromSuperview];
      }
    }

    [self addButtonSubviews:animated];
  }
}

- (void)onButtonDown:(id)sender {
  static bool handled = true;

  if (!handled) {
    return;
  }
  handled = false;

  id<SquareViewProvider> squareProvider = [providers lastObject];
  assert(squareProvider);

  if (([sender isKindOfClass:[UIButton class]]) &&
      ([squareProvider respondsToSelector:@selector(onButtonDown:index:)])) {
    [squareProvider onButtonDown:self index:[sender tag]];
  }
  handled = true;
}

- (void)onButtonDragEnter:(id)sender {
  id<SquareViewProvider> squareProvider = [providers lastObject];
  assert(squareProvider);

  if (([sender isKindOfClass:[UIButton class]]) &&
      ([squareProvider
          respondsToSelector:@selector(onButtonDragEnter:index:)])) {
    [squareProvider onButtonDragEnter:self index:[sender tag]];
  }
}

- (void)onButtonDragExit:(id)sender {
  id<SquareViewProvider> squareProvider = [providers lastObject];
  assert(squareProvider);

  if (([sender isKindOfClass:[UIButton class]]) &&
      ([squareProvider
          respondsToSelector:@selector(onButtonDragExit:index:)])) {
    [squareProvider onButtonDragExit:self index:[sender tag]];
  }
}

- (void)onBackButtonPressed:(id)sender {
  if ([sender isKindOfClass:[UIButton class]]) {
    const UIButton *const button = sender;
    [self popToIndex:button.tag animated:YES];
  }
}

- (void)onButtonPressed:(id)sender {
  id<SquareViewProvider> squareProvider = [providers lastObject];

  if (squareProvider != nil) {
    [squareProvider onButtonPressed:self index:[sender tag]];
  }
}

- (IBAction)onSelectDevice:(id)sender {
  [switchDeviceAlertView show];
}

- (IBAction)deviceFunctionsPressed {
  [deviceFunctionActionSheet showInView:self.view];
}

- (IBAction)configurationFunctionsPressed {
  [configurationActionSheet showInView:self.view];
}

- (IBAction)helpButtonPressed:(UIBarButtonItem *)sender {
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
      [self popToIndex:0 animated:NO];
    } break;

    default:
      break;
  }
}

- (void)handleHelpActionSheet:(NSInteger)buttonIndex {
  switch (buttonIndex) {
    case 0:  // Manual
    {
        
      //Fixed online URLs bug, zx, 2017-07-26
      const auto &deviceID = self.device->getDeviceID();

      const auto *const productString =
          [NSString stringWithFormat:@"%04d", deviceID.pid()];
      NSString *const urlString =
          [NSString stringWithFormat:
                        //@"https://www.iconnectivity.com/support/iconfig/%@/iOS/",
                         @"https://support.iconnectivity.com/support/iconfig/%@/iOS/",
                         productString];
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
        //Fixed online URLs bug, zx, 2017-07-26
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
    [self popToIndex:0 animated:YES];
  }
}

- (void)handleRestoreToFlashAlertView:(NSInteger)buttonIndex {
  if (buttonIndex == 1) {  // continue
    self.device->send<SaveRestoreCommand>(SaveRestoreID::RestoreFromFlash);
    [self popToIndex:0 animated:YES];
  }
}

- (void)handleRestoreToDefaultsAlertView:(NSInteger)buttonIndex {
  if (buttonIndex == 1) {  // continue
    self.device->send<SaveRestoreCommand>(SaveRestoreID::FactoryDefault);
    [self popToIndex:0 animated:YES];
  }
}

- (void)handleResetDeviceAlertView:(NSInteger)buttonIndex {
  if (buttonIndex == 1) {  // continue
    self.device->send<ResetCommand>(BootMode::AppMode);
    [self showOverlay:@"Resetting..."];

    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:kCommunicationTimeout
                                                  object:nil];
    runOnMainAfter(3.2, ^{
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
  // NSLog(@"Query Complete!");
  NSNumber *const screenNumber = (notification.userInfo)[@"screen"];
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
}

- (void)onTimeout {
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
  runOnMainAfter(4.0, ^{
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

@end
