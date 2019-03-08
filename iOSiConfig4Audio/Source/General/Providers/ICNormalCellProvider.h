/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <Foundation/Foundation.h>

typedef NSString * (^NormalCellGetBlock)(void);

@protocol NormalCellDelegate
- (NSString *)title;
- (NSString *)value;
@end

@interface ICNormalCellProvider : NSObject

@property(strong, nonatomic) id<NormalCellDelegate> normalDelegate;

+ (ICNormalCellProvider *)providerWithTitle:(NSString *)title
                                       value:(NSString *)value;
+ (ICNormalCellProvider *)providerWithTitle:(NSString *)title
                                    getValue:(NormalCellGetBlock)getValue;
+ (ICNormalCellProvider *)providerWithDelegate:
        (id<NormalCellDelegate>)normalDelegate;

- (id)initWithDelegate:(id<NormalCellDelegate>)normalDelegate;

- (UITableViewCell *)cellForTableView:(UITableView *)tableView;

@end
