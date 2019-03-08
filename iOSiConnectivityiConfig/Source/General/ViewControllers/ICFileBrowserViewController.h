/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <UIKit/UIKit.h>
#import "DeviceInfo.h"

@interface ICFileBrowserViewController
    : UIViewController<UITableViewDataSource, UITableViewDelegate>

@property(assign, nonatomic) DeviceInfoPtr device;
@property(strong, nonatomic) NSMutableArray *directoryListing;
@property(strong, nonatomic) IBOutlet UITableView *myTableView;

+ (ICFileBrowserViewController *)fileBrowserWithDevice:(DeviceInfoPtr)dev;

- (id)initWithDevice:(DeviceInfoPtr)dev;

- (void)editButton;

@end
