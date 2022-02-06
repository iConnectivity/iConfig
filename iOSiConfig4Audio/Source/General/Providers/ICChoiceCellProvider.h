/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "ICChoiceDelegate.h"

@interface ICChoiceCellProvider : NSObject

@property(nonatomic, strong) id<ICChoiceDelegate> choiceDelegate;

+ (ICChoiceCellProvider *)providerWithDelegate:
        (id<ICChoiceDelegate>)choiceDelegate;

- (id)initWithDelegate:(id<ICChoiceDelegate>)choiceDelegate;

- (UITableViewCell *)cellForTableView:(UITableView *)tableView;

@end
