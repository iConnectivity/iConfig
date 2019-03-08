/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <UIKit/UIKit.h>

#include "DeviceInfo.h"
#include "MIDIInfo.h"

@interface ICBaseInfoViewController
    : UITableViewController<UITableViewDataSource, UITableViewDelegate,
                            UITextFieldDelegate> {
  NSArray *sectionArrays;
  NSArray *sectionTitles;
}

- (void) refreshInfo;
@property(weak, nonatomic) IBOutlet UITableView *myTableView;

@end
