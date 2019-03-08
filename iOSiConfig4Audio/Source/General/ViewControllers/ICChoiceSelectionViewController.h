/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <UIKit/UIKit.h>
#import "ICChoiceDelegate.h"

typedef void (^SelectionCallback)(NSInteger index);

@interface ICChoiceSelectionViewController
    : UIViewController<UITableViewDataSource, UITableViewDelegate>

@property(strong, nonatomic) id<ICChoiceDelegate> rangeDelegate;

@property(strong, nonatomic) NSArray *choiceArray;
@property(weak, nonatomic) IBOutlet UITableView *myTable;
@property(nonatomic, assign) NSInteger selectedChoice;
@property(strong) SelectionCallback selectedCallback;

- (id)initWithDelegate:(id<ICChoiceDelegate>) rangeDelegate;

@end
