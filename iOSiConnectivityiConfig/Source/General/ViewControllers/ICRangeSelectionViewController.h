/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <UIKit/UIKit.h>
#import "ICRangeChoiceDelegate.h"

@interface ICRangeSelectionViewController
    : UIViewController<UITableViewDataSource, UITableViewDelegate>


+ (ICRangeSelectionViewController *)rangeSelectionWithDelegate:
        (id<ICRangeChoiceDelegate>)rangeDelegate;

- (id)initWithDelegate:(id<ICRangeChoiceDelegate>)rangeDelegate;

@end
