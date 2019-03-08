/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <Foundation/Foundation.h>
#import "ICTextEditCellDelegate.h"

typedef NSString * (^TextEditCellGetBlock)(void);
typedef void (^TextEditCellSetBlock)(NSString *);

@interface ICTextEditCellProvider : NSObject<UITextFieldDelegate>

@property(strong, nonatomic) id<ICTextEditCellDelegate> textEditDelegate;

+ (ICTextEditCellProvider *)providerWithDelegate:
        (id<ICTextEditCellDelegate>)textEditDelegate;

- (id)initWithDelegate:(id<ICTextEditCellDelegate>)textEditDelegate;

- (UITableViewCell *)cellForTableView:(UITableView *)tableView;

@end
