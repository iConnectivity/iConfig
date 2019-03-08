/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <UIKit/UIKit.h>
#import "ICViewController.h"

@interface ICMenuViewController : UIViewController <UITableViewDelegate, UITableViewDataSource> {
  NSMutableArray *itemsArray;

  __weak ICViewController *ICvc;
}
@property (strong, nonatomic) IBOutlet UITableView *menuTableView;

- (void)setICViewController:(ICViewController*)ic;

@end
