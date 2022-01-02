/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "ICRangeChoiceDelegate.h"

@interface ICRangeCellProvider : NSObject

@property(strong, nonatomic) id<ICRangeChoiceDelegate> rangeDelegate;

+ (ICRangeCellProvider *)providerWithDelegate:
        (id<ICRangeChoiceDelegate>)rangeDelegate;

- (id)initWithDelegate:(id<ICRangeChoiceDelegate>)rangeDelegate;

- (UITableViewCell *)cellForTableView:(UITableView *)tableView;

@end
