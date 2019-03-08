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

@protocol SquareViewProvider<NSObject>
- (NSString *)providerName;

- (NSString *)title;
- (NSArray *)buttonNames;

- (UIColor *)titleBackgroundColor;
- (UIColor *)titleBorderColor;

- (UIColor *)colorForIndex:(NSUInteger)index;
- (UIColor *)selectedColorForIndex:(NSUInteger)index;
- (UIColor *)borderColorForIndex:(NSUInteger)index;

- (NSUInteger)spanForIndex:(NSUInteger)index;

- (void)onButtonPressed:(ICViewController *)sender
                  index:(NSInteger)buttonIndex;

@optional

- (void)onButtonDown:(ICViewController *)sender index:(NSInteger)buttonIndex;
- (void)onButtonDragEnter:(ICViewController *)sender
                    index:(NSInteger)buttonIndex;
- (void)onButtonDragExit:(ICViewController *)sender
                   index:(NSInteger)buttonIndex;

- (void)initializeProviderButtons:(ICViewController *)sender;
- (void)providerWillAppear:(ICViewController *)sender;
- (void)providerWillDissappear:(ICViewController *)sender;

- (NSUInteger)numberOfColumns;
- (NSUInteger)numberOfRows;

- (CGSize)contentScale;

- (BOOL)isIndexSelected:(NSUInteger)index;
- (BOOL)isIndexEnabled:(NSUInteger)index;

@end

@interface ICViewController
    : UIViewController<UIScrollViewDelegate, UIActionSheetDelegate,
                       UIAlertViewDelegate> {
  IBOutlet UIView *headerView;
  IBOutlet UILabel *titleLabel;
  IBOutlet UIScrollView *scrollView;

  NSMutableArray *providers;
  __weak IBOutlet UIToolbar *myToolbar;

  NSArray *deviceFunctionArray;

  long ackHandlerID;

  bool isPortrait;
  bool isLandscape;
}

@property(strong, nonatomic) NSArray *providerButtons;
@property(assign, nonatomic) GeneSysLib::CommPtr comm;
@property(assign, nonatomic) DeviceInfoPtr device;
@property(strong, nonatomic) NSMutableDictionary *storedUserInformation;
@property(strong, nonatomic) IBOutlet UILabel *titleLabel;
@property(strong, nonatomic) IBOutlet UIView *readingView;
@property(strong, nonatomic) IBOutlet UILabel *statusText;
@property(weak, nonatomic)
    IBOutlet UIBarButtonItem *deviceFunctionBarButtonItem;
@property(weak, nonatomic) IBOutlet UIBarButtonItem *configurationBarButtonItem;

@property(strong, nonatomic) IBOutlet UIView *fileNameView;
@property(strong, nonatomic) IBOutlet UITextField *fileNameTextField;
@property(strong, nonatomic) IBOutlet UIView *BackColorView;

@property(strong, nonatomic) IBOutlet UIScrollView *headerScrollView;

+ (UIImage *)imageFromColor:(UIColor *)color;

- (id)initWithProvider:(id<SquareViewProvider>)provider
          communicator:(GeneSysLib::CommPtr)communicator
                device:(DeviceInfoPtr)deviceInfomation;

- (void)addButtonSubviews:(BOOL)animated;

- (NSArray *)rectsBounds:(CGRect)bounds
                withRows:(NSUInteger)rows
         numberOfColumns:(NSUInteger)cols
               withSpans:(NSArray *)spans;
- (NSArray *)rectsWithBound:(CGRect)bounds
       numberOfSubdivisions:(NSUInteger)number;
- (NSArray *)splitRectangle:(CGRect)rectangle;

- (void)pushToProvider:(id<SquareViewProvider>)provider animated:(BOOL)animated;
- (void)popProvider:(BOOL)animated;
- (void)popToIndex:(NSInteger)index animated:(BOOL)animated;

- (id<SquareViewProvider>)currentProvider;

- (void)reloadData:(BOOL)animated;

- (void)onBackButtonPressed:(id)sender;
- (void)onButtonPressed:(id)sender;
- (void)onButtonDown:(id)sender;
- (void)onButtonDragEnter:(id)sender;
- (void)onButtonDragExit:(id)sender;

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
