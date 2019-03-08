/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <Foundation/Foundation.h>

typedef void (^ButtonCellFunc)(void);

@protocol ButtonCellDelegate
- (NSString *)title;
- (NSString *)value;
- (ButtonCellFunc)func;
@end

@interface ICButtonCellProvider : NSObject

@property(strong, nonatomic) id<ButtonCellDelegate> buttonDelegate;

+ (ICButtonCellProvider *)providerWithTitle:(NSString *)title
                                       value:(NSString *)value;
+ (ICButtonCellProvider *)providerWithTitle:(NSString *)title
                                    value:(NSString*)value
                                       func:(ButtonCellFunc)func;
- (id)initWithDelegate:(id<ButtonCellDelegate>)buttonDelegate;

- (UITableViewCell *)cellForTableView:(UITableView *)tableView;

@end
